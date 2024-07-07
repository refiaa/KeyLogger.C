#include "../include/file_hiding.h"
#include <windows.h>
#include <stdio.h>

int hide_file_attribute(const char* filepath) {
    DWORD attributes = GetFileAttributesA(filepath);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return -1;
    
    if (!SetFileAttributesA(filepath, attributes | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
        return -1;
    
    return 0;
}

int unhide_file_attribute(const char* filepath) {
    DWORD attributes = GetFileAttributesA(filepath);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return -1;
    
    if (!SetFileAttributesA(filepath, attributes & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
        return -1;
    
    return 0;
}

int hide_file_ads(const char* filepath, const char* content) {
    char ads_path[MAX_PATH];
    snprintf(ads_path, sizeof(ads_path), "%s:hidden", filepath);
    
    HANDLE hFile = CreateFileA(ads_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    
    DWORD bytesWritten;
    BOOL result = WriteFile(hFile, content, strlen(content), &bytesWritten, NULL);
    CloseHandle(hFile);
    
    return result ? 0 : -1;
}

int read_hidden_file_ads(const char* filepath, char** content) {
    char ads_path[MAX_PATH];
    snprintf(ads_path, sizeof(ads_path), "%s:hidden", filepath);
    
    HANDLE hFile = CreateFileA(ads_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return -1;
    }
    
    *content = (char*)malloc(fileSize + 1);
    if (!*content) {
        CloseHandle(hFile);
        return -1;
    }
    
    DWORD bytesRead;
    BOOL result = ReadFile(hFile, *content, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    
    if (!result) {
        free(*content);
        return -1;
    }
    
    (*content)[fileSize] = '\0';
    return 0;
}