#ifndef GPT_DEMO_TOKENIZER_H
#define GPT_DEMO_TOKENIZER_H

#include "fundamental/gguf/gguf.h"

typedef struct {
	int token_id;
	int children[256];
} TrieNode;

typedef struct {
	int n_tokens;
	char **tokens;
	int *token_lens;
	TrieNode *trie;
	int trie_count;
	int bos_id;
	int eos_id;
} Tokenizer;

void tokenizer_load(Tokenizer *t, GGufFile *gguf);
int tokenizer_encode(Tokenizer *t, const char *text, int *out_tokens,
					 int max_out);
void tokenizer_decode(Tokenizer *t, int token_id, char *out, int out_size);

#endif
