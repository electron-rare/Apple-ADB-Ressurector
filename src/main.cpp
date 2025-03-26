/**
 * @file main.cpp
 * @brief Main file for the Apple ADB Ressurector project.

 * Inspiré et basé sur le travail initial de Szymon Łopaciuk https://github.com/szymonlopaciuk/stm32-adb2usb
 * @credits Szymon Łopaciuk
 * @author Clément SAILLANT
 * @date 2025
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#ifndef UNIT_TEST

#include <ADB.h>
#include "hid_keyboard.h"
#include "hid_mouse.h"

#define POLL_DELAY 5

/**
 * @struct DeviceState
 * @brief Structure pour regrouper les états des périphériques.
 */
struct DeviceState {
    bool apple_extended_detected = false; /**< Détection du clavier Apple étendu. */
    bool keyboard_present = false;       /**< Présence d'un clavier. */
    bool mouse_present = false;          /**< Présence d'une souris. */
    bool led_caps = false;               /**< État de la LED Caps Lock. */
    bool led_num = true;                 /**< État de la LED Num Lock (actif par défaut). */
};

// Instances globales
ADB adb(PB4); /**< Instance du bus ADB. */
ADBDevices adbDevices(adb); /**< Gestionnaire des périphériques ADB. */
DeviceState deviceState; /**< État des périphériques. */

/**
 * @brief Initialise un périphérique ADB.
 * 
 * @param addr Adresse du périphérique.
 * @param handler_id Identifiant du gestionnaire de périphérique.
 * @return true si l'initialisation a réussi, false sinon.
 */
bool initializeDevice(uint8_t addr, uint8_t handler_id) {
    bool error = false;
    adb_data<adb_register3> reg3 = {0}, mask = {0};
    reg3.data.device_handler_id = handler_id;
    mask.data.device_handler_id = 0xFF;
    return adbDevices.deviceUpdateRegister3(addr, reg3, mask.raw, &error) && !error;
}

/**
 * @brief Fonction d'initialisation du programme.
 */
void setup() {
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, LOW);

    Serial.begin(115200);

    adb.init(PB4, true);

    delay(1000);

    deviceState.keyboard_present = initializeDevice(ADBKey::Address::KEYBOARD, 0x03);
    deviceState.mouse_present = initializeDevice(ADBKey::Address::MOUSE, 0x02);

    digitalWrite(PC13, HIGH);

    adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
}

/**
 * @brief Gère les événements du clavier.
 */
void handleKeyboard() {
    static hid_key_report key_report = {0};
    bool error = false;

    auto key_press = adbDevices.keyboardReadKeyPress(&error);
    if (error) return;

    bool report_changed = hid_keyboard_set_keys_from_adb_register(&key_report, key_press);

    if ((key_press.data.key0 == ADBKey::KeyCode::CAPS_LOCK && !key_press.data.released0) ||
        (key_press.data.key1 == ADBKey::KeyCode::CAPS_LOCK && !key_press.data.released1)) {
        deviceState.led_caps = !deviceState.led_caps;
        adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
        report_changed = true;
    }

    if ((key_press.data.key0 == ADBKey::KeyCode::NUM_LOCK && !key_press.data.released0) ||
        (key_press.data.key1 == ADBKey::KeyCode::NUM_LOCK && !key_press.data.released1)) {
        deviceState.led_num = !deviceState.led_num;
        adbDevices.keyboardWriteLEDs(false, deviceState.led_caps, deviceState.led_num);
        report_changed = true;
    }

    if (report_changed) {
        hid_keyboard_send_report(&key_report);
    }
}

/**
 * @brief Gère les événements de la souris.
 */
void handleMouse() {
    bool error = false;

    auto mouse_data = adbDevices.mouseReadData(&error);
    if (error || mouse_data.raw == 0) return;

    int8_t mouse_x = adbMouseConvertAxis(mouse_data.data.x_offset);
    int8_t mouse_y = adbMouseConvertAxis(mouse_data.data.y_offset);

    hid_mouse_send_report(mouse_data.data.button ? 0 : 1, mouse_x, mouse_y);
}

/**
 * @brief Boucle principale du programme.
 */
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