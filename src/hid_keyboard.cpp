/**
 * @file hid_keyboard.cpp
 * @brief Implémentation des fonctionnalités HID pour les claviers.
 * @part of Apple-ADB-Ressurector
 * Inspiré et basé sur le travail initial de Szymon Łopaciuk
 * https://github.com/szymonlopaciuk/stm32-adb2usb
 *
 *
 * @date 2025
 * @author Clément SAILLANT
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#include "hid_keyboard.h"
#ifdef ARDUINO_ARCH_STM32
#include "usbd_hid_composite_if.h"
#endif
#ifdef ARDUINO_ARCH_ESP32
#include <BLEHIDDevice.h> // Ajout de l'inclusion manquante pour BLECharacteristic
extern bool isBleConnected; // Déclaration externe pour isBleConnected
extern BLECharacteristic* input_keyboard; // Déclaration externe pour input_keyboard
#endif
#include <Arduino.h>

/**
 * @brief Initialise le clavier HID.
 */
void hid_keyboard_init() {
#ifdef ARDUINO_ARCH_STM32
  Serial.println("Initialisation du clavier HID USB...");
  HID_Composite_Init(HID_KEYBOARD);
  Serial.println("Clavier HID USB initialisé.");
#endif

#ifdef ARDUINO_ARCH_ESP32
  Serial.println("Initialisation du clavier HID Bluetooth...");
  // L'initialisation Bluetooth HID est déjà gérée dans `setupBluetoothHID` dans
  // main.cpp
  Serial.println("Clavier HID Bluetooth initialisé.");
#endif
}

/**
 * @brief Ferme le clavier HID.
 */
void hid_keyboard_close() {
#ifdef ARDUINO_ARCH_STM32
  Serial.println("Fermeture du clavier HID USB...");
  HID_Composite_DeInit(HID_KEYBOARD);
  Serial.println("Clavier HID USB fermé.");
#endif
#ifdef ARDUINO_ARCH_ESP32
  Serial.println("Fermeture du clavier HID Bluetooth...");
  // Aucune action spécifique nécessaire pour ESP32, car Bluetooth HID est géré
  // globalement.
  Serial.println("Clavier HID Bluetooth fermé.");
#endif
}

/**
 * @brief Envoie un rapport HID pour le clavier.
 *
 * @param report Pointeur vers le rapport HID à envoyer.
 */
void hid_keyboard_send_report(hid_key_report *report) {
  uint8_t buf[8] = {report->modifiers, 0,
                    report->keys[0],   report->keys[1],
                    report->keys[2],   report->keys[3],
                    report->keys[4],   report->keys[5]};

  Serial.print("Envoi du rapport HID clavier - Modificateurs: ");
  Serial.print(report->modifiers, HEX);
  Serial.print(", Touches: ");
  for (int i = 0; i < KEY_REPORT_KEYS_COUNT; i++) {
    Serial.print(report->keys[i], HEX);
    if (i < KEY_REPORT_KEYS_COUNT - 1)
      Serial.print(", ");
  }
  Serial.println();

#ifdef ARDUINO_ARCH_STM32
  HID_Composite_keyboard_sendReport(buf, 8);
#endif

#ifdef ARDUINO_ARCH_ESP32
  if (isBleConnected) {
    input_keyboard->setValue(buf, sizeof(buf));
    input_keyboard->notify();
  }
#endif
}

/**
 * @brief Met à jour les touches du rapport HID à partir d'un registre ADB.
 *
 * @param report Pointeur vers le rapport HID.
 * @param key_press Données du registre ADB.
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_set_keys_from_adb_register(
    hid_key_report *report, adb_data<adb_kb_keypress> key_press) {
  Serial.print("Mise à jour des touches HID depuis le registre ADB - Raw: ");
  Serial.println(key_press.raw, HEX);

  if (key_press.raw == ADBKey::KeyCode::POWER_DOWN)
    return hid_keyboard_update_key_in_report(report, ADB_KEY_POWER, false);
  else if (key_press.raw == ADBKey::KeyCode::POWER_UP)
    return hid_keyboard_update_key_in_report(report, ADB_KEY_POWER, true);

  bool report_changed = false;

  uint8_t key0 = key_press.data.key0;
  if (ADBKeymap::isModifier(key0))
    report_changed = hid_keyboard_update_modifier_in_report(
        report, key0, key_press.data.released0);
  else
    report_changed = hid_keyboard_update_key_in_report(
        report, ADBKeymap::toHID(key0), key_press.data.released0);

  uint8_t key1 = key_press.data.key1;
  if (ADBKeymap::isModifier(key1))
    report_changed = hid_keyboard_update_modifier_in_report(
                         report, key1, key_press.data.released1) ||
                     report_changed;
  else
    report_changed =
        hid_keyboard_update_key_in_report(report, ADBKeymap::toHID(key1),
                                          key_press.data.released1) ||
        report_changed;

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
bool hid_keyboard_update_key_in_report(hid_key_report *report,
                                       uint8_t hid_keycode, bool released) {
  Serial.print("Mise à jour d'une touche HID - Code: ");
  Serial.print(hid_keycode, HEX);
  Serial.print(", Relâché: ");
  Serial.println(released);

  if (hid_keycode == ADB_KEY_NONE)
    return false;

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
bool hid_keyboard_add_key_to_report(hid_key_report *report,
                                    uint8_t hid_keycode) {
  Serial.print("Ajout d'une touche HID - Code: ");
  Serial.println(hid_keycode, HEX);

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
bool hid_keyboard_remove_key_from_report(hid_key_report *report,
                                         uint8_t hid_keycode) {
  Serial.print("Suppression d'une touche HID - Code: ");
  Serial.println(hid_keycode, HEX);

  bool report_changed = false;
  for (uint8_t i = 0; i < KEY_REPORT_KEYS_COUNT; i++) {
    if (report->keys[i] == hid_keycode) {
      report->keys[i] = 0;
      report_changed = true;
    }
  }
  return report_changed;
}

/**
 * @brief Met à jour les modificateurs dans le rapport HID.
 *
 * @param report Pointeur vers le rapport HID.
 * @param modifier Code du modificateur (ex. Shift, Ctrl).
 * @param pressed Indique si le modificateur est pressé (true) ou relâché
 * (false).
 * @return true si le rapport a été modifié, false sinon.
 */
bool hid_keyboard_update_modifier_in_report(hid_key_report *report,
                                            uint8_t adb_keycode,
                                            bool released) {
  Serial.print("Mise à jour d'un modificateur HID - ADB Keycode: ");
  Serial.print(adb_keycode, HEX);
  Serial.print(", Relâché: ");
  Serial.println(released);

  auto update_modifier = [released, report](uint8_t mask) {
    // Vérifie si le modificateur est déjà dans l'état souhaité
    if (released == !(report->modifiers & mask))
      return false;

    // Met à jour le modificateur
    if (!released)
      report->modifiers |= mask; // Active le modificateur
    else
      report->modifiers &= ~mask; // Désactive le modificateur

    return true;
  };

  // Correspondance des codes ADB avec les modificateurs HID
  if (adb_keycode == ADBKey::KeyCode::LEFT_SHIFT)
    return update_modifier(KEY_MOD_LSHIFT);
  if (adb_keycode == ADBKey::KeyCode::RIGHT_SHIFT)
    return update_modifier(KEY_MOD_RSHIFT);
  if (adb_keycode == ADBKey::KeyCode::LEFT_CONTROL)
    return update_modifier(KEY_MOD_LCTRL);
  if (adb_keycode == ADBKey::KeyCode::RIGHT_CONTROL)
    return update_modifier(KEY_MOD_RCTRL);
  if (adb_keycode == ADBKey::KeyCode::LEFT_OPTION)
    return update_modifier(KEY_MOD_LALT);
  if (adb_keycode == ADBKey::KeyCode::RIGHT_OPTION)
    return update_modifier(KEY_MOD_RALT);
  if (adb_keycode == ADBKey::KeyCode::LEFT_COMMAND)
    return update_modifier(KEY_MOD_LMETA);
  if (adb_keycode == ADBKey::KeyCode::RIGHT_COMMAND)
    return update_modifier(KEY_MOD_RMETA);

  Serial.println("Modificateur inconnu.");
  return false; // Aucun changement
}