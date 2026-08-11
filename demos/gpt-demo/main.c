#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include "fundamental/timing/timing.h"
#include "fundamental/trace/trace.h"
#include "model.h"
#include "tokenizer.h"

#define MAX_TOKENS 64
#define MAX_SEQ 256

static void _trace_output(const char *line)
{
	fun_console_write_line(line);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fun_console_write_line("Usage: demo.exe \"your prompt\" [--trace]");
		return 1;
	}

	fun_math_init();

	char *prompt = argv[1];
	bool do_trace = (argc > 2 && fun_string_compare(argv[2], "--trace") == 0);

	double t0 = (double)fun_timing_now_ns() / 1e9;

	fun_console_write_line("Loading model...");

	GGufFileHandleResult gr =
		fun_gguf_open("../../models/openai_gpt-oss-20b-MXFP4.gguf");
	if (fun_error_is_error(gr.error)) {
		fun_console_error_line("Cannot open model file");
		return 1;
	}
	GGufFile *gguf = gr.value;

	Tokenizer tok;
	tokenizer_load(&tok, gguf);

	size_t trace_sz = fun_trace_memory_required(16);
	Memory trace_mem = fun_memory_allocate(trace_sz).value;
	if (do_trace)
		fun_trace_init(trace_mem, trace_sz, 16);

	Model model;
	model_load(&model, gguf, "../../models/openai_gpt-oss-20b-MXFP4.gguf");

	double t1 = (double)fun_timing_now_ns() / 1e9;
	double load_s = t1 - t0;

	char buf[256];
	MemoryResult num_mem = fun_memory_allocate(256);
	char *num_buf = (char *)num_mem.value;
	fun_string_from_double(load_s, 2, num_buf, 256);
	fun_console_write("  Loaded in ");
	fun_console_write(num_buf);
	fun_console_write_line(" seconds");

	int tokens[MAX_SEQ];
	int n_tokens = tokenizer_encode(&tok, prompt, tokens, MAX_SEQ);

	if (n_tokens == 0) {
		fun_console_error_line("  Empty prompt after tokenization");
		return 1;
	}

	fun_console_write("  Prompt tokens: ");
	fun_string_from_int(n_tokens, 10, num_buf, 256);
	fun_console_write_line(num_buf);

	fun_console_write("Prompt: ");
	fun_console_write_line(prompt);
	fun_console_write_line("");

	t0 = (double)fun_timing_now_ns() / 1e9;

	int generated = 0;
	char response[2048];
	size_t resp_len = 0;
	float *logits_buf =
		(float *)fun_memory_allocate(201088 * sizeof(float)).value;
	while (generated < MAX_TOKENS) {
		model_forward(&model, tokens, n_tokens, logits_buf);

		int next_id = 0;
		float max_l = logits_buf[0];
		for (int i = 1; i < 201088; i++) {
			if (logits_buf[i] > max_l) {
				max_l = logits_buf[i];
				next_id = i;
			}
		}

		if (next_id == tok.eos_id)
			break;

		tokens[n_tokens++] = next_id;
		generated++;

		char token_str[32];
		tokenizer_decode(&tok, next_id, token_str, 32);
		fun_console_write(token_str);
		if (resp_len + 32 < sizeof(response)) {
			fun_string_copy(token_str, response + resp_len,
					sizeof(response) - resp_len);
			resp_len += (size_t)fun_string_length(token_str);
		}
	}
	fun_memory_free((Memory *)&logits_buf);

	t1 = (double)fun_timing_now_ns() / 1e9;
	double eval_s = t1 - t0;

	fun_console_write_line("");
	fun_console_write_line("");
	fun_console_write_line("  Final result:");
	fun_console_write(response);
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

	if (do_trace) {
		fun_console_write_line("");
		fun_console_write_line("  === Trace Report ===");
		fun_trace_report(_trace_output);
	}

	model_free(&model);
	fun_gguf_close(gguf);
	fun_memory_free((Memory *)&num_buf);
	fun_trace_destroy();
	fun_memory_free(&trace_mem);

	return 0;
}
