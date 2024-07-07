#include "../include/encryption.h"
#include <string.h>

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d, 16), \
    c += d, b ^= c, b = ROTL(b, 12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))

static void chacha20_block(ChaCha20_ctx *ctx, uint32_t *output) {
    uint32_t x[16];
    memcpy(x, ctx->state, sizeof(x));

    for (int i = 0; i < 10; i++) {
        QR(x[0], x[4], x[8], x[12]);
        QR(x[1], x[5], x[9], x[13]);
        QR(x[2], x[6], x[10], x[14]);
        QR(x[3], x[7], x[11], x[15]);
        QR(x[0], x[5], x[10], x[15]);
        QR(x[1], x[6], x[11], x[12]);
        QR(x[2], x[7], x[8], x[13]);
        QR(x[3], x[4], x[9], x[14]);
    }

    for (int i = 0; i < 16; i++)
        output[i] = x[i] + ctx->state[i];

    ctx->state[12]++;
    if (ctx->state[12] == 0)
        ctx->state[13]++;
}

void chacha20_init(ChaCha20_ctx *ctx, const uint8_t *key, const uint8_t *nonce) {
    const uint8_t *sigma = (uint8_t*)"expand 32-byte k";
    ctx->state[0] = *(uint32_t*)(sigma + 0);
    ctx->state[1] = *(uint32_t*)(sigma + 4);
    ctx->state[2] = *(uint32_t*)(sigma + 8);
    ctx->state[3] = *(uint32_t*)(sigma + 12);
    ctx->state[4] = *(uint32_t*)(key + 0);
    ctx->state[5] = *(uint32_t*)(key + 4);
    ctx->state[6] = *(uint32_t*)(key + 8);
    ctx->state[7] = *(uint32_t*)(key + 12);
    ctx->state[8] = *(uint32_t*)(key + 16);
    ctx->state[9] = *(uint32_t*)(key + 20);
    ctx->state[10] = *(uint32_t*)(key + 24);
    ctx->state[11] = *(uint32_t*)(key + 28);
    ctx->state[12] = 0;
    ctx->state[13] = 0;
    ctx->state[14] = *(uint32_t*)(nonce + 0);
    ctx->state[15] = *(uint32_t*)(nonce + 4);
}

void chacha20_encrypt(ChaCha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length) {
    uint32_t block[16];
    uint8_t *block_bytes = (uint8_t*)block;
    size_t i;

    for (i = 0; i < length; i++) {
        if (i % 64 == 0)
            chacha20_block(ctx, block);
        out[i] = in[i] ^ block_bytes[i % 64];
    }
}

void chacha20_decrypt(ChaCha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length) {
    chacha20_encrypt(ctx, in, out, length);
}