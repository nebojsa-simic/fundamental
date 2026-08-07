#include "tokenizer.h"
#include "fundamental/console/console.h"
#include "fundamental/gguf/gguf.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdint.h>

#define VOCAB_SIZE 201088
#define MAX_SEQ 256
#define EOS_TOKEN 200002
#define BOS_TOKEN 199998

static int trie_alloc(TrieNode *nodes, int *count)
{
	int id = *count;
	(*count)++;
	nodes[id].token_id = -1;
	for (int i = 0; i < 256; i++)
		nodes[id].children[i] = -1;
	return id;
}

static void trie_insert(TrieNode *nodes, int *count, const char *token,
			 int token_id, int len)
{
	int node = 0;
	for (int i = 0; i < len; i++) {
		int c = (unsigned char)token[i];
		if (nodes[node].children[c] == -1)
			nodes[node].children[c] = trie_alloc(nodes, count);
		node = nodes[node].children[c];
	}
	nodes[node].token_id = token_id;
}

void tokenizer_load(Tokenizer *t, GGufFile *gguf)
{
	t->bos_id = BOS_TOKEN;
	t->eos_id = EOS_TOKEN;
	t->n_tokens = VOCAB_SIZE;

	t->tokens =
		(char **)fun_memory_allocate(VOCAB_SIZE * sizeof(char *))
			 .value;
	t->token_lens =
		(int *)fun_memory_allocate(VOCAB_SIZE * sizeof(int))
			 .value;

	t->trie = (TrieNode *)fun_memory_allocate(
				  VOCAB_SIZE * 16 * sizeof(TrieNode))
				  .value;
	t->trie_count = 0;
	trie_alloc(t->trie, &t->trie_count);

	for (int i = 0; i < VOCAB_SIZE; i++) {
		uint64_t tlen = 0;
		StringResult sr = fun_gguf_get_token_string(
			gguf, (uint32_t)i, &tlen);
		if (fun_error_is_error(sr.error))
			break;

		int len = (int)tlen;
		t->token_lens[i] = len;

		char *copy = (char *)fun_memory_allocate(
				     (size_t)(len + 1))
				     .value;
		for (int j = 0; j < len; j++)
			copy[j] = sr.value[j];
		copy[len] = '\0';
		t->tokens[i] = copy;

		if (len > 0)
			trie_insert(t->trie, &t->trie_count, copy, i,
				    len);
	}
}

int tokenizer_encode(Tokenizer *t, const char *text, int *out_tokens,
		     int max_out)
{
	int n = 0;
	int text_len = (int)fun_string_length(text);
	out_tokens[n++] = t->bos_id;

	int i = 0;
	while (i < text_len && n < max_out) {
		int best_id = -1;
		int best_len = 0;
		int node = 0;

		for (int j = i; j < text_len; j++) {
			int c = (unsigned char)text[j];
			if (t->trie[node].children[c] == -1)
				break;
			node = t->trie[node].children[c];
			if (t->trie[node].token_id >= 0) {
				best_id = t->trie[node].token_id;
				best_len = j - i + 1;
			}
		}

		if (best_id >= 0) {
			out_tokens[n++] = best_id;
			i += best_len;
		} else {
			i++;
		}
	}

	return n;
}

void tokenizer_decode(Tokenizer *t, int token_id, char *out, int out_size)
{
	if (token_id < 0 || token_id >= t->n_tokens) {
		out[0] = '\0';
		return;
	}
	int len = t->token_lens[token_id];
	if (len >= out_size)
		len = out_size - 1;
	for (int i = 0; i < len; i++)
		out[i] = t->tokens[token_id][i];
	out[len] = '\0';
}
