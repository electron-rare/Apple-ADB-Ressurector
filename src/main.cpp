/*
 * This file is part of the stm32-adb2usb project.
 * Inspiré et adapté du projet https://github.com/szymonlopaciuk/stm32-adb2usb
 *
 * Copyright (C) 2025 Clément SAILLANT - L'électron rare
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UNIT_TEST

#include <ADB.h>
#include "hid_keyboard.h"
#include "hid_mouse.h"

#define POLL_DELAY 5

// Structure pour regrouper les états des périphériques
struct DeviceState {
    bool apple_extended_detected = false;
    bool keyboard_present = false;
    bool mouse_present = false;
    bool led_caps = false; // État de la LED Caps Lock
    bool led_num = true;   // État de la LED Num Lock (actif par défaut)
};

// Instances globales
ADB adb(PB4); // Remplacez PB4 par la pin appropriée
ADBDevices adbDevices(adb);
DeviceState deviceState;

// Fonction utilitaire pour initialiser un périphérique
bool initializeDevice(uint8_t addr, uint8_t handler_id) {
    bool error = false;
    adb_data<adb_register3> reg3 = {0}, mask = {0};
    reg3.data.device_handler_id = handler_id;
    mask.data.device_handler_id = 0xFF;
    return adbDevices.deviceUpdateRegister3(addr, reg3, mask.raw, &error) && !error;
}

void setup() {
    // Configuration de la LED d'état
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, LOW);

    // Initialisation de la communication série
    Serial.begin(115200);

    // Initialisation du bus ADB
    adb.init(PB4, true); // Active l'utilisation de ADBDevices

    delay(1000); // Attente pour permettre aux périphériques de se réinitialiser

    // Initialisation des périphériques
    deviceState.keyboard_present = initializeDevice(ADBKey::Address::KEYBOARD, 0x03);
    deviceState.mouse_present = initializeDevice(ADBKey::Address::MOUSE, 0x02);

    // Désactivation de la LED d'état après initialisation
    digitalWrite(PC13, HIGH);

    // Activation de Num Lock au démarrage
    adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
}

void handleKeyboard() {
    static hid_key_report key_report = {0};
    bool error = false;

    // Lecture des touches pressées
    auto key_press = adbDevices.keyboardReadKeyPress(&error);
    if (error) return;

    bool report_changed = hid_keyboard_set_keys_from_adb_register(&key_report, key_press);

    // Gestion de Caps Lock - utilisation des constantes du namespace ADBKey
    if ((key_press.data.key0 == ADBKey::KeyCode::CAPS_LOCK && !key_press.data.released0) ||
        (key_press.data.key1 == ADBKey::KeyCode::CAPS_LOCK && !key_press.data.released1)) {
        deviceState.led_caps = !deviceState.led_caps;
        adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
        report_changed = true;
    }

    // Gestion de Num Lock - utilisation des constantes du namespace ADBKey
    if ((key_press.data.key0 == ADBKey::KeyCode::NUM_LOCK && !key_press.data.released0) ||
        (key_press.data.key1 == ADBKey::KeyCode::NUM_LOCK && !key_press.data.released1)) {
        deviceState.led_num = !deviceState.led_num;
        adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
        report_changed = true;
    }

    // Envoi du rapport HID si modifié
    if (report_changed) {
        hid_keyboard_send_report(&key_report);
    }
}

void handleMouse() {
    bool error = false;

    // Lecture des données de la souris
    auto mouse_data = adbDevices.mouseReadData(&error);
    if (error || mouse_data.raw == 0) return;

    // Conversion des axes avec adbMouseConvertAxis
    int8_t mouse_x = adbMouseConvertAxis(mouse_data.data.x_offset);
    int8_t mouse_y = adbMouseConvertAxis(mouse_data.data.y_offset);

    // Envoi du rapport HID pour la souris
    hid_mouse_send_report(mouse_data.data.button ? 0 : 1, mouse_x, mouse_y);
}

void loop() {
    if (deviceState.keyboard_present) {
        handleKeyboard();
        delay(POLL_DELAY);
    }

    if (deviceState.mouse_present) {
        handleMouse();
        delay(POLL_DELAY);
    }
}

#endif