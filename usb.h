#ifndef USB_H
#define USB_H

#include "boards.h"

#include "app_usbd_hid_mouse.h"

extern app_usbd_hid_mouse_t *usb_mouse_handler;
void usb_init();
void usb_in_loop();

#endif