#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdint.h>

static uint32_t le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
		   ((uint32_t)p[3] << 24);
}

static uint64_t le64(const uint8_t *p)
{
	return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

static float half_to_float(uint16_t h)
{
	uint32_t sign = (h >> 15) << 31;
	uint32_t exp = (h >> 10) & 0x1F;
	uint32_t mant = h & 0x3FF;

	if (exp == 0) {
		if (mant == 0) {
			uint32_t zero = sign;
			return *(float *)&zero;
		}
		int e = 0;
		while (!(mant & 0x400)) {
			mant <<= 1;
			e++;
		}
		mant &= 0x3FF;
		uint32_t f32 = sign | ((uint32_t)(113 - e) << 23) | (mant << 13);
		return *(float *)&f32;
	}
	if (exp == 31) {
		uint32_t nan_bits = 0x7FC00000;
		uint32_t inf_bits = 0x7F800000;
		return mant ? *(float *)&nan_bits :
					  (sign ? -*(float *)&inf_bits : *(float *)&inf_bits);
	}
	uint32_t f32 = sign | ((exp + 112) << 23) | (mant << 13);
	return *(float *)&f32;
}

static int name_is(const char *nm, uint64_t nlen, String want)
{
	uint64_t wl = fun_string_length(want);
	if (wl != nlen)
		return 0;
	for (uint64_t i = 0; i < nlen; i++)
		if ((uint8_t)nm[i] != (uint8_t)want[i])
			return 0;
	return 1;
}

static void print_u64(String label, uint64_t v)
{
	char buf[64];
	fun_console_write("    ");
	fun_console_write(label);
	fun_console_write(": ");
	fun_string_from_int((int64_t)v, 10, buf, 64);
	fun_console_write_line(buf);
}

static void print_q8(const uint8_t *base, uint64_t abs_off, String label)
{
	char buf[256];
	float d =
		half_to_float((uint16_t)(base[abs_off] | (base[abs_off + 1] << 8)));
	int8_t q0 = (int8_t)base[abs_off + 2];
	int8_t q1 = (int8_t)base[abs_off + 3];
	fun_console_write("    ");
	fun_console_write(label);
	fun_console_write(" scale=");
	fun_string_from_double(d, 6, buf, 256);
	fun_console_write(buf);
	fun_console_write(" q[0]=");
	fun_string_from_int(q0, 10, buf, 256);
	fun_console_write(buf);
	fun_console_write(" q[1]=");
	fun_string_from_int(q1, 10, buf, 256);
	fun_console_write_line(buf);
}

int main(void)
{
	char buf[256];
	fun_console_write_line("GGUF probe");

	GGufFileHandleResult gr =
		fun_gguf_open("../../models/openai_gpt-oss-20b-MXFP4.gguf");
	if (fun_error_is_error(gr.error)) {
		fun_console_error_line("Cannot open model file");
		return 1;
	}
	GGufFile *gguf = gr.value;

	const uint8_t *data_start_ptr = fun_gguf_get_raw_data(gguf);
	uint64_t total_size = fun_gguf_get_raw_size(gguf);
	print_u64("file size", total_size);

	const uint8_t *data = fun_gguf_get_file_base(gguf);
	uint64_t fsize = fun_gguf_get_raw_size(gguf);
	(void)data_start_ptr;

	const uint8_t *p = data;
	uint32_t magic = le32(p);
	p += 4;
	uint32_t version = le32(p);
	p += 4;
	uint64_t n_tensors = le64(p);
	p += 8;
	uint64_t n_kv = le64(p);
	p += 8;

	fun_console_write("    magic=0x");
	fun_string_from_int(magic, 16, buf, 256);
	fun_console_write_line(buf);
	print_u64("version", version);
	print_u64("tensors", n_tensors);
	print_u64("kv", n_kv);

	uint64_t data_align = 0;
	for (uint64_t i = 0; i < n_kv; i++) {
		uint64_t klen = le64(p);
		p += 8;
		const char *key = (const char *)p;
		p += klen;
		if (name_is(key, klen, "general.alignment")) {
			uint32_t kvtype = le32(p);
			if (kvtype == 4)
				data_align = le32(p + 4);
		}
		uint32_t vt = le32(p);
		p += 4;
		switch (vt) {
		case 0:
		case 1:
		case 7:
			p += 1;
			break;
		case 2:
		case 3:
			p += 2;
			break;
		case 4:
		case 5:
		case 6:
			p += 4;
			break;
		case 8: {
			uint64_t sl = le64(p);
			p += 8 + sl;
			break;
		}
		case 9: {
			uint32_t et = le32(p);
			p += 4;
			uint64_t cnt = le64(p);
			p += 8;
			for (uint64_t j = 0; j < cnt; j++) {
				if (et == 8) {
					uint64_t sl = le64(p);
					p += 8 + sl;
				} else if (et == 0 || et == 1 || et == 7)
					p += 1;
				else if (et == 2 || et == 3)
					p += 2;
				else if (et == 4 || et == 5 || et == 6)
					p += 4;
				else
					p += 8;
			}
			break;
		}
		default:
			p += 8;
			break;
		}
	}
	print_u64("data_align key", data_align);

	print_u64("kv section ends (file offset)", (uint64_t)(p - data));

	while (*p == 0) {
		p++;
		if (p >= data + fsize)
			break;
	}
	uint64_t tmeta_off = (uint64_t)(p - data);
	print_u64("tensor metadata starts (file offset)", tmeta_off);

	uint64_t emb_off = 0, out_off = 0, emb_size = 0, out_size = 0;
	uint32_t emb_type = 0, out_type = 0;
	for (uint64_t i = 0; i < n_tensors; i++) {
		uint64_t nlen = le64(p);
		p += 8;
		const char *nm = (const char *)p;
		p += nlen;
		uint32_t ndims = le32(p);
		p += 4;
		uint64_t total = 1;
		for (uint32_t d = 0; d < ndims; d++) {
			total *= le64(p);
			p += 8;
		}
		uint32_t ty = le32(p);
		p += 4;
		uint64_t off = le64(p);
		p += 8;
		uint64_t sz;
		if (ty == 8)
			sz = (total + 31) / 32 * 34;
		else if (ty == 39)
			sz = (total + 31) / 32 * 17;
		else if (ty == 0)
			sz = total * 4;
		else
			sz = total * 2;
		if (i < 3) {
			fun_console_write("    tensor[");
			fun_string_from_int((int64_t)i, 10, buf, 256);
			fun_console_write(buf);
			fun_console_write("] len=");
			fun_string_from_int((int64_t)nlen, 10, buf, 256);
			fun_console_write(buf);
			fun_console_write(" name='");
			fun_console_write(nm);
			fun_console_write("' ");
			for (uint64_t bi = 0; bi < nlen; bi++) {
				fun_string_from_int((int64_t)nm[bi] & 0xFF, 16, buf, 256);
				fun_console_write(buf);
				fun_console_write(" ");
			}
			fun_console_write_line("");
		}
		for (uint64_t bi = 0; bi + 4 < nlen && nlen > 6; bi++) {
			if (nm[bi] == 'b' && nm[bi + 1] == 'l' && nm[bi + 2] == 'k' &&
				nm[bi + 3] == '.' && nm[bi + 4] == '0' && nm[bi + 5] == '.') {
				fun_console_write("    tensor[");
				fun_string_from_int((int64_t)i, 10, buf, 256);
				fun_console_write(buf);
				fun_console_write("] type=");
				fun_string_from_int((int64_t)ty, 10, buf, 256);
				fun_console_write(buf);
				fun_console_write(" name='");
				fun_console_write(nm);
				fun_console_write_line("'");
				break;
			}
		}
		if (name_is(nm, nlen, "token_embd.weight")) {
			emb_off = off;
			emb_size = sz;
			emb_type = ty;
		}
		if (name_is(nm, nlen, "output.weight")) {
			out_off = off;
			out_size = sz;
			out_type = ty;
		}
	}
	uint64_t tmeta_end = (uint64_t)(p - data);
	print_u64("tensor metadata ends (file offset)", tmeta_end);
	print_u64("token_embd.off", emb_off);
	print_u64("token_embd.size", emb_size);
	print_u64("token_embd.type", emb_type);
	print_u64("output.off", out_off);
	print_u64("output.size", out_size);
	print_u64("output.type", out_type);

	uint64_t align = data_align ? data_align : 32;
	uint64_t aligned_start = (tmeta_end + align - 1) / align * align;
	print_u64("aligned_start (meta_end aligned to ALIGN)", aligned_start);

	uint64_t zero_start = tmeta_end;
	while (zero_start < fsize && data[zero_start] == 0)
		zero_start++;
	print_u64("after-zero data start (file offset)", zero_start);

	fun_console_write_line("Q8 at file_offset = data_base + blob.off:");
	print_q8(data, zero_start + emb_off, "emb at zero_start+off");
	print_q8(data, aligned_start + emb_off, "emb at aligned+off");
	print_q8(data, aligned_start + out_off, "out at aligned+off");

	uint16_t rd = (uint16_t)(data[aligned_start + emb_off] |
							 (data[aligned_start + emb_off + 1] << 8));
	fun_console_write("    raw half at aligned+emb: 0x");
	fun_string_from_int((int64_t)rd, 16, buf, 256);
	fun_console_write_line(buf);

	fun_gguf_close(gguf);
	return 0;
}