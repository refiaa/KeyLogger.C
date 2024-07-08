#ifndef KEYLOGGER_SYSTEM_H
#define KEYLOGGER_SYSTEM_H

#include <stdbool.h>

typedef struct KeyloggerSystem KeyloggerSystem;

KeyloggerSystem* keylogger_system_create(void);
void keylogger_system_destroy(KeyloggerSystem* system);

bool keylogger_system_initialize(KeyloggerSystem* system);
bool keylogger_system_run(KeyloggerSystem* system);
void keylogger_system_shutdown(KeyloggerSystem* system);

#endif // KEYLOGGER_SYSTEM_H