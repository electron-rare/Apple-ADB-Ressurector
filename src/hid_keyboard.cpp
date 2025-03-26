#include "hid_keyboard.h"
#include "usbd_hid_composite_if.h"
#include <Arduino.h>

void hid_keyboard_init()
{
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_Init(HID_KEYBOARD);
#endif
}

void hid_keyboard_close()
{
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_DeInit(HID_KEYBOARD);
#endif
}

void hid_keyboard_send_report(hid_key_report* report)
{
    uint8_t buf[8] = {report->modifiers, 0, report->keys[0],
        report->keys[1], report->keys[2], report->keys[3], report->keys[4],
        report->keys[5]
    };
#ifdef PIO_FRAMEWORK_ARDUINO_ENABLE_HID
    HID_Composite_keyboard_sendReport(buf, 8);
#endif
}

bool hid_keyboard_set_keys_from_adb_register(
        hid_key_report* report, adb_data<adb_kb_keypress> key_press) {
    // Power button est un cas spécial, présent dans les deux octets:
    if (key_press.raw == ADBKey::KeyCode::POWER_DOWN)
        return hid_keyboard_update_key_in_report(report, KEY_POWER, false);
    else if (key_press.raw == ADBKey::KeyCode::POWER_UP)
        return hid_keyboard_update_key_in_report(report, KEY_POWER, true);

    // Autres touches:
    bool report_changed = false;
    // Higher octet key:
    uint8_t key0 = key_press.data.key0;
    if (ADBKeymap::isModifier(key0))
        report_changed = hid_keyboard_update_modifier_in_report(
                report, key0, key_press.data.released0);
    else
        report_changed = hid_keyboard_update_key_in_report(
                report, ADBKeymap::toHID(key0), key_press.data.released0);

    // Lower octet key:
    uint8_t key1 = key_press.data.key1;
    if (ADBKeymap::isModifier(key1))
        report_changed = hid_keyboard_update_modifier_in_report(
                report, key1, key_press.data.released1) || report_changed;
    else
        report_changed = hid_keyboard_update_key_in_report(
                report, ADBKeymap::toHID(key1), key_press.data.released1) || report_changed;
    
    return report_changed;
}

bool hid_keyboard_set_modifiers_from_adb_register(
        hid_key_report* report, adb_data<adb_kb_keypress> reg) {
    // TODO: don't seem necessary, as modifiers still register normal keypresses
    return false;
}

bool hid_keyboard_update_key_in_report(hid_key_report* report, uint8_t hid_keycode, bool released) {
    // Sometimes the response from the ADB keyboard is rubbish:
    // either an invalid key, or lifting a key that's not been pressed.
    // This detects the first case, so we don't waste time processing it.
    if (hid_keycode == KEY_NONE) return false;

    if (released)
        return hid_keyboard_remove_key_from_report(report, hid_keycode);
    else
        return hid_keyboard_add_key_to_report(report, hid_keycode);
}

bool hid_keyboard_update_modifier_in_report(hid_key_report* report, uint8_t adb_keycode, bool released) {
    uint8_t mask = ADBKeymap::getModifierMask(adb_keycode);
    if (mask == 0) return false;

    // No change if the modifier is already set
    if (released == !(report->modifiers & mask)) return false;

    // Flip the modifier
    if (!released) report->modifiers |= mask;
    else report->modifiers &= ~mask;

    return true;
}

// Returns true if after execution the key is in the report
// in other words, returns false if insertion unsuccessful (report unchanged)
bool hid_keyboard_add_key_to_report(hid_key_report* report, uint8_t hid_keycode) {
    int8_t free_slot = -1;

    for (uint8_t i = 0; i < KEY_REPORT_KEYS_COUNT; i++) {
        if (report->keys[i] == hid_keycode)
            return true; // key is already in the report

        if (report->keys[i] == 0 && free_slot == -1)
            free_slot = i; // memorise empty entry idx
    }

    if (free_slot == -1)
        return false; // key report is full
    
    report->keys[free_slot] = hid_keycode;
    return true;
}

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
