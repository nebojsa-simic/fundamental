/* X.509 certificate parsing and chain validation.
 *
 * Freestanding DER/ASN.1 decoder for the subset of RFC 5280 needed by the
 * TLS client: certificate fields, Basic Constraints and Subject Alternative
 * Name extensions, and ECDSA-P256/SHA-256 signature verification.
 */
#include <stddef.h>
#include <stdint.h>

#include "fundamental/crypto/crypto.h"
#include "fundamental/error/error.h"
#include "fundamental/timing/timing.h"

#define DER_SEQUENCE 0x30
#define DER_SET 0x31
#define DER_INTEGER 0x02
#define DER_BIT_STRING 0x03
#define DER_OCTET_STRING 0x04
#define DER_NULL 0x05
#define DER_OID 0x06
#define DER_BOOLEAN 0x01
#define DER_UTF8_STRING 0x0C
#define DER_PRINTABLE_STRING 0x13
#define DER_IA5_STRING 0x16
#define DER_UTCTIME 0x17
#define DER_GENERALIZEDTIME 0x18
#define DER_BMP_STRING 0x1E

#define DER_CONTEXT_0 0xA0
#define DER_CONTEXT_1 0xA1
#define DER_CONTEXT_3 0xA3
#define DER_CONTEXT_PRIM_1 0x81
#define DER_CONTEXT_PRIM_2 0x82

typedef struct {
	const uint8_t *p;
	size_t len;
	size_t pos;
} der_cursor;

typedef struct {
	const uint8_t *oid;
	size_t oid_len;
	const uint8_t *value;
	size_t value_len;
} name_attr;

#define X509_MAX_ATTRS 16

static void bytes_zero(void *dst, size_t n)
{
	uint8_t *d = (uint8_t *)dst;
	size_t i;
	for (i = 0; i < n; i++)
		d[i] = 0;
}

static int bytes_equal(const uint8_t *a, const uint8_t *b, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}

/* ---- DER primitives ---- */

static int der_peek_tag(const der_cursor *c)
{
	if (c->pos >= c->len)
		return -1;
	return c->p[c->pos];
}

static int der_read_length(const uint8_t *p, size_t avail, size_t *out_len,
						   size_t *out_hdr)
{
	uint8_t first;
	size_t n;
	size_t i;
	size_t value;

	if (avail < 1)
		return -1;
	first = p[0];
	if (first < 0x80) {
		*out_len = first;
		*out_hdr = 1;
		return 0;
	}
	if (first == 0x80)
		return -1; /* indefinite length is not valid DER */
	n = (size_t)(first & 0x7F);
	if (n == 0 || n > 4 || n + 1 > avail)
		return -1;
	value = 0;
	for (i = 0; i < n; i++)
		value = (value << 8) | p[1 + i];
	*out_len = value;
	*out_hdr = 1 + n;
	return 0;
}

static int der_read(der_cursor *c, int expected_tag, const uint8_t **value,
					size_t *value_len, const uint8_t **full, size_t *full_len)
{
	size_t hdr;
	size_t body;
	size_t total;
	int tag;

	if (c->pos >= c->len)
		return -1;
	tag = c->p[c->pos];
	if (expected_tag >= 0 && tag != expected_tag)
		return -1;
	if (der_read_length(c->p + c->pos + 1, c->len - c->pos - 1, &body, &hdr) !=
		0)
		return -1;
	total = 1 + hdr + body;
	if (total > c->len - c->pos)
		return -1;

	if (value)
		*value = c->p + c->pos + 1 + hdr;
	if (value_len)
		*value_len = body;
	if (full)
		*full = c->p + c->pos;
	if (full_len)
		*full_len = total;

	c->pos += total;
	return 0;
}

static int der_skip(der_cursor *c, int expected_tag)
{
	return der_read(c, expected_tag, NULL, NULL, NULL, NULL);
}

static int der_expect_end(const der_cursor *c)
{
	return c->pos == c->len ? 0 : -1;
}

/* ---- OIDs ---- */

static const uint8_t OID_EC_PUBLIC_KEY[] = { 0x2A, 0x86, 0x48, 0xCE,
											 0x3D, 0x02, 0x01 };
static const uint8_t OID_PRIME256V1[] = { 0x2A, 0x86, 0x48, 0xCE,
										  0x3D, 0x03, 0x01, 0x07 };
static const uint8_t OID_ECDSA_SHA256[] = { 0x2A, 0x86, 0x48, 0xCE,
											0x3D, 0x04, 0x03, 0x02 };
static const uint8_t OID_COMMON_NAME[] = { 0x55, 0x04, 0x03 };
static const uint8_t OID_BASIC_CONSTRAINTS[] = { 0x55, 0x1D, 0x13 };
static const uint8_t OID_SUBJECT_ALT_NAME[] = { 0x55, 0x1D, 0x11 };

static int oid_equal(const uint8_t *a, size_t a_len, const uint8_t *b,
					 size_t b_len)
{
	return a_len == b_len && bytes_equal(a, b, a_len);
}

/* ---- time ---- */

static int64_t days_from_civil(int64_t y, unsigned m, unsigned d)
{
	int64_t era;
	unsigned yoe;
	unsigned doy;
	unsigned doe;
	int64_t y2 = y - (m <= 2);

	era = (y2 >= 0 ? y2 : y2 - 399) / 400;
	yoe = (unsigned)(y2 - era * 400);
	doy = (153 * (m + (m > 2 ? (unsigned)-3 : 9)) + 2) / 5 + d - 1;
	doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + (int64_t)doe - 719468;
}

static int parse_decimal(const uint8_t *s, size_t n, int *out)
{
	int v = 0;
	size_t i;
	if (n == 0)
		return -1;
	for (i = 0; i < n; i++) {
		if (s[i] < '0' || s[i] > '9')
			return -1;
		v = v * 10 + (s[i] - '0');
	}
	*out = v;
	return 0;
}

static int parse_time(const uint8_t *v, size_t n, int64_t *out)
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	if (n < 13 || v[n - 1] != 'Z')
		return -1;

	if (n == 15) {
		/* GeneralizedTime: YYYYMMDDHHMMSSZ */
		if (parse_decimal(v, 4, &year) != 0)
			return -1;
		v += 4;
	} else if (n == 13) {
		/* UTCTime: YYMMDDHHMMSSZ (RFC 5280 uses 1950..2049) */
		int yy;
		if (parse_decimal(v, 2, &yy) != 0)
			return -1;
		year = yy >= 50 ? 1900 + yy : 2000 + yy;
		v += 2;
	} else {
		return -1;
	}

	if (parse_decimal(v, 2, &month) != 0)
		return -1;
	if (parse_decimal(v + 2, 2, &day) != 0)
		return -1;
	if (parse_decimal(v + 4, 2, &hour) != 0)
		return -1;
	if (parse_decimal(v + 6, 2, &minute) != 0)
		return -1;
	if (parse_decimal(v + 8, 2, &second) != 0)
		return -1;

	if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 ||
		minute > 59 || second > 60)
		return -1;

	*out = days_from_civil(year, (unsigned)month, (unsigned)day) * 86400 +
		   (int64_t)hour * 3600 + (int64_t)minute * 60 + second;
	return 0;
}

static int der_read_time(der_cursor *c, int64_t *out)
{
	int tag = der_peek_tag(c);
	const uint8_t *v;
	size_t n;

	if (tag != DER_UTCTIME && tag != DER_GENERALIZEDTIME)
		return -1;
	if (der_read(c, tag, &v, &n, NULL, NULL) != 0)
		return -1;
	return parse_time(v, n, out);
}

/* ---- names ---- */

static int is_string_tag(int tag)
{
	return tag == DER_UTF8_STRING || tag == DER_PRINTABLE_STRING ||
		   tag == DER_IA5_STRING || tag == DER_BMP_STRING;
}

static int ascii_case_equal(const uint8_t *a, size_t a_len, const uint8_t *b,
							size_t b_len)
{
	size_t i;
	if (a_len != b_len)
		return 0;
	for (i = 0; i < a_len; i++) {
		uint8_t ca = a[i];
		uint8_t cb = b[i];
		if (ca >= 'A' && ca <= 'Z')
			ca = (uint8_t)(ca - 'A' + 'a');
		if (cb >= 'A' && cb <= 'Z')
			cb = (uint8_t)(cb - 'A' + 'a');
		if (ca != cb)
			return 0;
	}
	return 1;
}

static int parse_name_attrs(const uint8_t *name, size_t name_len,
							name_attr *attrs, size_t max_attrs,
							size_t *out_count)
{
	der_cursor outer = { name, name_len, 0 };
	der_cursor seq;
	const uint8_t *v;
	size_t n;
	size_t count = 0;

	if (der_read(&outer, DER_SEQUENCE, &v, &n, NULL, NULL) != 0)
		return -1;
	seq.p = v;
	seq.len = n;
	seq.pos = 0;

	while (seq.pos < seq.len) {
		der_cursor set;
		const uint8_t *sv;
		size_t sn;

		if (der_read(&seq, DER_SET, &sv, &sn, NULL, NULL) != 0)
			return -1;
		set.p = sv;
		set.len = sn;
		set.pos = 0;

		while (set.pos < set.len) {
			der_cursor atv;
			const uint8_t *av;
			size_t an;
			const uint8_t *oid;
			size_t oid_len;
			const uint8_t *val;
			size_t val_len;
			int tag;

			if (der_read(&set, DER_SEQUENCE, &av, &an, NULL, NULL) != 0)
				return -1;
			atv.p = av;
			atv.len = an;
			atv.pos = 0;

			if (der_read(&atv, DER_OID, &oid, &oid_len, NULL, NULL) != 0)
				return -1;
			tag = der_peek_tag(&atv);
			if (tag < 0 || der_read(&atv, tag, &val, &val_len, NULL, NULL) != 0)
				return -1;

			if (count < max_attrs) {
				attrs[count].oid = oid;
				attrs[count].oid_len = oid_len;
				attrs[count].value = val;
				attrs[count].value_len = val_len;
				count++;
			}
		}
	}

	*out_count = count;
	return 0;
}

static int name_equal(const uint8_t *a, size_t a_len, const uint8_t *b,
					  size_t b_len)
{
	name_attr aa[X509_MAX_ATTRS];
	name_attr bb[X509_MAX_ATTRS];
	size_t ac = 0;
	size_t bc = 0;
	size_t i;
	size_t j;

	if (parse_name_attrs(a, a_len, aa, X509_MAX_ATTRS, &ac) != 0)
		return 0;
	if (parse_name_attrs(b, b_len, bb, X509_MAX_ATTRS, &bc) != 0)
		return 0;
	if (ac != bc)
		return 0;

	for (i = 0; i < ac; i++) {
		int found = 0;
		for (j = 0; j < bc; j++) {
			if (!oid_equal(aa[i].oid, aa[i].oid_len, bb[j].oid, bb[j].oid_len))
				continue;
			if (ascii_case_equal(aa[i].value, aa[i].value_len, bb[j].value,
								 bb[j].value_len)) {
				found = 1;
				break;
			}
		}
		if (!found)
			return 0;
	}
	return 1;
}

/* ---- extensions ---- */

static void parse_basic_constraints(fun_crypto_x509_cert *cert,
									const uint8_t *v, size_t n)
{
	der_cursor outer = { v, n, 0 };
	der_cursor seq;
	const uint8_t *sv;
	size_t sn;
	int tag;

	if (der_read(&outer, DER_SEQUENCE, &sv, &sn, NULL, NULL) != 0)
		return;
	seq.p = sv;
	seq.len = sn;
	seq.pos = 0;

	cert->has_basic_constraints = 1;
	cert->is_ca = 0;
	cert->path_length = -1;

	tag = der_peek_tag(&seq);
	if (tag == DER_BOOLEAN) {
		const uint8_t *bv;
		size_t bn;
		if (der_read(&seq, DER_BOOLEAN, &bv, &bn, NULL, NULL) != 0)
			return;
		cert->is_ca = (bn > 0 && bv[0] != 0) ? 1 : 0;
		tag = der_peek_tag(&seq);
	}
	if (tag == DER_INTEGER) {
		const uint8_t *iv;
		size_t in;
		size_t i;
		int64_t path = 0;
		if (der_read(&seq, DER_INTEGER, &iv, &in, NULL, NULL) != 0)
			return;
		if (in > 4)
			return;
		for (i = 0; i < in; i++)
			path = (path << 8) | iv[i];
		cert->path_length = (int)path;
	}
}

static void parse_subject_alt_name(fun_crypto_x509_cert *cert, const uint8_t *v,
								   size_t n)
{
	der_cursor outer = { v, n, 0 };
	der_cursor seq;
	const uint8_t *sv;
	size_t sn;

	if (der_read(&outer, DER_SEQUENCE, &sv, &sn, NULL, NULL) != 0)
		return;
	seq.p = sv;
	seq.len = sn;
	seq.pos = 0;

	cert->has_san = 1;
	cert->num_san_dns = 0;

	while (seq.pos < seq.len) {
		const uint8_t *gv;
		size_t gn;
		int tag = der_peek_tag(&seq);

		if (tag < 0 || der_read(&seq, tag, &gv, &gn, NULL, NULL) != 0)
			return;

		if (tag == DER_CONTEXT_PRIM_2 &&
			cert->num_san_dns < FUN_CRYPTO_X509_MAX_SAN_DNS) {
			size_t idx = cert->num_san_dns;
			cert->san_dns[idx] = (const char *)gv;
			cert->san_dns_len[idx] = gn;
			cert->num_san_dns++;
		}
	}
}

static void parse_extensions(fun_crypto_x509_cert *cert, const uint8_t *v,
							 size_t n)
{
	der_cursor outer = { v, n, 0 };
	der_cursor list;
	const uint8_t *lv;
	size_t ln;

	if (der_read(&outer, DER_SEQUENCE, &lv, &ln, NULL, NULL) != 0)
		return;
	list.p = lv;
	list.len = ln;
	list.pos = 0;

	while (list.pos < list.len) {
		der_cursor ext;
		const uint8_t *ev;
		size_t en;
		const uint8_t *oid;
		size_t oid_len;
		const uint8_t *value;
		size_t value_len;
		int tag;

		if (der_read(&list, DER_SEQUENCE, &ev, &en, NULL, NULL) != 0)
			return;
		ext.p = ev;
		ext.len = en;
		ext.pos = 0;

		if (der_read(&ext, DER_OID, &oid, &oid_len, NULL, NULL) != 0)
			return;

		if (der_peek_tag(&ext) == DER_BOOLEAN &&
			der_skip(&ext, DER_BOOLEAN) != 0)
			return;

		tag = der_peek_tag(&ext);
		if (tag != DER_OCTET_STRING || der_read(&ext, DER_OCTET_STRING, &value,
												&value_len, NULL, NULL) != 0)
			return;

		if (oid_equal(oid, oid_len, OID_BASIC_CONSTRAINTS,
					  sizeof(OID_BASIC_CONSTRAINTS)))
			parse_basic_constraints(cert, value, value_len);
		else if (oid_equal(oid, oid_len, OID_SUBJECT_ALT_NAME,
						   sizeof(OID_SUBJECT_ALT_NAME)))
			parse_subject_alt_name(cert, value, value_len);
	}
}

static void extract_common_name(const uint8_t *name, size_t name_len,
								const char **out, size_t *out_len)
{
	name_attr attrs[X509_MAX_ATTRS];
	size_t count = 0;
	size_t i;

	*out = NULL;
	*out_len = 0;

	if (parse_name_attrs(name, name_len, attrs, X509_MAX_ATTRS, &count) != 0)
		return;

	for (i = 0; i < count; i++) {
		if (oid_equal(attrs[i].oid, attrs[i].oid_len, OID_COMMON_NAME,
					  sizeof(OID_COMMON_NAME))) {
			*out = (const char *)attrs[i].value;
			*out_len = attrs[i].value_len;
			return;
		}
	}
}

/* ---- SPKI / signature ---- */

static int parse_spki(fun_crypto_x509_cert *cert, const uint8_t *v, size_t n)
{
	der_cursor outer = { v, n, 0 };
	const uint8_t *av;
	size_t an;
	const uint8_t *oid;
	size_t oid_len;
	const uint8_t *bits;
	size_t bits_len;

	if (der_read(&outer, DER_SEQUENCE, &av, &an, NULL, NULL) != 0)
		return -1;

	{
		der_cursor alg_inner = { av, an, 0 };
		if (der_read(&alg_inner, DER_OID, &oid, &oid_len, NULL, NULL) != 0)
			return -1;
		if (!oid_equal(oid, oid_len, OID_EC_PUBLIC_KEY,
					   sizeof(OID_EC_PUBLIC_KEY)))
			return -1;
		if (der_read(&alg_inner, DER_OID, &oid, &oid_len, NULL, NULL) != 0)
			return -1;
		if (!oid_equal(oid, oid_len, OID_PRIME256V1, sizeof(OID_PRIME256V1)))
			return -1;
	}

	if (der_read(&outer, DER_BIT_STRING, &bits, &bits_len, NULL, NULL) != 0)
		return -1;
	if (bits_len != 66 || bits[0] != 0 || bits[1] != 0x04)
		return -1;

	bytes_zero(&cert->public_key, sizeof(cert->public_key));
	{
		size_t i;
		for (i = 0; i < 32; i++) {
			cert->public_key.x[i] = bits[2 + i];
			cert->public_key.y[i] = bits[34 + i];
		}
	}
	cert->public_key_valid = 1;
	return 0;
}

static int parse_signature_algorithm(const uint8_t *v, size_t n)
{
	der_cursor outer = { v, n, 0 };
	const uint8_t *oid;
	size_t oid_len;

	if (der_read(&outer, DER_OID, &oid, &oid_len, NULL, NULL) != 0)
		return FUN_CRYPTO_X509_SIG_UNKNOWN;
	if (oid_equal(oid, oid_len, OID_ECDSA_SHA256, sizeof(OID_ECDSA_SHA256)))
		return FUN_CRYPTO_X509_SIG_ECDSA_SHA256;
	return FUN_CRYPTO_X509_SIG_UNKNOWN;
}

static int integer_to_fixed32(const uint8_t *v, size_t n, uint8_t out[32])
{
	size_t i = 0;
	size_t j;
	size_t rem;

	if (n == 0 || (v[0] & 0x80) != 0)
		return -1; /* zero or negative is not a valid component */
	while (i < n && v[i] == 0x00)
		i++;
	if (i == n)
		return -1;
	rem = n - i;
	if (rem > 32)
		return -1;

	for (j = 0; j < 32; j++)
		out[j] = 0;
	for (j = 0; j < rem; j++)
		out[32 - rem + j] = v[i + j];
	return 0;
}

static int verify_der_signature(const uint8_t *sig, size_t sig_len,
								const uint8_t *tbs, size_t tbs_len,
								const fun_crypto_ec_point *public_key)
{
	der_cursor outer = { sig, sig_len, 0 };
	const uint8_t *rv;
	size_t rn;
	const uint8_t *sv;
	size_t sn;
	uint8_t r[32];
	uint8_t s[32];
	uint8_t hash[32];

	if (der_read(&outer, DER_SEQUENCE, &rv, &rn, NULL, NULL) != 0)
		return -1;
	{
		der_cursor seq = { rv, rn, 0 };
		if (der_read(&seq, DER_INTEGER, &rv, &rn, NULL, NULL) != 0)
			return -1;
		if (der_read(&seq, DER_INTEGER, &sv, &sn, NULL, NULL) != 0)
			return -1;
	}
	if (integer_to_fixed32(rv, rn, r) != 0)
		return -1;
	if (integer_to_fixed32(sv, sn, s) != 0)
		return -1;

	fun_crypto_sha256(tbs, tbs_len, hash);
	return fun_crypto_ecdsa_verify(hash, public_key, r, s);
}

/* ---- public parse ---- */

int fun_crypto_x509_parse(const uint8_t *der, size_t der_len,
						  fun_crypto_x509_cert *cert)
{
	der_cursor outer;
	der_cursor body;
	der_cursor tbs;
	const uint8_t *v;
	size_t n;
	const uint8_t *tbs_ptr;
	size_t tbs_len;
	int tag;

	if (!der || !cert || der_len == 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	bytes_zero(cert, sizeof(*cert));
	cert->der = der;
	cert->der_len = der_len;
	cert->path_length = -1;

	outer.p = der;
	outer.len = der_len;
	outer.pos = 0;

	if (der_read(&outer, DER_SEQUENCE, &v, &n, NULL, NULL) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	if (der_expect_end(&outer) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	body.p = v;
	body.len = n;
	body.pos = 0;

	/* TBSCertificate */
	if (der_read(&body, DER_SEQUENCE, &v, &n, &tbs_ptr, &tbs_len) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	cert->tbs = tbs_ptr;
	cert->tbs_len = tbs_len;
	tbs.p = v;
	tbs.len = n;
	tbs.pos = 0;

	/* [0] version (optional) */
	if (der_peek_tag(&tbs) == DER_CONTEXT_0) {
		if (der_skip(&tbs, DER_CONTEXT_0) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
	}
	/* serialNumber */
	if (der_skip(&tbs, DER_INTEGER) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	/* signature algorithm */
	if (der_skip(&tbs, DER_SEQUENCE) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	/* issuer */
	if (der_read(&tbs, DER_SEQUENCE, &v, &n, &cert->issuer,
				 &cert->issuer_len) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	/* validity */
	{
		der_cursor validity;
		if (der_read(&tbs, DER_SEQUENCE, &v, &n, NULL, NULL) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
		validity.p = v;
		validity.len = n;
		validity.pos = 0;
		if (der_read_time(&validity, &cert->not_before) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
		if (der_read_time(&validity, &cert->not_after) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
		if (der_expect_end(&validity) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
	}

	/* subject */
	if (der_read(&tbs, DER_SEQUENCE, &v, &n, &cert->subject,
				 &cert->subject_len) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	/* subjectPublicKeyInfo */
	if (der_read(&tbs, DER_SEQUENCE, &v, &n, NULL, NULL) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	if (parse_spki(cert, v, n) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;

	extract_common_name(cert->subject, cert->subject_len, &cert->common_name,
						&cert->common_name_len);
	extract_common_name(cert->issuer, cert->issuer_len,
						&cert->issuer_common_name,
						&cert->issuer_common_name_len);

	/* issuerUniqueID / subjectUniqueID (optional in X.509 v1 style) */
	for (tag = der_peek_tag(&tbs);
		 tag == DER_CONTEXT_PRIM_1 || tag == DER_CONTEXT_PRIM_2;
		 tag = der_peek_tag(&tbs)) {
		if (der_skip(&tbs, tag) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
	}

	/* extensions [3] EXPLICIT (optional) */
	if (der_peek_tag(&tbs) == DER_CONTEXT_3) {
		if (der_read(&tbs, DER_CONTEXT_3, &v, &n, NULL, NULL) != 0)
			return ERROR_CODE_X509_PARSE_ERROR;
		parse_extensions(cert, v, n);
	}

	/* signatureAlgorithm */
	if (der_read(&body, DER_SEQUENCE, &v, &n, NULL, NULL) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	cert->signature_algorithm = parse_signature_algorithm(v, n);
	if (cert->signature_algorithm != FUN_CRYPTO_X509_SIG_ECDSA_SHA256)
		return ERROR_CODE_X509_PARSE_ERROR;

	/* signatureValue */
	if (der_read(&body, DER_BIT_STRING, &v, &n, NULL, NULL) != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	if (n < 1 || v[0] != 0)
		return ERROR_CODE_X509_PARSE_ERROR;
	cert->signature = v + 1;
	cert->signature_len = n - 1;

	return 0;
}

/* ---- hostname matching ---- */

static int dns_pattern_match(const char *pattern, size_t plen, const char *host,
							 size_t hlen)
{
	if (plen >= 2 && pattern[0] == '*' && pattern[1] == '.') {
		/* Wildcard matches exactly one left-most label. */
		size_t i;
		size_t dot = 0;
		int have_dot = 0;
		for (i = 0; i < hlen; i++) {
			if (host[i] == '.') {
				dot = i;
				have_dot = 1;
				break;
			}
		}
		if (!have_dot || dot == 0)
			return 0;
		if (hlen - dot != plen - 1)
			return 0;
		return ascii_case_equal((const uint8_t *)pattern + 1, plen - 1,
								(const uint8_t *)host + dot, hlen - dot);
	}
	return ascii_case_equal((const uint8_t *)pattern, plen,
							(const uint8_t *)host, hlen);
}

static int cert_matches_hostname(const fun_crypto_x509_cert *cert,
								 const char *hostname)
{
	size_t hlen = 0;
	size_t i;

	if (!hostname)
		return 1;
	while (hostname[hlen])
		hlen++;
	if (hlen > 0 && hostname[hlen - 1] == '.')
		hlen--; /* tolerate absolute FQDN form */

	if (cert->has_san) {
		for (i = 0; i < cert->num_san_dns; i++) {
			if (dns_pattern_match(cert->san_dns[i], cert->san_dns_len[i],
								  hostname, hlen))
				return 1;
		}
		return 0;
	}

	if (cert->common_name)
		return dns_pattern_match(cert->common_name, cert->common_name_len,
								 hostname, hlen);
	return 0;
}

/* ---- chain validation ---- */

static int cert_time_valid(const fun_crypto_x509_cert *cert, int64_t now)
{
	if (now < cert->not_before || now > cert->not_after)
		return 0;
	return 1;
}

static int verify_signed_by(const fun_crypto_x509_cert *child,
							const fun_crypto_x509_cert *issuer)
{
	if (!child->public_key_valid || !issuer->public_key_valid)
		return -1;
	if (!name_equal(child->issuer, child->issuer_len, issuer->subject,
					issuer->subject_len))
		return -1;
	return verify_der_signature(child->signature, child->signature_len,
								child->tbs, child->tbs_len,
								&issuer->public_key);
}

int fun_crypto_x509_validate_chain_at(const fun_crypto_x509_cert *certs,
									  size_t num_certs,
									  const fun_crypto_x509_cert *trusted_roots,
									  size_t num_roots, const char *hostname,
									  int64_t now_unix)
{
	size_t i;
	int found_root = 0;

	if (!certs || num_certs == 0 || !trusted_roots || num_roots == 0)
		return ERROR_CODE_TLS_CERTIFICATE_INVALID;

	for (i = 0; i < num_certs; i++) {
		if (!certs[i].public_key_valid)
			return ERROR_CODE_TLS_CERTIFICATE_INVALID;
		if (!cert_time_valid(&certs[i], now_unix))
			return ERROR_CODE_TLS_CERTIFICATE_EXPIRED;
	}

	if (!cert_matches_hostname(&certs[0], hostname))
		return ERROR_CODE_TLS_HOSTNAME_MISMATCH;

	for (i = 0; i + 1 < num_certs; i++) {
		const fun_crypto_x509_cert *child = &certs[i];
		const fun_crypto_x509_cert *issuer = &certs[i + 1];

		if (issuer->has_basic_constraints && !issuer->is_ca)
			return ERROR_CODE_TLS_CERTIFICATE_INVALID;
		if (verify_signed_by(child, issuer) != 0)
			return ERROR_CODE_TLS_CERTIFICATE_INVALID;
	}

	for (i = 0; i < num_roots; i++) {
		if (verify_signed_by(&certs[num_certs - 1], &trusted_roots[i]) == 0) {
			found_root = 1;
			break;
		}
	}
	if (!found_root)
		return ERROR_CODE_TLS_CERTIFICATE_INVALID;

	return 0;
}

int fun_crypto_x509_validate_chain(const fun_crypto_x509_cert *certs,
								   size_t num_certs,
								   const fun_crypto_x509_cert *trusted_roots,
								   size_t num_roots, const char *hostname)
{
	return fun_crypto_x509_validate_chain_at(certs, num_certs, trusted_roots,
											 num_roots, hostname,
											 fun_timing_now_unix());
}