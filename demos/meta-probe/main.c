#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/string/string.h"
#include <stddef.h>
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

static uint16_t le16(const uint8_t *p)
{
	return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int main(void)
{
	GGufFileHandleResult gr =
		fun_gguf_open("../../models/openai_gpt-oss-20b-MXFP4.gguf");
	if (fun_error_is_error(gr.error)) {
		fun_console_error_line("open failed");
		return 1;
	}
	GGufFile *f = gr.value;

	const uint8_t *p = fun_gguf_get_file_base(f);
	uint64_t n_kv = le64(p + 16);
	uint64_t kv_start = le64(p + 8) > 0 ? 0 : 0;
	(void)kv_start;
	p += 24;

	char msg[256];
	for (uint64_t i = 0; i < n_kv; i++) {
		uint64_t klen = le64(p);
		p += 8;
		const char *key = (const char *)p;
		p += klen;
		uint32_t vt = le32(p);
		p += 4;

		fun_console_write("    [");
		fun_string_from_int((int64_t)i, 10, msg, sizeof(msg));
		fun_console_write(msg);
		fun_console_write("] ");
		for (uint64_t j = 0; j < klen; j++) {
			msg[j] = (key[j] >= 32 && key[j] < 127) ? key[j] : '?';
		}
		msg[klen] = '\0';
		fun_console_write(msg);
		fun_console_write(" type=");
		fun_string_from_int((int64_t)vt, 10, msg, sizeof(msg));
		fun_console_write(msg);

		switch (vt) {
		case 0:
		case 1:
		case 7:
			fun_console_write(" val=");
			fun_string_from_int((int64_t)*(const int8_t *)p, 10, msg,
								sizeof(msg));
			fun_console_write_line(msg);
			p += 1;
			break;
		case 2:
		case 3:
			fun_console_write(" val=");
			fun_string_from_int(le16(p), 10, msg, sizeof(msg));
			fun_console_write_line(msg);
			p += 2;
			break;
		case 4:
		case 5:
			fun_console_write(" val=");
			fun_string_from_int((int64_t)le32(p), 10, msg, sizeof(msg));
			fun_console_write_line(msg);
			p += 4;
			break;
		case 6: {
			uint32_t b;
			fun_console_write(" val=0x");
			fun_string_from_int((int64_t)le32(p), 16, msg, sizeof(msg));
			fun_console_write(msg);
			p += 4;
			fun_console_write_line("");
			break;
		}
		case 8: {
			uint64_t sl = le64(p);
			p += 8;
			fun_console_write(" val='");
			for (uint64_t j = 0; j < sl; j++)
				if (p[j] >= 32 && p[j] < 127)
					msg[j] = (char)p[j];
				else
					msg[j] = '?';
			msg[sl] = '\0';
			fun_console_write(msg);
			fun_console_write_line("'");
			p += sl;
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
			fun_console_write_line("");
			break;
		}
		default:
			fun_console_write_line("");
			p += 8;
			break;
		}
	}

	fun_gguf_close(f);
	return 0;
}