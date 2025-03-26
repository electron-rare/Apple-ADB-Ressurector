/**
 * @file hid_mouse.h
 * @brief Gestion des fonctionnalités HID pour les souris.
 * @part of Apple-ADB-Ressurector
 * Inspiré et basé sur le travail initial de Szymon Łopaciuk https://github.com/szymonlopaciuk/stm32-adb2usb
 * 
 * 
 * @date 2025
 * @author Clément SAILLANT
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#ifndef HID_MOUSE_h
#define HID_MOUSE_h

#include <cstdint>

/**
 * @brief Initialise la souris HID.
 */
void hid_mouse_init();

/**
 * @brief Ferme la souris HID.
 */
void hid_mouse_close();

/**
 * @brief Envoie un rapport HID pour la souris.
 * 
 * @param button État du bouton de la souris (appuyé ou relâché).
 * @param offset_x Déplacement horizontal de la souris.
 * @param offset_y Déplacement vertical de la souris.
 */
void hid_mouse_send_report(bool button, int8_t offset_x, int8_t offset_y);

#endif