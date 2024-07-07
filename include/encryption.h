#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdint.h>
#include <stddef.h>

#define CHACHA20_KEY_SIZE 32
#define CHACHA20_NONCE_SIZE 12

typedef struct {
    uint32_t state[16];
} ChaCha20_ctx;

void chacha20_init(ChaCha20_ctx *ctx, const uint8_t *key, const uint8_t *nonce);
void chacha20_encrypt(ChaCha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length);
void chacha20_decrypt(ChaCha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length);

#endif // ENCRYPTION_H