/**
 * @file hid_keyboard.h
 * @brief Gestion des fonctionnalités HID pour les claviers.
 * @part of Apple-ADB-Ressurector
 * Inspiré et basé sur le travail initial de Szymon Łopaciuk https://github.com/szymonlopaciuk/stm32-adb2usb
 * 
 * @date 2025
 * @license GNU GPL v3
 */

#ifndef HID_KEYBOARD_H
#define HID_KEYBOARD_H

#include <cstdint>
#include <stdbool.h>
#include "adb.h"

#define KEY_REPORT_KEYS_COUNT 6 /**< Nombre maximum de touches dans un rapport HID. */

// Masques pour les modificateurs HID
#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_LALT   0x04
#define KEY_MOD_LMETA  0x08
#define KEY_MOD_RCTRL  0x10
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_RALT   0x40
#define KEY_MOD_RMETA  0x80

/**
 * @struct hid_key_report
 * @brief Structure représentant un rapport HID pour un clavier.
 */
struct hid_key_report {
    uint8_t modifiers; /**< Modificateurs actifs (Ctrl, Alt, etc.). */
    uint8_t keys[KEY_REPORT_KEYS_COUNT]; /**< Tableau des touches actives. */
};

/**
 * @brief Initialise le clavier HID.
 */
void hid_keyboard_init();

/**
 * @brief Ferme le clavier HID.
 */
void hid_keyboard_close();

/**
 * @brief Envoie un rapport HID pour le clavier.
 * 
 * @param report Pointeur vers le rapport HID à envoyer.
 */
void hid_keyboard_send_report(hid_key_report* report);

/**
 * @brief Met à jour les touches du rapport HID à partir d'un registre ADB.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param reg Données du registre ADB.
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_set_keys_from_adb_register(hid_key_report* report, adb_data<adb_kb_keypress> reg);

/**
 * @brief Met à jour une touche spécifique dans le rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @param released Indique si la touche est relâchée.
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_update_key_in_report(hid_key_report* report, uint8_t hid_keycode, bool released);

/**
 * @brief Ajoute une touche au rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @return true si la touche a été ajoutée, false sinon.
 */
bool hid_keyboard_add_key_to_report(hid_key_report* report, uint8_t hid_keycode);

/**
 * @brief Supprime une touche du rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @return true si la touche a été supprimée, false sinon.
 */
bool hid_keyboard_remove_key_from_report(hid_key_report* report, uint8_t hid_keycode);

/**
 * @brief Met à jour les modificateurs dans le rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param modifier Code du modificateur (ex. Shift, Ctrl).
 * @param pressed Indique si le modificateur est pressé (true) ou relâché (false).
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_update_modifier_in_report(hid_key_report* report, uint8_t modifier, bool pressed);

#endif // HID_KEYBOARD_H