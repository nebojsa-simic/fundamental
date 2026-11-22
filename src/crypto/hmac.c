/* HMAC-SHA256 per RFC 2104.  Handles messages up to HMAC_CHUNK bytes.
   For larger messages the caller should chunk externally or use a
   streaming SHA-256 (not yet available).  Returns 0 on success, -1 if
   the message exceeds the chunk limit. */
#include <stdint.h>
#include "fundamental/crypto/crypto.h"

#define HMAC_CHUNK 4096

/* Volatile wipe that the optimizer cannot elide. */
static void secure_zero(volatile uint8_t *p, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
		p[i] = 0;
}

int fun_crypto_hmac_sha256(const uint8_t *key, size_t key_len,
						   const uint8_t *msg, size_t msg_len,
						   uint8_t output[32])
{
	uint8_t block_key[64];
	uint8_t hashed_key[32];
	uint8_t inner[64 + HMAC_CHUNK];
	uint8_t outer[32 + 64];
	uint32_t i;

	if (msg_len > HMAC_CHUNK)
		return -1;

	for (i = 0; i < 64; i++)
		block_key[i] = 0;

	if (key_len > 64) {
		fun_crypto_sha256(key, key_len, hashed_key);
		for (i = 0; i < 32; i++)
			block_key[i] = hashed_key[i];
	} else {
		for (i = 0; i < key_len; i++)
			block_key[i] = key[i];
	}

	for (i = 0; i < 64; i++)
		inner[i] = (uint8_t)(block_key[i] ^ 0x36);

	for (i = 0; i < msg_len; i++)
		inner[64 + i] = msg[i];
	fun_crypto_sha256(inner, 64 + msg_len, hashed_key);

	for (i = 0; i < 64; i++)
		outer[i] = (uint8_t)(block_key[i] ^ 0x5C);
	for (i = 0; i < 32; i++)
		outer[64 + i] = hashed_key[i];
	fun_crypto_sha256(outer, 64 + 32, output);

	/* Scrub all key-derived material from the stack (volatile). */
	secure_zero(block_key, sizeof(block_key));
	secure_zero(hashed_key, sizeof(hashed_key));
	secure_zero(inner, sizeof(inner));
	secure_zero(outer, sizeof(outer));
	return 0;
}