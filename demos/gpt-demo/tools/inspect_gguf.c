#include "fundamental/console/console.h"
#include "fundamental/file/file.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdint.h>

#define HEADER_READ_SIZE (16 * 1024 * 1024)
#define GGUF_MAGIC 0x46554747

static uint32_t read_le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
		   ((uint32_t)p[3] << 24);
}

static uint64_t read_le64(const uint8_t *p)
{
	return (uint64_t)read_le32(p) | ((uint64_t)read_le32(p + 4) << 32);
}

static uint64_t read_str_len(const uint8_t **pp, const uint8_t *end)
{
	const uint8_t *p = *pp;
	if (p + 8 > end)
		return 0;
	uint64_t len = read_le64(p);
	if (p + 8 + len > end)
		return 0;
	*pp = p + 8;
	return len;
}

static int skip_kv_val(const uint8_t **pp, const uint8_t *end, uint32_t vtype)
{
	const uint8_t *p = *pp;
	switch (vtype) {
	case 0:
		p += 1;
		break;
	case 1:
		p += 1;
		break;
	case 2:
		p += 2;
		break;
	case 3:
		p += 2;
		break;
	case 4:
		p += 4;
		break;
	case 5:
		p += 4;
		break;
	case 6:
		p += 4;
		break;
	case 7:
		p += 1;
		break;
	case 8: {
		uint64_t l = read_str_len(pp, end);
		p = *pp + l;
		*pp = p;
		return 0;
	}
	case 9: {
		uint32_t et = read_le32(p);
		p += 4;
		uint64_t n = read_le64(p);
		p += 8;
		if (et == 8) {
			*pp = p;
			for (uint64_t j = 0; j < n; j++) {
				uint64_t sl = read_str_len(pp, end);
				if (sl == 0 && *pp + 8 + 10 > end)
					break;
				*pp += sl;
			}
			return 0;
		}
		size_t sz = (et == 0 || et == 1 || et == 7)	   ? 1 :
					(et == 2 || et == 3)			   ? 2 :
					(et == 4 || et == 5 || et == 6)	   ? 4 :
					(et == 10 || et == 11 || et == 12) ? 8 :
														 0;
		p += n * sz;
		break;
	}
	case 10:
		p += 8;
		break;
	case 11:
		p += 8;
		break;
	case 12:
		p += 8;
		break;
	default:
		break;
	}
	*pp = p;
	return 0;
}

static void write_u64(char *buf, size_t buf_size, uint64_t v)
{
	fun_string_from_int((int64_t)v, 10, buf, buf_size);
}

int main(void)
{
	MemoryResult mem = fun_memory_allocate(HEADER_READ_SIZE);
	if (fun_error_is_error(mem.error)) {
		fun_console_error_line("Failed to allocate memory");
		return 1;
	}
	Memory header = mem.value;

	AsyncResult read_result = fun_read_file_in_memory((Read){
		.file_path = "../../../models/openai_gpt-oss-20b-MXFP4.gguf",
		.output = header,
		.bytes_to_read = HEADER_READ_SIZE,
		.offset = 0,
	});
	fun_async_await(&read_result, -1);

	if (read_result.status != ASYNC_COMPLETED) {
		fun_console_error_line("Failed to read file");
		fun_memory_free(&header);
		return 1;
	}

	const uint8_t *p = (const uint8_t *)header;
	const uint8_t *end = p + HEADER_READ_SIZE;

	uint32_t magic = read_le32(p);
	p += 4;
	if (magic != GGUF_MAGIC) {
		fun_console_error_line("Not a GGUF file");
		fun_memory_free(&header);
		return 1;
	}

	uint32_t version = read_le32(p);
	p += 4;
	char version_buf[32];
	fun_string_from_int(version, 10, version_buf, 32);

	uint64_t tensor_count = read_le64(p);
	p += 8;
	uint64_t kv_count = read_le64(p);
	p += 8;

	fun_console_write_line("");
	char count_buf[64];

	fun_console_write("Version: ");
	fun_console_write_line(version_buf);

	write_u64(count_buf, 64, tensor_count);
	fun_console_write("Tensors: ");
	fun_console_write_line(count_buf);

	write_u64(count_buf, 64, kv_count);
	fun_console_write("Metadata KV: ");
	fun_console_write_line(count_buf);

	fun_console_write_line("");
	fun_console_write_line("=== METADATA ===");

	for (uint64_t i = 0; i < kv_count; i++) {
		uint64_t klen = read_str_len(&p, end);
		if (klen == 0 || p + klen > end)
			break;
		const uint8_t *kstart = p;
		p += klen;

		uint32_t vtype = read_le32(p);
		p += 4;

		char kb[256];
		int kcopy = klen < 255 ? (int)klen : 255;
		for (int j = 0; j < kcopy; j++)
			kb[j] = (char)kstart[j];
		kb[kcopy] = '\0';

		if (vtype > 12) {
			skip_kv_val(&p, end, vtype);
			char nb[32];
			fun_string_from_int(vtype, 10, nb, 32);
			fun_console_write("  [");
			fun_console_write(kb);
			fun_console_write("]  type=");
			fun_console_write(nb);
			fun_console_write_line(" <weird, skipped>");
			continue;
		}

		fun_console_write("  ");
		fun_console_write(kb);

		if (vtype == 8) {
			uint64_t vlen = read_str_len(&p, end);
			const uint8_t *vstart = p;
			p += vlen;
			char vb[1024];
			int vcopy = vlen < 1023 ? (int)vlen : 1023;
			for (int j = 0; j < vcopy; j++)
				vb[j] = (char)vstart[j];
			vb[vcopy] = '\0';
			fun_console_write(" = ");
			fun_console_write_line(vb);
		} else if (vtype == 4) {
			uint32_t v = read_le32(p);
			p += 4;
			char nb[32];
			fun_string_from_int(v, 10, nb, 32);
			fun_console_write(" = ");
			fun_console_write_line(nb);
		} else if (vtype == 6) {
			p += 4;
			fun_console_write_line(" = <float32>");
		} else if (vtype == 7) {
			p += 1;
			fun_console_write_line(" = <bool>");
		} else if (vtype == 9) {
			uint32_t et = read_le32(p);
			p += 4;
			uint64_t n = read_le64(p);
			p += 8;
			if (et == 8) {
				char nb[64];
				write_u64(nb, 64, n);
				fun_console_write(" = ARRAY[");
				fun_console_write(nb);
				fun_console_write_line("] of STRING");
				for (uint64_t j = 0; j < n; j++) {
					uint64_t slen = read_str_len(&p, end);
					if (slen == 0 && p + 8 + 10 > end)
						break;
					p += slen;
				}
			} else {
				size_t sz = (et == 0 || et == 1 || et == 7)	   ? 1 :
							(et == 2 || et == 3)			   ? 2 :
							(et == 4 || et == 5 || et == 6)	   ? 4 :
							(et == 10 || et == 11 || et == 12) ? 8 :
																 0;
				p += n * sz;
				fun_console_write_line(" = <array, skipped>");
			}
		} else {
			skip_kv_val(&p, end, vtype);
			fun_console_write_line(" = <skipped>");
		}
	}

	fun_console_write_line("");
	{
		char pos_buf[64];
		write_u64(pos_buf, 64, (uint64_t)(p - (const uint8_t *)header));
		fun_console_write("Position after KV: ");
		fun_console_write_line(pos_buf);
	}

	fun_console_write_line("");
	fun_console_write_line("=== TENSORS ===");

	{
		uint64_t tensor_base = (uint64_t)(p - (const uint8_t *)header);
		char pos_buf[64];
		write_u64(pos_buf, 64, tensor_base);
		fun_console_write("Tensor section starts at byte: ");
		fun_console_write_line(pos_buf);
		write_u64(pos_buf, 64, (uint64_t)(end - p));
		fun_console_write("Bytes remaining: ");
		fun_console_write_line(pos_buf);

		if (p < end && *p == 0) {
			uint64_t pad = 0;
			while (p < end && *p == 0) {
				p++;
				pad++;
			}
			write_u64(pos_buf, 64, pad);
			fun_console_write("Zero padding skipped: ");
			fun_console_write_line(pos_buf);
		}
		write_u64(pos_buf, 64, read_le64(p));
		fun_console_write("Next 8 bytes as u64: ");
		fun_console_write_line(pos_buf);
	}

	for (uint64_t i = 0; i < tensor_count; i++) {
		uint64_t nlen = read_str_len(&p, end);
		if (nlen == 0)
			break;
		const uint8_t *nstart = p;
		p += nlen;

		char name[256];
		int ncopy = nlen < 255 ? (int)nlen : 255;
		for (int j = 0; j < ncopy; j++)
			name[j] = (char)nstart[j];
		name[ncopy] = '\0';

		uint32_t ndims = read_le32(p);
		p += 4;
		for (uint32_t d = 0; d < ndims; d++) {
			p += 8;
		}
		uint32_t ttype = read_le32(p);
		p += 4;
		uint64_t offset = read_le64(p);
		p += 8;

		char ob[64];
		write_u64(ob, 64, offset);

		char tb[32];
		fun_string_from_int(ttype, 10, tb, 32);

		fun_console_write("  ");
		fun_console_write(name);
		fun_console_write("  type=");
		fun_console_write(tb);
		fun_console_write("  offset=");
		fun_console_write_line(ob);
	}

	fun_memory_free(&header);

	return 0;
}
