#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include "model.h"
#include "tokenizer.h"
#include <stdint.h>
#include <windows.h>

#define MAX_TOKENS 64
#define MAX_SEQ 256

int main(int argc, char **argv)
{
	if (argc < 2) {
		fun_console_write_line("Usage: demo.exe \"your prompt\"");
		return 1;
	}

	fun_math_init();

	LARGE_INTEGER freq, t0, t1;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&t0);

	fun_console_write_line("Loading model...");

	GGufFileHandleResult gr = fun_gguf_open(
		"../../models/openai_gpt-oss-20b-MXFP4.gguf");
	if (fun_error_is_error(gr.error)) {
		fun_console_error_line("Cannot open model file");
		return 1;
	}
	GGufFile *gguf = gr.value;

	Tokenizer tok;
	tokenizer_load(&tok, gguf);

	Model model;
	model_load(&model, gguf);

	QueryPerformanceCounter(&t1);
	double load_s = (double)(t1.QuadPart - t0.QuadPart) /
			(double)freq.QuadPart;

	char buf[256];
	MemoryResult num_mem = fun_memory_allocate(256);
	char *num_buf = (char *)num_mem.value;
	fun_string_from_double(load_s, 2, num_buf, 256);
	fun_console_write("  Loaded in ");
	fun_console_write(num_buf);
	fun_console_write_line(" seconds");

	char *prompt = argv[1];
	int tokens[MAX_SEQ];
	int n_tokens = tokenizer_encode(&tok, prompt, tokens, MAX_SEQ);

	fun_console_write("  Prompt tokens: ");
	fun_string_from_int(n_tokens, 10, num_buf, 256);
	fun_console_write_line(num_buf);

	fun_console_write("Prompt: ");
	fun_console_write_line(prompt);
	fun_console_write_line("");

	QueryPerformanceCounter(&t0);

	int generated = 0;
	while (generated < MAX_TOKENS) {
		fun_console_write(".");
		fun_console_flush();
		float *logits = (float *)fun_memory_allocate(
					201088 * sizeof(float))
					.value;
		model_forward(&model, tokens, n_tokens, logits);

		int next_id = 0;
		float max_l = logits[0];
		for (int i = 1; i < 201088; i++) {
			if (logits[i] > max_l) {
				max_l = logits[i];
				next_id = i;
			}
		}
		fun_memory_free((Memory *)&logits);

		if (next_id == tok.eos_id)
			break;

		tokens[n_tokens++] = next_id;
		generated++;

		char token_str[32];
		tokenizer_decode(&tok, next_id, token_str, 32);
		fun_console_write(token_str);
	}

	QueryPerformanceCounter(&t1);
	double eval_s = (double)(t1.QuadPart - t0.QuadPart) /
			(double)freq.QuadPart;

	fun_console_write_line("");
	fun_console_write_line("");

	fun_console_write("  Time: ");
	fun_string_from_double(eval_s, 2, num_buf, 256);
	fun_console_write(num_buf);
	fun_console_write("s  Tokens: ");
	fun_string_from_int(generated, 10, num_buf, 256);
	fun_console_write(num_buf);
	fun_console_write("  tok/s: ");
	double tps = eval_s > 0.0 ? (double)generated / eval_s : 0.0;
	fun_string_from_double(tps, 2, num_buf, 256);
	fun_console_write_line(num_buf);

	model_free(&model);
	fun_gguf_close(gguf);
	fun_memory_free((Memory *)&num_buf);

	return 0;
}
