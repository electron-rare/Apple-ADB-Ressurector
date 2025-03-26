/**
 * @file hid_mouse.cpp
 * @brief Implémentation des fonctionnalités HID pour les souris.
 * @part of Apple-ADB-Ressurector
 * Inspiré et basé sur le travail initial de Szymon Łopaciuk https://github.com/szymonlopaciuk/stm32-adb2usb
 * 
 * 
 * @date 2025
 * @author Clément SAILLANT
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#include "hid_mouse.h"
#include "usbd_hid_composite_if.h"
#include <Arduino.h>

/**
 * @brief Initialise la souris HID.
 */
void hid_mouse_init() {
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_Init(HID_MOUSE);
#endif
}

/**
 * @brief Ferme la souris HID.
 */
void hid_mouse_close() {
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_DeInit(HID_MOUSE);
#endif
}

/**
 * @brief Envoie un rapport HID pour la souris.
 * 
 * @param button État du bouton de la souris (appuyé ou relâché).
 * @param offset_x Déplacement horizontal de la souris.
 * @param offset_y Déplacement vertical de la souris.
 */
void hid_mouse_send_report(bool button, int8_t offset_x, int8_t offset_y) {
    uint8_t m[4];
    m[0] = button;
    m[1] = offset_x;
    m[2] = offset_y;
    m[3] = 0;

#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_mouse_sendReport(m, 4);
#endif
}