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

/* Allocate one more node from the pool, zero its children, and return its id.
 * Inputs:  nodes[] pool, count in/out pointer (incremented).
 * Outputs: id of the fresh node. */
static int trie_alloc(TrieNode *nodes, int *count)
{
	int id = *count;
	(*count)++;
	nodes[id].token_id = -1;
	for (int i = 0; i < 256; i++)
		nodes[id].children[i] = -1;
	return id;
}

/* Insert token (a byte string of length len) into the trie so that walking the
 * bytes from the root lands on a node carrying token_id. Creates missing
 * interior nodes on demand.
 * Inputs:  nodes[] pool, count in/out, token bytes, token id, byte length. */
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

/* Reverse of the GPT-2 byte-level BPE "bytes_to_unicode" mapping. GGUF stores
 * BPE vocab pieces with non-printable bytes replaced by U+0100..U+0143 and
 * printable latin-1 bytes kept as themselves. Returns the raw byte a codepoint
 * decodes to, or -1 if it is a genuine (non-escaped) character. */
static int byte_from_codepoint(int cp)
{
	if (cp >= 0x21 && cp <= 0x7e) /* ASCII printable */
		return cp;
	if (cp >= 0xa1 && cp <= 0xac) /* latin-1 printable */
		return cp;
	if (cp >= 0xae && cp <= 0xff) /* latin-1 printable */
		return cp;
	if (cp >= 0x100 && cp <= 0x143) { /* escaped non-printable byte */
		int n = cp - 0x100;
		if (n < 33)
			return n; /* 0x00..0x20 */
		if (n == 33)
			return 0x7f;
		if (n <= 66)
			return n + 94; /* 0x80..0xa0 */
		return 0xad;       /* n == 67 */
	}
	return -1;
}

/* Decode one UTF-8 codepoint from *p (which is advanced past it). Returns the
 * codepoint, or -1 if the bytes are not valid UTF-8. */
static int utf8_decode(const char **p, const char *end)
{
	const unsigned char *s = (const unsigned char *)*p;
	if (s >= (const unsigned char *)end)
		return -1;
	unsigned char b0 = *s;
	if (b0 < 0x80) {
		(*p)++;
		return b0;
	}
	int n, cp;
	if ((b0 & 0xe0) == 0xc0) {
		n = 1;
		cp = b0 & 0x1f;
	} else if ((b0 & 0xf0) == 0xe0) {
		n = 2;
		cp = b0 & 0x0f;
	} else if ((b0 & 0xf8) == 0xf0) {
		n = 3;
		cp = b0 & 0x07;
	} else {
		(*p)++;
		return -1;
	}
	for (int i = 0; i < n; i++) {
		s++;
		if (s >= (const unsigned char *)end || (*s & 0xc0) != 0x80)
			return -1;
		cp = (cp << 6) | (*s & 0x3f);
	}
	*p = (const char *)(s + 1);
	return cp;
}

/* Pull every token string out of the GGUF metadata and un-escape its byte-level
 * BPE encoding (space/newline/tab and other control bytes are stored as
 * U+0100..U+0143; e.g. U+00E2/U+0122/U+0136 -> 0xE2/0x80/0x94 = em-dash).
 * Builds the byte trie and stores the decoded strings for later decode lookups.
 * Inputs:  t (zeroed), gguf handle.
 * Outputs: t fully populated (tokens, token_lens, trie). */
void tokenizer_load(Tokenizer *t, GGufFile *gguf)
{
	t->bos_id = BOS_TOKEN;
	t->eos_id = EOS_TOKEN;
	t->n_tokens = VOCAB_SIZE;

	t->tokens = (char **)fun_memory_allocate(VOCAB_SIZE * sizeof(char *)).value;
	t->token_lens = (int *)fun_memory_allocate(VOCAB_SIZE * sizeof(int)).value;

	t->trie =
		(TrieNode *)fun_memory_allocate(VOCAB_SIZE * 16 * sizeof(TrieNode))
			.value;
	t->trie_count = 0;
	trie_alloc(t->trie, &t->trie_count);

	for (int i = 0; i < VOCAB_SIZE; i++) {
		uint64_t tlen = 0;
		StringResult sr = fun_gguf_get_token_string(gguf, (uint32_t)i, &tlen);
		if (fun_error_is_error(sr.error))
			break;

		int len = (int)tlen;
		t->token_lens[i] = len;

		char *copy = (char *)fun_memory_allocate((size_t)(len + 1)).value;
		for (int j = 0; j < len; j++)
			copy[j] = sr.value[j];
		copy[len] = '\0';

		int out_len = 0;
		const char *p = copy;
		const char *end = copy + len;
		while (p < end) {
			const char *start = p;
			int cp = utf8_decode(&p, end);
			int b = (cp >= 0) ? byte_from_codepoint(cp) : -1;
			if (b >= 0) {
				copy[out_len++] = (char)b;
			} else {
				/* genuine (non-escaped) character: keep its UTF-8 bytes */
				for (const char *q = start; q < p; q++)
					copy[out_len++] = *q;
			}
		}
		copy[out_len] = '\0';
		t->token_lens[i] = out_len;
		t->tokens[i] = copy;

		if (out_len > 0)
			trie_insert(t->trie, &t->trie_count, copy, i, out_len);
	}
}

/* Greedy longest-prefix match of text against the trie, one token at a time.
 * At each position, walks bytes forward as far as the trie allows, remembering
 * the last node that held a token id; emits that id and skips its bytes. If no
 * token matches, advances one byte (no id emitted).
 * Inputs:  t, text (UTF-8 bytes), out_tokens buffer, max_out capacity.
 * Outputs: out_tokens[0..n) token ids; returns n. */
int tokenizer_encode(Tokenizer *t, const char *text, int *out_tokens,
					 int max_out)
{
	int n = 0;
	int text_len = (int)fun_string_length(text);
	if (n >= max_out)
		return n;

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

/* Inverse of encode: copy the decoded byte string for token_id into out.
 * Inputs:  t, token_id, out buffer, out_size (capacity incl. NUL).
 * Outputs: out NUL-terminated; out-of-range ids yield empty string. */
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
