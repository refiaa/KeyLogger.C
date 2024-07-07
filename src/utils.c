#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#define MAX_RETRIES 5

static void log_error(const char* message) {
    FILE* log_file = fopen("error.log", "a");
    if (log_file) {
        time_t now = time(NULL);
        char* timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0';
        fprintf(log_file, "[%s] ERROR: %s\n", timestamp, message);
        fclose(log_file);
    }
}

void* safe_malloc(size_t size) {
    void* ptr = NULL;
    int retries = 0;

    while (ptr == NULL && retries < MAX_RETRIES) {
        ptr = malloc(size);
        if (ptr == NULL) {
            retries++;
            log_error("memory alolcation error.");
            Sleep(100);
        }
    }

    if (ptr == NULL) {
        log_error("memory alolcation error; maximum retries");
        exit(1);
    }

    return ptr;
}

char* safe_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char* new_str = safe_malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

void safe_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}
