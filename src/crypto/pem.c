/* PEM parsing (RFC 7468) - Base64 decoding of CERTIFICATE blocks.
 *
 * Freestanding: no libc, no heap. The caller owns both the PEM input and
 * the DER output buffers.
 */
#include <stddef.h>
#include <stdint.h>

#include "fundamental/crypto/crypto.h"

#define PEM_BEGIN "-----BEGIN CERTIFICATE-----"
#define PEM_END "-----END CERTIFICATE-----"
#define PEM_BEGIN_LEN (sizeof(PEM_BEGIN) - 1)
#define PEM_END_LEN (sizeof(PEM_END) - 1)

static int pem_b64_value(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	return -1;
}

static int pem_b64_decode(const char *in, size_t in_len, uint8_t *out,
						  size_t out_cap, size_t *out_len)
{
	uint32_t acc = 0;
	size_t written = 0;
	int have = 0;
	int pad = 0;
	size_t i;

	for (i = 0; i < in_len; i++) {
		char c = in[i];
		int v;

		if (c == '\r' || c == '\n' || c == ' ' || c == '\t')
			continue;
		if (c == '=') {
			pad++;
			continue;
		}
		if (pad > 0)
			return -1; /* data after padding */
		v = pem_b64_value(c);
		if (v < 0)
			return -1;

		acc = (acc << 6) | (uint32_t)v;
		have++;
		if (have == 4) {
			if (written + 3 > out_cap)
				return -1;
			out[written++] = (uint8_t)(acc >> 16);
			out[written++] = (uint8_t)(acc >> 8);
			out[written++] = (uint8_t)acc;
			acc = 0;
			have = 0;
		}
	}

	if (have == 1 || have > 3 || pad > 2)
		return -1;
	if (have == 2) {
		if (pad > 2)
			return -1;
		if (written + 1 > out_cap)
			return -1;
		out[written++] = (uint8_t)(acc >> 4);
	} else if (have == 3) {
		if (pad > 1)
			return -1;
		if (written + 2 > out_cap)
			return -1;
		out[written++] = (uint8_t)(acc >> 10);
		out[written++] = (uint8_t)(acc >> 2);
	} else if (pad > 0) {
		return -1;
	}

	*out_len = written;
	return 0;
}

static int pem_match(const char *data, size_t len, size_t pos,
					 const char *marker, size_t marker_len)
{
	size_t i;
	if (pos + marker_len > len)
		return 0;
	for (i = 0; i < marker_len; i++)
		if (data[pos + i] != marker[i])
			return 0;
	return 1;
}

int fun_crypto_pem_parse(const char *pem_data, size_t pem_len,
						 uint8_t *der_outputs[], size_t der_sizes[],
						 size_t *num_certs, size_t max_certs)
{
	size_t pos = 0;
	size_t count = 0;

	if (!pem_data || !der_outputs || !der_sizes || !num_certs)
		return -1;
	*num_certs = 0;

	while (pos < pem_len) {
		size_t body_start;
		size_t body_end;
		size_t der_len = 0;

		if (!pem_match(pem_data, pem_len, pos, PEM_BEGIN, PEM_BEGIN_LEN)) {
			pos++;
			continue;
		}

		body_start = pos + PEM_BEGIN_LEN;
		body_end = body_start;
		while (body_end < pem_len &&
			   !pem_match(pem_data, pem_len, body_end, PEM_END, PEM_END_LEN))
			body_end++;

		if (body_end >= pem_len)
			return -1; /* BEGIN without matching END */

		if (count >= max_certs)
			return -1;

		if (pem_b64_decode(pem_data + body_start, body_end - body_start,
						   der_outputs[count], der_sizes[count], &der_len) != 0)
			return -1;

		der_sizes[count] = der_len;
		count++;
		pos = body_end + PEM_END_LEN;
	}

	*num_certs = count;
	return (int)count;
}