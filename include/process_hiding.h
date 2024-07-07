#ifndef PROCESS_HIDING_H
#define PROCESS_HIDING_H

#include <windows.h>

typedef struct ProcessHider ProcessHider;

struct ProcessHider {
    BOOL (*hide_process)(ProcessHider*, DWORD);
    BOOL (*unhide_process)(ProcessHider*, DWORD);
};

ProcessHider* process_hider_create(void);
void process_hider_destroy(ProcessHider*);

#endif // PROCESS_HIDING_H