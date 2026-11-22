/*
 * pem-inspect — read a PEM file and print the X.509 certificate details.
 *
 * Demonstrates the crypto module's PEM and X.509 parsers together with
 * file I/O, filesystem and console modules.
 *
 * Usage: pem-inspect [path/to/certs.pem]   (defaults to sample.pem)
 */
#include "fundamental/async/async.h"
#include "fundamental/console/console.h"
#include "fundamental/crypto/crypto.h"
#include "fundamental/file/file.h"
#include "fundamental/filesystem/filesystem.h"
#include "fundamental/filesystem/path.h"
#include "fundamental/memory/memory.h"
#include "fundamental/timing/timing.h"

#define DEMO_MAX_CERTS 128
#define DEMO_MAX_DER 8192
#define DEMO_MAX_FILE (4 * 1024 * 1024)

static size_t append_str(char *buf, size_t pos, const char *s)
{
	while (*s && pos < 500)
		buf[pos++] = *s++;
	buf[pos] = '\0';
	return pos;
}

static size_t append_uint(char *buf, size_t pos, uint64_t value)
{
	char tmp[20];
	size_t n = 0;
	size_t i;

	if (value == 0) {
		buf[pos++] = '0';
		buf[pos] = '\0';
		return pos;
	}
	while (value > 0 && n < sizeof(tmp)) {
		tmp[n++] = (char)('0' + (value % 10));
		value /= 10;
	}
	for (i = 0; i < n; i++)
		buf[pos++] = tmp[n - 1 - i];
	buf[pos] = '\0';
	return pos;
}

static size_t append_padded(char *buf, size_t pos, uint64_t value, int width)
{
	char tmp[20];
	int n = 0;
	int i;

	while (value > 0 && n < 20) {
		tmp[n++] = (char)('0' + (value % 10));
		value /= 10;
	}
	while (n < width && n < 20) {
		tmp[n++] = '0';
	}
	for (i = n - 1; i >= 0; i--)
		buf[pos++] = tmp[i];
	buf[pos] = '\0';
	return pos;
}

/* days since the Unix epoch -> proleptic Gregorian date (Howard Hinnant). */
static void append_utc_time(char *buf, size_t *pos, int64_t unix_seconds)
{
	int64_t days = unix_seconds / 86400;
	int64_t secs = unix_seconds % 86400;
	int64_t z;
	int64_t era;
	unsigned doe;
	unsigned yoe;
	int64_t year;
	unsigned doy;
	unsigned mp;
	unsigned day;
	unsigned month;

	if (secs < 0) {
		secs += 86400;
		days--;
	}

	z = days + 719468;
	era = (z >= 0 ? z : z - 146096) / 146097;
	doe = (unsigned)(z - era * 146097);
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	year = (int64_t)yoe + era * 400;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	mp = (5 * doy + 2) / 153;
	day = doy - (153 * mp + 2) / 5 + 1;
	month = mp + (mp < 10 ? 3 : -9);
	year += (month <= 2);

	*pos = append_padded(buf, *pos, (uint64_t)year, 4);
	*pos = append_str(buf, *pos, "-");
	*pos = append_padded(buf, *pos, month, 2);
	*pos = append_str(buf, *pos, "-");
	*pos = append_padded(buf, *pos, day, 2);
	*pos = append_str(buf, *pos, " ");
	*pos = append_padded(buf, *pos, (uint64_t)(secs / 3600), 2);
	*pos = append_str(buf, *pos, ":");
	*pos = append_padded(buf, *pos, (uint64_t)((secs % 3600) / 60), 2);
	*pos = append_str(buf, *pos, ":");
	*pos = append_padded(buf, *pos, (uint64_t)(secs % 60), 2);
	*pos = append_str(buf, *pos, " UTC");
}

static size_t count_pem_blocks(const char *data, size_t len)
{
	static const char marker[] = "-----BEGIN CERTIFICATE-----";
	const size_t marker_len = sizeof(marker) - 1;
	size_t count = 0;
	size_t i;

	for (i = 0; i + marker_len <= len; i++) {
		size_t j = 0;
		while (j < marker_len && data[i + j] == marker[j])
			j++;
		if (j == marker_len) {
			count++;
			i += marker_len - 1;
		}
	}
	return count;
}

static int read_whole_file(const char *path, Memory *out, uint64_t *out_size)
{
	char path_buf[512];
	const char *components[64];
	Path parsed = { components, 0, false };
	uint64_t size = 0;
	voidResult size_result;
	MemoryResult mem_result;
	Read read;
	AsyncResult read_result;
	uint8_t *bytes;

	if (fun_error_is_error(
			fun_path_from_cstr(path, path_buf, sizeof(path_buf), &parsed))) {
		fun_console_error_line("Invalid path");
		return -1;
	}

	size_result = fun_file_size(parsed, &size);
	if (fun_error_is_error(size_result.error)) {
		fun_console_error_line("Cannot stat file");
		return -1;
	}
	if (size == 0) {
		fun_console_error_line("File is empty");
		return -1;
	}
	if (size > DEMO_MAX_FILE) {
		fun_console_error_line("File is larger than the 4 MiB demo limit");
		return -1;
	}

	mem_result = fun_memory_allocate((size_t)size + 1);
	if (fun_error_is_error(mem_result.error)) {
		fun_console_error_line("Out of memory");
		return -1;
	}

	read = (Read){ .file_path = path,
				   .output = mem_result.value,
				   .bytes_to_read = size,
				   .offset = 0,
				   .mode = FILE_MODE_AUTO };
	read_result = fun_read_file_in_memory(read);
	fun_async_await(&read_result, -1);
	if (fun_error_is_error(read_result.error)) {
		fun_console_error_line("Failed to read file");
		fun_memory_free(&mem_result.value);
		return -1;
	}

	bytes = (uint8_t *)mem_result.value;
	bytes[size] = '\0';
	*out = mem_result.value;
	*out_size = size;
	return 0;
}

static void print_common_name(char *line, size_t pos, const char *label,
							  const char *cn, size_t cn_len)
{
	size_t i;

	pos = append_str(line, pos, label);
	if (!cn) {
		pos = append_str(line, pos, "(not present)");
	} else {
		for (i = 0; i < cn_len && pos < 490; i++)
			line[pos++] = cn[i];
		line[pos] = '\0';
	}
	fun_console_write_line(line);
}

static void print_certificate(const fun_crypto_x509_cert *cert, size_t index,
							  size_t total)
{
	char line[512];
	char digest[32];
	size_t pos;
	size_t i;

	pos = append_str(line, 0, "[");
	pos = append_uint(line, pos, index + 1);
	pos = append_str(line, pos, "/");
	pos = append_uint(line, pos, total);
	pos = append_str(line, pos, "]");
	fun_console_write_line(line);

	print_common_name(line, 0, "  Subject:       ", cert->common_name,
					  cert->common_name_len);
	print_common_name(line, 0, "  Issuer:        ", cert->issuer_common_name,
					  cert->issuer_common_name_len);

	pos = append_str(line, 0, "  Valid from:    ");
	append_utc_time(line, &pos, cert->not_before);
	fun_console_write_line(line);

	pos = append_str(line, 0, "  Valid until:   ");
	append_utc_time(line, &pos, cert->not_after);
	fun_console_write_line(line);

	if (!cert->has_basic_constraints) {
		fun_console_write_line("  CA:            not specified");
	} else if (!cert->is_ca) {
		fun_console_write_line("  CA:            no");
	} else if (cert->path_length >= 0) {
		pos = append_str(line, 0, "  CA:            yes (pathlen ");
		pos = append_uint(line, pos, (uint64_t)cert->path_length);
		pos = append_str(line, pos, ")");
		fun_console_write_line(line);
	} else {
		fun_console_write_line("  CA:            yes");
	}

	pos = append_str(line, 0, "  SAN DNS:       ");
	if (!cert->has_san) {
		pos = append_str(line, pos, "(no SAN extension)");
	} else if (cert->num_san_dns == 0) {
		pos = append_str(line, pos, "(none)");
	} else {
		for (i = 0; i < cert->num_san_dns; i++) {
			size_t j;
			if (i > 0)
				pos = append_str(line, pos, ", ");
			for (j = 0; j < cert->san_dns_len[i] && pos < 490; j++)
				line[pos++] = cert->san_dns[i][j];
			line[pos] = '\0';
		}
	}
	fun_console_write_line(line);

	if (cert->public_key_valid) {
		fun_console_write_line("  Public key:    EC P-256");
	} else {
		fun_console_write_line("  Public key:    unsupported");
	}

	if (cert->signature_algorithm == FUN_CRYPTO_X509_SIG_ECDSA_SHA256) {
		fun_console_write_line("  Signature:     ecdsa-with-SHA256");
	} else {
		fun_console_write_line("  Signature:     unknown");
	}

	pos = append_str(line, 0, "  DER size:      ");
	pos = append_uint(line, pos, (uint64_t)cert->der_len);
	pos = append_str(line, pos, " bytes");
	fun_console_write_line(line);

	fun_crypto_sha256(cert->der, cert->der_len, (uint8_t *)digest);
	pos = append_str(line, 0, "  SHA-256:       ");
	for (i = 0; i < sizeof(digest); i++) {
		static const char hex[] = "0123456789abcdef";
		if (i > 0)
			line[pos++] = ':';
		line[pos++] = hex[((uint8_t)digest[i]) >> 4];
		line[pos++] = hex[((uint8_t)digest[i]) & 0x0F];
	}
	line[pos] = '\0';
	fun_console_write_line(line);
	fun_console_write_line("");
}

int main(int argc, char **argv)
{
	const char *path = argc > 1 ? argv[1] : "sample.pem";
	Memory file_data = NULL;
	uint64_t file_size = 0;
	Memory der_arena = NULL;
	uint8_t *der_outputs[DEMO_MAX_CERTS];
	size_t der_sizes[DEMO_MAX_CERTS];
	size_t block_count;
	size_t cert_count = 0;
	size_t i;
	int parsed;

	fun_console_write_line("pem-inspect — PEM / X.509 Certificate Inspector");
	fun_console_write_line("================================================");

	if (read_whole_file(path, &file_data, &file_size) != 0)
		return 1;

	fun_console_write("File: ");
	fun_console_write_line(path);
	{
		char line[128];
		size_t pos = append_str(line, 0, "Size: ");
		pos = append_uint(line, pos, file_size);
		pos = append_str(line, pos, " bytes");
		fun_console_write_line(line);
	}

	block_count = count_pem_blocks((const char *)file_data, (size_t)file_size);
	if (block_count == 0) {
		fun_console_error_line("No 'BEGIN CERTIFICATE' blocks found");
		fun_memory_free(&file_data);
		return 1;
	}
	if (block_count > DEMO_MAX_CERTS) {
		fun_console_error_line("Too many certificates for this demo");
		fun_memory_free(&file_data);
		return 1;
	}

	{
		MemoryResult arena = fun_memory_allocate(block_count * DEMO_MAX_DER);
		if (fun_error_is_error(arena.error)) {
			fun_console_error_line("Out of memory");
			fun_memory_free(&file_data);
			return 1;
		}
		der_arena = arena.value;
	}

	for (i = 0; i < block_count; i++) {
		der_outputs[i] = (uint8_t *)der_arena + i * DEMO_MAX_DER;
		der_sizes[i] = DEMO_MAX_DER;
	}

	parsed = fun_crypto_pem_parse((const char *)file_data, (size_t)file_size,
								  der_outputs, der_sizes, &cert_count,
								  block_count);
	if (parsed < 0) {
		fun_console_error_line(
			"PEM parsing failed (malformed PEM or certificate too large)");
		fun_memory_free(&der_arena);
		fun_memory_free(&file_data);
		return 1;
	}

	{
		char line[128];
		size_t pos = append_str(line, 0, "Certificates: ");
		pos = append_uint(line, pos, cert_count);
		fun_console_write_line(line);
	}
	fun_console_write_line("");

	for (i = 0; i < cert_count; i++) {
		fun_crypto_x509_cert cert;
		int result = fun_crypto_x509_parse(der_outputs[i], der_sizes[i], &cert);
		if (result != 0) {
			char line[128];
			size_t pos = append_str(line, 0, "[");
			pos = append_uint(line, pos, i + 1);
			pos = append_str(line, pos, "] parse error (code ");
			pos = append_uint(line, pos, (uint64_t)result);
			pos = append_str(line, pos, ")");
			fun_console_write_line(line);
			fun_console_write_line("");
			continue;
		}
		print_certificate(&cert, i, cert_count);
	}

	fun_console_write_line("Done.");
	fun_console_flush();

	fun_memory_free(&der_arena);
	fun_memory_free(&file_data);
	return 0;
}