#include "fundamental/async/async.h"
#include "fundamental/console/console.h"
#include "fundamental/file/file.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/stream/stream.h"
#include "fundamental/string/string.h"
#include "fundamental/timing/timing.h"
#include "fundamental/trace/trace.h"
#include "model.h"
#include "tokenizer.h"

#define MAX_TOKENS 128
#define MAX_SEQ 1024
#define MAX_PROMPT_BYTES 16384

/* Read a prompt text file into out (NUL-terminated), trimming surrounding
 * whitespace. Returns 0 on success, -1 if the file can't be opened or is empty.
 * Uses the stream API so the file size need not be known up front. */
static int read_prompt_file(const char *path, char *out, size_t out_size)
{
	const uint64_t CHUNK_SIZE = 4096;
	MemoryResult buf_res = fun_memory_allocate(CHUNK_SIZE);
	if (fun_error_is_error(buf_res.error))
		return -1;
	Memory buffer = buf_res.value;

	AsyncResult open_res = fun_stream_create_file_read(path, buffer,
													   CHUNK_SIZE,
													   FILE_MODE_AUTO);
	fun_async_await(&open_res, -1);
	if (fun_error_is_error(open_res.error)) {
		fun_memory_free(&buffer);
		return -1;
	}
	FileStream *stream = (FileStream *)open_res.state;

	size_t out_len = 0;
	while (!fun_stream_is_end_of_stream(stream)) {
		uint64_t bytes_read = 0;
		AsyncResult read_res = fun_stream_read(stream, &bytes_read);
		fun_async_await(&read_res, -1);
		if (fun_error_is_error(read_res.error))
			break;
		if (bytes_read > 0) {
			if (out_len + bytes_read >= out_size)
				bytes_read = out_size - out_len - 1;
			fun_memory_copy(buffer, out + out_len, bytes_read);
			out_len += (size_t)bytes_read;
		}
	}

	fun_stream_destroy(stream);
	fun_memory_free(&buffer);

	out[out_len] = '\0';
	fun_string_trim_in_place(out);
	return out_len > 0 ? 0 : -1;
}

/* gpt-demo: CPU inference of the GPT-OSS-20B MoE model (MXFP4 weights).
 *
 * Pipeline:
 *   1. parse prompt + --threads N
 *   2. open the GGUF, load the tokenizer and model weights, build compute graph
 *   3. tokenize the prompt, then autoregressively generate:
 *        forward pass -> argmax(logits) -> append token -> repeat until EOS
 *   4. print the detokenized response and timing stats.
 *
 * Vocab size is hard-coded to 201088 (see tokenizer.c VOCAB_SIZE). */
int main(int argc, char **argv)
{
	if (argc < 2) {
		fun_console_write_line(
			"Usage: demo.exe \"your prompt\" [--threads N] [--prompt-file FILE]");
		fun_console_write_line(
			"       demo.exe --prompt-file prompts/prompt.txt [--threads N]");
		return 1;
	}

	fun_math_init();

	/* Parse the prompt (positional) plus the --threads / --prompt-file flags. */
	const char *prompt = NULL;
	const char *prompt_file = NULL;
	int n_threads = 4;
	for (int i = 1; i < argc; i++) {
		if (fun_string_compare(argv[i], "--threads") == 0 && i + 1 < argc) {
			n_threads = 0;
			const char *ns = argv[++i];
			while (*ns >= '0' && *ns <= '9')
				n_threads = n_threads * 10 + (*ns++ - '0');
		} else if (fun_string_compare(argv[i], "--prompt-file") == 0 &&
				   i + 1 < argc) {
			prompt_file = argv[++i];
		} else if (!prompt) {
			prompt = argv[i];
		}
	}

	char prompt_buf[MAX_PROMPT_BYTES];
	if (prompt_file) {
		if (read_prompt_file(prompt_file, prompt_buf,
							 sizeof(prompt_buf)) != 0) {
			fun_console_error_line("Cannot read prompt file");
			return 1;
		}
		prompt = prompt_buf;
	}

	if (!prompt) {
		fun_console_error_line("No prompt supplied");
		return 1;
	}

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

	Model model;
	model_load(&model, gguf, "../../models/openai_gpt-oss-20b-MXFP4.gguf",
			   n_threads);

	fun_console_write_line("Model loaded successfully.");

	char output[512];

	double t1 = (double)fun_timing_now_ns() / 1e9;
	StringTemplateParam loaded_params[] = {
    	{ "count", { .doubleValue = t1 - t0 } }
	};
	fun_string_template("Loaded in %{count} seconds", loaded_params, 1, output, 512);
	fun_console_write_line(output);
	
	int tokens[MAX_SEQ];
	int n_tokens = tokenizer_encode(&tok, prompt, tokens, MAX_SEQ);
	if (n_tokens == 0) {
		fun_console_error_line("  Empty prompt after tokenization");
		return 1;
	}

	StringTemplateParam token_params[] = {
    	{ "tokens", { .intValue = n_tokens } }
	};
	fun_string_template("Prompt tokens: #{tokens}", token_params, 1, output, 512);
	fun_console_write_line(output);
	fun_console_write("  Prompt: ");
	fun_console_write_line(prompt);

	t0 = (double)fun_timing_now_ns() / 1e9;

	int generated = 0;
	char response[16384] = { 0 };
	size_t resp_len = 0;
	/* logits_buf: one float per vocab entry (201088), reused every step. */
	float *logits_buf =
		(float *)fun_memory_allocate(201088 * sizeof(float)).value;
	while (generated < MAX_TOKENS) {
		/* Forward the whole token sequence; only the newest position is new
		 * (earlier ones are served from the KV cache), and logits_buf gets
		 * the final position's output distribution over the vocabulary. */
		model_forward(&model, tokens, n_tokens, logits_buf);

		/* Greedy decode: pick the argmax logit as the next token. */
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

		/* Stream the token's text and accumulate it into the response buffer. */
		char token_str[32];
		tokenizer_decode(&tok, next_id, token_str, 32);
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

	StringTemplateParam final_params[] = {
		{ "seconds", { .doubleValue = eval_s } },
		{ "generated", { .intValue = generated } },
		{ "toks_per_sec", { .doubleValue = eval_s > 0.0 ? (double)generated / eval_s : 0.0 } }
	};
	fun_string_template("  Time (seconds): %{seconds}, Tokens: #{generated}, tok/s: %{toks_per_sec}", final_params, 3, output, 512);
	fun_console_write_line(output);

	model_free(&model);
	fun_gguf_close(gguf);

	return 0;
}
