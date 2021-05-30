/** Project: RFID-based door lock
 * Configuration file (potentially sensitive info only)
 * Scrub this file before putting it anywhere online.
 */

#pragma once

#include "includes.h"

// Sensitive setting: "debug pin" to force show UID on line 1 instead of unlock status
// UIDs can be arbitrarily rewritten on a card, so be careful.
#define __DEBUG_SHOW_UID_ENABLED__ false
#define __DEBUG_SHOW_UID_PIN__ 0

// Quick and dirty way to reset Wi-Fi
#define __DEBUG_RESET_WIFI_ENABLED__ false
#define __DEBUG_RESET_WIFI_PIN__ 0

// NOTE: the size for UID storage should be "mfrc522.uid.size" instead of 4,
// but that is what we typically get with our keys while testing.
#define UID_SIZE 4

// Allowed card UID
// Remember to change the list size if updated
#define VALID_CARD_LIST_SIZE 1
const byte VALID_CARDS[VALID_CARD_LIST_SIZE][UID_SIZE] = {
    {0xAB, 0xCD, 0xEF, 0x00}
};

// MQTT
#define MQTT_SERVER_HOST "{BROKER_HOST}" // HiveMQ Public MQTT Broker
#define MQTT_SERVER_PORT {BROKER_PORT}

