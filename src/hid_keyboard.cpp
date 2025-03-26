/**
 * @file hid_keyboard.cpp
 * @brief Implémentation des fonctionnalités HID pour les claviers.
 * @part of Apple-ADB-Ressurector
 * Inspiré et basé sur le travail initial de Szymon Łopaciuk https://github.com/szymonlopaciuk/stm32-adb2usb
 * 
 * 
 * @date 2025
 * @author Clément SAILLANT
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#include "hid_keyboard.h"
#include "usbd_hid_composite_if.h"
#include <Arduino.h>

/**
 * @brief Initialise le clavier HID.
 */
void hid_keyboard_init() {
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_Init(HID_KEYBOARD);
#endif
}

/**
 * @brief Ferme le clavier HID.
 */
void hid_keyboard_close() {
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_DeInit(HID_KEYBOARD);
#endif
}

/**
 * @brief Envoie un rapport HID pour le clavier.
 * 
 * @param report Pointeur vers le rapport HID à envoyer.
 */
void hid_keyboard_send_report(hid_key_report* report) {
    uint8_t buf[8] = {report->modifiers, 0, report->keys[0],
                      report->keys[1], report->keys[2], report->keys[3],
                      report->keys[4], report->keys[5]};
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_keyboard_sendReport(buf, 8);
#endif
}

/**
 * @brief Met à jour les touches du rapport HID à partir d'un registre ADB.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param key_press Données du registre ADB.
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_set_keys_from_adb_register(hid_key_report* report, adb_data<adb_kb_keypress> key_press) {
    if (key_press.raw == ADBKey::KeyCode::POWER_DOWN)
        return hid_keyboard_update_key_in_report(report, KEY_POWER, false);
    else if (key_press.raw == ADBKey::KeyCode::POWER_UP)
        return hid_keyboard_update_key_in_report(report, KEY_POWER, true);

    bool report_changed = false;

    uint8_t key0 = key_press.data.key0;
    if (ADBKeymap::isModifier(key0))
        report_changed = hid_keyboard_update_modifier_in_report(report, key0, key_press.data.released0);
    else
        report_changed = hid_keyboard_update_key_in_report(report, ADBKeymap::toHID(key0), key_press.data.released0);

    uint8_t key1 = key_press.data.key1;
    if (ADBKeymap::isModifier(key1))
        report_changed = hid_keyboard_update_modifier_in_report(report, key1, key_press.data.released1) || report_changed;
    else
        report_changed = hid_keyboard_update_key_in_report(report, ADBKeymap::toHID(key1), key_press.data.released1) || report_changed;

    return report_changed;
}

/**
 * @brief Met à jour une touche spécifique dans le rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @param released Indique si la touche est relâchée.
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_update_key_in_report(hid_key_report* report, uint8_t hid_keycode, bool released) {
    if (hid_keycode == KEY_NONE) return false;

    if (released)
        return hid_keyboard_remove_key_from_report(report, hid_keycode);
    else
        return hid_keyboard_add_key_to_report(report, hid_keycode);
}

/**
 * @brief Ajoute une touche au rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @return true si la touche a été ajoutée, false sinon.
 */
bool hid_keyboard_add_key_to_report(hid_key_report* report, uint8_t hid_keycode) {
    int8_t free_slot = -1;

    for (uint8_t i = 0; i < KEY_REPORT_KEYS_COUNT; i++) {
        if (report->keys[i] == hid_keycode)
            return true;

        if (report->keys[i] == 0 && free_slot == -1)
            free_slot = i;
    }

    if (free_slot == -1)
        return false;

    report->keys[free_slot] = hid_keycode;
    return true;
}

/**
 * @brief Supprime une touche du rapport HID.
 * 
 * @param report Pointeur vers le rapport HID.
 * @param hid_keycode Code HID de la touche.
 * @return true si la touche a été supprimée, false sinon.
 */
bool hid_keyboard_remove_key_from_report(hid_key_report* report, uint8_t hid_keycode) {
    bool report_changed = false;
    for (uint8_t i = 0; i < KEY_REPORT_KEYS_COUNT; i++) {
        if (report->keys[i] == hid_keycode) {
            report->keys[i] = 0;
            report_changed = true;
        }
    }
    return report_changed;
}
