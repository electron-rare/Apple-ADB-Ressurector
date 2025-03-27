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

#ifdef ARDUINO_ARCH_STM32
#include "usbd_hid_composite_if.h"
#endif

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
#include <BLEHIDDevice.h> // Ajout de l'inclusion manquante pour BLECharacteristic
#include "ADBKeymap.h" // Assurez-vous que la conversion ADB vers HID est incluse
extern BLECharacteristic* input_mouse; // Déclaration externe pour input_mouse
extern bool isBleConnected; // Déclaration externe pour isBleConnected
#endif

/**
 * @brief Initialise la souris HID.
 */
void hid_mouse_init() {
#ifdef ARDUINO_ARCH_STM32
    Serial.println("Initialisation de la souris HID USB...");
    HID_Composite_Init(HID_MOUSE);
    Serial.println("Souris HID USB initialisée.");
#endif

#ifdef ARDUINO_ARCH_ESP32
    Serial.println("Initialisation de la souris HID Bluetooth...");
    // L'initialisation Bluetooth HID est déjà gérée dans `setupBluetoothHID` dans main.cpp
    Serial.println("Souris HID Bluetooth initialisée.");
#endif
}

/**
 * @brief Ferme la souris HID.
 */
void hid_mouse_close() {
#ifdef ARDUINO_ARCH_STM32
    Serial.println("Fermeture de la souris HID USB...");
    HID_Composite_DeInit(HID_MOUSE);
    Serial.println("Souris HID USB fermée.");
#endif

#ifdef ARDUINO_ARCH_ESP32
    Serial.println("Fermeture de la souris HID Bluetooth...");
    // Aucune action spécifique nécessaire pour ESP32, car Bluetooth HID est géré globalement.
    Serial.println("Souris HID Bluetooth fermée.");
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
    m[0] = button; // Bouton de la souris (0 = relâché, 1 = appuyé)
    m[1] = offset_x; // Déplacement horizontal
    m[2] = offset_y; // Déplacement vertical
    m[3] = 0; // Réservé

    Serial.print("Envoi du rapport HID souris - Bouton: ");
    Serial.print(button);
    Serial.print(", X: ");
    Serial.print(offset_x);
    Serial.print(", Y: ");
    Serial.println(offset_y);

#ifdef ARDUINO_ARCH_STM32
    HID_Composite_mouse_sendReport(m, 4);
#endif

#ifdef ARDUINO_ARCH_ESP32
    if (isBleConnected) {
        // Traduction des données ADB en HID Bluetooth
        uint8_t bt_report[4] = {m[0], m[1], m[2], m[3]};

        // Envoi du rapport via Bluetooth
        input_mouse->setValue(bt_report, sizeof(bt_report));
        input_mouse->notify();

        // Libération des boutons entre deux actions pour éviter les répétitions
        delay(5);
        bt_report[0] = 0; // Aucun bouton appuyé
        input_mouse->setValue(bt_report, sizeof(bt_report));
        input_mouse->notify();
    }
#endif
}