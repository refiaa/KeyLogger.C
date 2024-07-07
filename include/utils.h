#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

char* safe_strdup(const char* str);
void* safe_malloc(size_t size);
void safe_free(void* ptr);

#endif // UTILS_H