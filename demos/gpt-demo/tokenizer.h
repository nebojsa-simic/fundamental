#ifndef GPT_DEMO_TOKENIZER_H
#define GPT_DEMO_TOKENIZER_H

#include "fundamental/gguf/gguf.h"

/* One node in a byte trie keyed by UTF-8 byte value (0-255). token_id is -1
 * for interior nodes and >=0 when the path from the root spells a complete
 * token. children[i] is the child node reached by byte i, or -1 if absent. */
typedef struct {
	int token_id;
	int children[256];
} TrieNode;

/* A byte-trie tokenizer backed by GGUF's token vocabulary. Encoding walks the
 * trie byte-by-byte and takes the longest prefix that ends at a token id (a
 * simple greedy longest-match, not a full BPE merge). */
typedef struct {
	int n_tokens;         /* vocab size */
	char **tokens;        /* [n_tokens]: decoded byte strings, NUL-terminated */
	int *token_lens;      /* [n_tokens]: byte length of each token string */
	TrieNode *trie;       /* trie node pool */
	int trie_count;       /* number of nodes in use */
	int bos_id;
	int eos_id;
} Tokenizer;

void tokenizer_load(Tokenizer *t, GGufFile *gguf);
/* Encode text into up to max_out token ids; returns the number written. */
int tokenizer_encode(Tokenizer *t, const char *text, int *out_tokens,
					 int max_out);
/* Copy the byte string for token_id into out (NUL-terminated, truncated to
 * out_size-1). */
void tokenizer_decode(Tokenizer *t, int token_id, char *out, int out_size);

#endif
