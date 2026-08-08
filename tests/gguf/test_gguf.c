#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdint.h>

static int test_open(void)
{
	GGufFileHandleResult res =
		fun_gguf_open("../../models/openai_gpt-oss-20b-MXFP4.gguf");
	if (fun_error_is_error(res.error)) {
		fun_console_error_line("FAIL: open");
		return 1;
	}
	GGufFile *f = res.value;

	uint32_tResult blocks =
		fun_gguf_get_metadata_u32(f, "gpt-oss.block_count");
	uint32_tResult ctx =
		fun_gguf_get_metadata_u32(f, "gpt-oss.context_length");
	uint32_tResult experts =
		fun_gguf_get_metadata_u32(f, "gpt-oss.expert_count");

	if (blocks.value != 24 || ctx.value != 131072 ||
	    experts.value != 32) {
		fun_console_error_line("FAIL: metadata");
		fun_gguf_close(f);
		return 1;
	}

	uint64_tResult off = fun_gguf_get_tensor_offset(
		f, "blk.0.attn_q.weight");
	uint32_tResult tp =
		fun_gguf_get_tensor_type(f, "blk.0.attn_q.weight");

	if (fun_error_is_error(off.error) || off.value == 0 ||
	    tp.value != GGUF_TYPE_Q8_0) {
		fun_console_error_line("FAIL: tensor info");
		fun_gguf_close(f);
		return 1;
	}

	uint64_tResult sz =
		fun_gguf_get_tensor_size(f, "blk.0.attn_q.weight");
	uint64_t el_count = sz.value / 34 * 32;

	MemoryResult mem = fun_memory_allocate(el_count * sizeof(float));
	if (fun_error_is_error(mem.error)) {
		fun_console_error_line("FAIL: allocate");
		fun_gguf_close(f);
		return 1;
	}
	float *buf = (float *)mem.value;

	voidResult dq =
		fun_gguf_dequant_q8_0(f, "blk.0.attn_q.weight", buf);
	if (fun_error_is_error(dq.error)) {
		fun_console_error_line("FAIL: dequant Q8_0");
		fun_memory_free((Memory *)&buf);
		fun_gguf_close(f);
		return 1;
	}

	if (buf[0] == 0.0f && buf[1] == 0.0f) {
		fun_console_error_line("FAIL: all zeros after dequant");
		fun_memory_free((Memory *)&buf);
		fun_gguf_close(f);
		return 1;
	}

	{
		uint64_t nan = 0, huge = 0;
		for (uint64_t i = 0; i < el_count; i++) {
			if (buf[i] != buf[i])
				nan++;
			float a = buf[i] < 0 ? -buf[i] : buf[i];
			if (a > 1000.0f)
				huge++;
		}
		if (nan > 0 || huge > 0) {
			fun_console_error_line("FAIL: dequant garbage");
			fun_memory_free((Memory *)&buf);
			fun_gguf_close(f);
			return 1;
		}
	}

	fun_memory_free((Memory *)&buf);

	uint64_tResult mx_sz =
		fun_gguf_get_tensor_size(f, "blk.0.ffn_gate_exps.weight");
	uint64_t mx_el = mx_sz.value / 17 * 32;

	MemoryResult mx_mem = fun_memory_allocate(mx_el * sizeof(float));
	if (fun_error_is_error(mx_mem.error)) {
		fun_console_error_line("FAIL: allocate MXFP4");
		fun_gguf_close(f);
		return 1;
	}
	float *mx_buf = (float *)mx_mem.value;

	voidResult mx_dq = fun_gguf_dequant_mxfp4(
		f, "blk.0.ffn_gate_exps.weight", mx_buf);
	if (fun_error_is_error(mx_dq.error)) {
		fun_console_error_line("FAIL: dequant MXFP4");
		fun_memory_free((Memory *)&mx_buf);
		fun_gguf_close(f);
		return 1;
	}

	{
		uint64_t nan = 0, huge = 0, zero = 0;
		float mn = 1e30f, mx = -1e30f;
		for (uint64_t i = 0; i < mx_el; i++) {
			if (mx_buf[i] != mx_buf[i])
				nan++;
			else {
				if (mx_buf[i] == 0.0f)
					zero++;
				if (mx_buf[i] < mn)
					mn = mx_buf[i];
				if (mx_buf[i] > mx)
					mx = mx_buf[i];
			}
			float a = mx_buf[i] < 0 ? -mx_buf[i] : mx_buf[i];
			if (a > 1000.0f)
				huge++;
		}
		char sb[128];
		fun_console_write("    MXFP4 dbg: nan=");
		fun_string_from_int(nan, 10, sb, 128);
		fun_console_write(sb);
		fun_console_write(" huge=");
		fun_string_from_int(huge, 10, sb, 128);
		fun_console_write(sb);
		fun_console_write(" zero=");
		fun_string_from_int(zero, 10, sb, 128);
		fun_console_write(sb);
		fun_console_write(" min=");
		fun_string_from_double(mn, 3, sb, 128);
		fun_console_write(sb);
		fun_console_write(" max=");
		fun_string_from_double(mx, 3, sb, 128);
		fun_console_write_line(sb);
		if (nan > 0 || huge > 0) {
			fun_console_error_line("FAIL: MXFP4 dequant garbage");
			fun_memory_free((Memory *)&mx_buf);
			fun_gguf_close(f);
			return 1;
		}
	}

	fun_memory_free((Memory *)&mx_buf);

	fun_gguf_close(f);
	fun_console_write_line("PASS: all tests");
	return 0;
}

int main(void)
{
	return test_open();
}
