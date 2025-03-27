/**
 * @file main.cpp
 * @brief Main file for the Apple ADB Ressurector project.

 * Inspiré et basé sur le travail initial de Szymon Łopaciuk
 https://github.com/szymonlopaciuk/stm32-adb2usb
 * @credits Szymon Łopaciuk
 * @author Clément SAILLANT
 * @date 2025
 * Dépôt actuel : https://github.com/electron-rare/Apple-ADB-Ressurector
 * @license GNU GPL v3
 */

#ifndef UNIT_TEST

#include "hid_keyboard.h"
#include "hid_mouse.h"
#include <ADB.h>

#define POLL_DELAY 5
// Définition de la pin ADB selon la plateforme
#ifdef ARDUINO_ARCH_ESP32
#define ADB_PIN 2
#endif
#ifdef ARDUINO_ARCH_STM32
#define ADB_PIN PB4
#endif

// Définition de la pin LED selon la plateforme
#ifdef ARDUINO_ARCH_ESP32
#define LED_PIN 4 // Pin pour ESP32 (Devkit Wemos)
#endif
#ifdef ARDUINO_ARCH_STM32
#define LED_PIN PC13 // Pin pour STM32
#endif

/**
 * @struct DeviceState
 * @brief Structure pour regrouper les états des périphériques.
 */
struct DeviceState {
  bool apple_extended_detected =
      false;                     /**< Détection du clavier Apple étendu. */
  bool keyboard_present = false; /**< Présence d'un clavier. */
  bool mouse_present = false;    /**< Présence d'une souris. */
  bool led_num = true;     /**< État de la LED Num Lock (actif par défaut). */
  bool led_caps = false;   /**< État de la LED Caps Lock. */
  bool led_scroll = false; /**< État de la LED Scroll Lock. */
  bool led_power = false;  /**< État de la LED Power. */
};

// Instances globales
ADB adb(ADB_PIN);               /**< Instance du bus ADB. */
ADBDevices adbDevices(adb);     /**< Gestionnaire des périphériques ADB. */
DeviceState deviceState;        /**< État des périphériques. */
bool caps_lock_pressed = false; /**< État de la touche Caps Lock. */

#ifdef ARDUINO_ARCH_ESP32
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <HIDKeyboardTypes.h>
#include <HIDTypes.h>

// Déclaration de la structure InputReport
struct InputReport {
  uint8_t modifiers;      // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
  uint8_t reserved;       // doit être 0
  uint8_t pressedKeys[6]; // jusqu'à six touches pressées simultanément
};

// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),
    0x01, // Generic Desktop Controls
    USAGE(1),
    0x06, // Keyboard
    COLLECTION(1),
    0x01, // Application
    REPORT_ID(1),
    0x01, //   Report ID (1)
    USAGE_PAGE(1),
    0x07, //   Keyboard/Keypad
    USAGE_MINIMUM(1),
    0xE0, //   Keyboard Left Control
    USAGE_MAXIMUM(1),
    0xE7, //   Keyboard Right Control
    LOGICAL_MINIMUM(1),
    0x00, //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1),
    0x01,
    REPORT_COUNT(1),
    0x08, //   8 bits for the modifier keys
    REPORT_SIZE(1),
    0x01,
    HIDINPUT(1),
    0x02, //   Data, Var, Abs
    REPORT_COUNT(1),
    0x01, //   1 byte (unused)
    REPORT_SIZE(1),
    0x08,
    HIDINPUT(1),
    0x01, //   Const, Array, Abs
    REPORT_COUNT(1),
    0x06, //   6 bytes (for up to 6 concurrently pressed keys)
    REPORT_SIZE(1),
    0x08,
    LOGICAL_MINIMUM(1),
    0x00,
    LOGICAL_MAXIMUM(1),
    0x65, //   101 keys
    USAGE_MINIMUM(1),
    0x00,
    USAGE_MAXIMUM(1),
    0x65,
    HIDINPUT(1),
    0x00, //   Data, Array, Abs
    REPORT_COUNT(1),
    0x05, //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),
    0x01,
    USAGE_PAGE(1),
    0x08, //   LEDs
    USAGE_MINIMUM(1),
    0x01, //   Num Lock
    USAGE_MAXIMUM(1),
    0x05, //   Kana
    LOGICAL_MINIMUM(1),
    0x00,
    LOGICAL_MAXIMUM(1),
    0x01,
    HIDOUTPUT(1),
    0x02, //   Data, Var, Abs
    REPORT_COUNT(1),
    0x01, //   3 bits (Padding)
    REPORT_SIZE(1),
    0x03,
    HIDOUTPUT(1),
    0x01,             //   Const, Array, Abs
    END_COLLECTION(0) // End application collection
};

// Déclarations HID Bluetooth
BLEHIDDevice *hid;
BLECharacteristic *input_keyboard;
BLECharacteristic *input_mouse;
BLECharacteristic *output_keyboard;
bool isBleConnected = false;

const InputReport NO_KEY_PRESSED = {};

// Callbacks pour la connexion BLE
class BleHIDCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    isBleConnected = true;

    BLE2902 *cccDescKeyboard = (BLE2902 *)input_keyboard->getDescriptorByUUID(
        BLEUUID((uint16_t)0x2902));
    cccDescKeyboard->setNotifications(true);

    BLE2902 *cccDescMouse =
        (BLE2902 *)input_mouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDescMouse->setNotifications(true);

    Serial.println("Client connecté au clavier et souris HID Bluetooth.");
  }

  void onDisconnect(BLEServer *server) {
    isBleConnected = false;

    BLE2902 *cccDescKeyboard = (BLE2902 *)input_keyboard->getDescriptorByUUID(
        BLEUUID((uint16_t)0x2902));
    cccDescKeyboard->setNotifications(false);

    BLE2902 *cccDescMouse =
        (BLE2902 *)input_mouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDescMouse->setNotifications(false);

    Serial.println("Client déconnecté du clavier et souris HID Bluetooth.");
  }
};

// Callbacks pour les LEDs (Num Lock, Caps Lock, etc.)
class OutputCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    uint8_t *data = characteristic->getData();
    Serial.print("LED state (Bluetooth): ");
    Serial.println(*data, HEX);

    // Synchronisation des états des LEDs
    //deviceState.led_num = (*data & 0x01) != 0;  // Num Lock
    //deviceState.led_caps = (*data & 0x02) != 0; // Caps Lock

    // Suppression de la réactivation automatique de Caps Lock
    adbDevices.keyboardWriteLEDs(deviceState.led_num, deviceState.led_caps,
                                 deviceState.led_scroll);

    Serial.print("bluetooth LED Num Lock : ");
    Serial.println(deviceState.led_num ? "Allumée" : "Éteinte");
    Serial.print("bluetooth LED Caps Lock : ");
    Serial.println(deviceState.led_caps ? "Allumée" : "Éteinte");

#ifdef ARDUINO_ARCH_ESP32
    if (isBleConnected) {
      uint8_t buf[1] = {static_cast<uint8_t>((deviceState.led_caps << 1) |
                                             deviceState.led_num)};
      output_keyboard->setValue(buf, sizeof(buf));
      output_keyboard->notify();
    }
#endif
  }
};

void bluetoothTask(void *) {
  BLEDevice::init("Apple ADB Ressurector");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new BleHIDCallbacks());

  hid = new BLEHIDDevice(server);
  input_keyboard = hid->inputReport(1);   // Report ID 1 pour le clavier
  input_mouse = hid->inputReport(2);      // Report ID 2 pour la souris
  output_keyboard = hid->outputReport(1); // Report ID 1 pour les LEDs clavier
  output_keyboard->setCallbacks(new OutputCallbacks());

  hid->manufacturer()->setValue("Maker Community");
  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x02);

  BLESecurity *security = new BLESecurity();
  security->setAuthenticationMode(ESP_LE_AUTH_BOND);

  // Rapport HID pour clavier et souris
  hid->reportMap((uint8_t *)REPORT_MAP, sizeof(REPORT_MAP));
  hid->startServices();

  BLEAdvertising *advertising = server->getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->start();

  Serial.println("Bluetooth HID prêt.");
  delay(portMAX_DELAY);
}

void setupBluetoothTask() {
  xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
}

#endif

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
  return adbDevices.deviceUpdateRegister3(addr, reg3, mask.raw, &error) &&
         !error;
}

/**
 * @brief Fonction d'initialisation du programme.
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);   // Configuration de la pin LED
  digitalWrite(LED_PIN, LOW); // État initial de la LED

  Serial.begin(115200);
  Serial.println("Initialisation du programme...");

#ifdef ARDUINO_ARCH_ESP32
  setupBluetoothTask(); // Lancer la tâche Bluetooth
#endif

  hid_keyboard_init();
  hid_mouse_init();
  Serial.println("HID  initialisé.");

  adb.init(ADB_PIN, true);
  Serial.println("Bus ADB initialisé.");

  delay(1000);

  deviceState.keyboard_present =
      initializeDevice(ADBKey::Address::KEYBOARD, 0x03);
  Serial.print("Clavier détecté : ");
  Serial.println(deviceState.keyboard_present ? "Oui" : "Non");

  deviceState.mouse_present = initializeDevice(ADBKey::Address::MOUSE, 0x02);
  Serial.print("Souris détectée : ");
  Serial.println(deviceState.mouse_present ? "Oui" : "Non");

  digitalWrite(LED_PIN, HIGH); // Allumer la LED après l'initialisation

  adbDevices.keyboardWriteLEDs(deviceState.led_num, deviceState.led_caps,
                               deviceState.led_scroll);
  Serial.println("LEDs initialisées.");
}

/**
 * @brief Gère les événements du clavier.
 */
void handleKeyboard() {
  static hid_key_report key_report = {0};
  bool error = false;

  auto key_press = adbDevices.keyboardReadKeyPress(&error);
  if (error) {
    return;
  }

  bool report_changed =
      hid_keyboard_set_keys_from_adb_register(&key_report, key_press);

  // Gestion de Caps Lock
  if (key_press.data.key0 == ADBKey::KeyCode::CAPS_LOCK ||
      key_press.data.key1 == ADBKey::KeyCode::CAPS_LOCK) {
    bool is_pressed = (key_press.data.key0 == ADBKey::KeyCode::CAPS_LOCK &&
                       !key_press.data.released0) ||
                      (key_press.data.key1 == ADBKey::KeyCode::CAPS_LOCK &&
                       !key_press.data.released1);

    if (is_pressed) {
      // Activer Caps Lock
      deviceState.led_caps = true;
      Serial.println("Caps Lock activé.");
      report_changed = true;
    
      // Envoyer un événement de pression pour Caps Lock
      key_report.keys[0] = ADBKey::KeyCode::CAPS_LOCK;
      hid_keyboard_send_report(&key_report);
      Serial.println("Événement de pression de Caps Lock envoyé.");

      delay(100); // Attendre un court instant pour éviter les rebonds
      // Envoyer un événement de relâchement pour Caps Lock
      key_report.keys[0] = 0;
      hid_keyboard_send_report(&key_report);
      Serial.println("Événement de relâchement de Caps Lock envoyé.");
    } else {
      // Désactiver Caps Lock au relâchement
      deviceState.led_caps = false;
      Serial.println("Caps Lock désactivé.");
      report_changed = true;

      // Envoyer un événement de pression pour Caps Lock
      key_report.keys[0] = ADBKey::KeyCode::CAPS_LOCK;
      hid_keyboard_send_report(&key_report);
      Serial.println("Événement de pression de Caps Lock envoyé.");
      delay(100); // Attendre un court instant pour éviter les rebonds
                  // Envoyer un événement de relâchement pour Caps Lock
      key_report.keys[0] = 0;
      hid_keyboard_send_report(&key_report);
      Serial.println("Événement de relâchement de Caps Lock envoyé.");
    }
  }

  // Gestion de Num Lock
  if ((key_press.data.key0 == ADBKey::KeyCode::NUM_LOCK &&
       !key_press.data.released0) ||
      (key_press.data.key1 == ADBKey::KeyCode::NUM_LOCK &&
       !key_press.data.released1)) {
    deviceState.led_num = !deviceState.led_num;
    Serial.print("Num Lock LED (ADB) : ");
    Serial.println(deviceState.led_num ? "Allumée" : "Éteinte");
    report_changed = true;
  }

  if (report_changed) {
    Serial.println("Rapport clavier mis à jour.");
    hid_keyboard_send_report(&key_report);

#ifdef ARDUINO_ARCH_ESP32
    if (isBleConnected) {
      uint8_t buf[1] = {static_cast<uint8_t>((deviceState.led_caps << 1) |
                                             deviceState.led_num)};
      output_keyboard->setValue(buf, sizeof(buf));
      output_keyboard->notify();
    }
#endif
  }
}

/**
 * @brief Gère les événements de la souris.
 */
void handleMouse() {
  bool error = false;

  auto mouse_data = adbDevices.mouseReadData(&error);
  if (error || mouse_data.raw == 0) {
    return;
  }

  int8_t mouse_x = adbMouseConvertAxis(mouse_data.data.x_offset);
  int8_t mouse_y = adbMouseConvertAxis(mouse_data.data.y_offset);

  Serial.print("Mouvement souris - X: ");
  Serial.print(mouse_x);
  Serial.print(", Y: ");
  Serial.println(mouse_y);

  // Envoyer le rapport HID pour la souris
  hid_mouse_send_report(mouse_data.data.button ? 0 : 1, mouse_x, mouse_y);

#ifdef ARDUINO_ARCH_ESP32
  if (isBleConnected) {
    uint8_t buf[4] = {
        static_cast<uint8_t>(mouse_data.data.button ? 0 : 1), // Bouton
        static_cast<uint8_t>(mouse_x),                       // Déplacement X
        static_cast<uint8_t>(mouse_y),                       // Déplacement Y
        0                                                   // Molette (non utilisée)
    };
    input_mouse->setValue(buf, sizeof(buf));
    input_mouse->notify();
    Serial.println("Rapport HID souris envoyé via Bluetooth.");
  }
#endif
}

/**
 * @brief Boucle principale du programme.
 */
void loop() {
  if (deviceState.keyboard_present) {
    //  Serial.println("Gestion du clavier...");
    handleKeyboard();
    delay(POLL_DELAY);
  }

  if (deviceState.mouse_present) {
    //  Serial.println("Gestion de la souris...");
    handleMouse();
    delay(POLL_DELAY);
  }
}

#endif