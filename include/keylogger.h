#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <windows.h>
#include <stdbool.h>

typedef struct Keylogger Keylogger;

Keylogger* keylogger_create(void);
void keylogger_destroy(Keylogger* logger);
void keylogger_start(Keylogger* logger);
void keylogger_stop(Keylogger* logger);
char* keylogger_get_logged_keys(Keylogger* logger);

#endif // KEYLOGGER_H