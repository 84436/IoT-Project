/** Project: RFID-based door lock
 * Configuration file
 */

#pragma once

#include "includes.h"
#include "config_sensitive.h"

////////////////////////////////////////////////////////////////////////////////
// CONSTANTS
// Except for valid_cards[][] and pin configuration (P_*),
// please don't change anything else unless you know exactly what you're doing.

// hexToString(): Please do not touch this.
const char* H = "0123456789ABCDEF";

// Placeholder variables for current card UID
static byte CURRENT_CARD[UID_SIZE];

// ESP32 internal LED
#define P_INTERNAL_LED 2

// RC522 (RFID)
#define P_RC522_SDA 5  // originally D4
#define P_RC522_RST 4 // originally D3
static MFRC522 mfrc522(P_RC522_SDA, P_RC522_RST);

// LCD (status)
#define P_LCD_SDA 21 // originally D2
#define P_LCD_SCK 22 // originally D1
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
static LiquidCrystal_I2C lcd(0x27, 16, 2);
static bool isStandingBy = false;

// Relay (physical lock)
#define P_RELAY 27 // originally D0
#define LCD_WAIT_OK 3
#define LCD_WAIT_FAIL 2

// Buzzer (beep)
#define P_BUZZER 32 // originally D8
#define BUZZER_OK_FREQ 4500
#define BUZZER_OK_DELAY 100
#define BUZZER_OK_LOOP 2
#define BUZZER_FAIL_FREQ 500
#define BUZZER_FAIL_DELAY 200
#define BUZZER_FAIL_LOOP 3
#define BUZZER_INTER_BREAK 50

// DHT11 (temperature)
#define P_DHT11 33
static DHTesp dht;
static float dht_temp;
static float dht_humid;

// Degree (temperature) sign
// This is not guaranteed to exist at the same code point on every LCD.
// Refer to the CGROM table of the LCD (or perform a simple sweep from 0x00 to 0xFF) to find out.
const String CHAR_DEGREE_SIGN = String(char(0xDF));

// NTP (time)
#define NTP_TIME_POOL "vn.pool.ntp.org"
#define NTP_TIME_ZONE +7
static WiFiUDP ntpudp;
static NTPClient ntp(ntpudp, NTP_TIME_POOL, NTP_TIME_ZONE * 3600);

// Wi-Fi
static WiFiManager wifiManager;
static WiFiClient wifiClient;

// MQTT
#define MQTT_RECONNECT_DELAY 3000 // 3s
static PubSubClient mqttClient(MQTT_SERVER_HOST, MQTT_SERVER_PORT, wifiClient);

// Event types
enum EVENT {
    // Doorlock has powered back on (from a possible power failure)
    POWER_ON,

    // Unlocked by local card
    UNLOCK_CARD_OK,

    // Failed to unlock door (invalid card)
    UNLOCK_CARD_FAIL,

    // Unlock command sent from Telegram bot
    UNLOCK_TELEGRAM,

    // Unlock command sent from web UI
    UNLOCK_WEB,
};

// MQTT: channels + payloads (sub)
#define MQTT_TOPIC_UNLOCK_REMOTE        "stupid-door-REDACTED/unlock-remote"
#define MQTT_PAYLOAD_UNLOCK_BY_TELEGRAM "telegram>>unlock"
#define MQTT_PAYLOAD_UNLOCK_BY_WEB      "web>>unlock"

// MQTT: channels + payloads (pub)
#define MQTT_TOPIC_UNLOCK_STATUS        "stupid-door-REDACTED/unlock-status"
#define MQTT_TOPIC_POWER_ON             "stupid-door-REDACTED/poweron"
#define MQTT_TOPIC_TEMPERATURE          "stupid-door-REDACTED/temp"
#define MQTT_TOPIC_HUMIDITY             "stupid-door-REDACTED/humid"
#define MQTT_PAYLOAD_UNLOCKED_CARD_OK   "unlocked:card"
#define MQTT_PAYLOAD_UNLOCKED_CARD_FAIL "unlock-fail:card"
#define MQTT_PAYLOAD_UNLOCKED_TELEGRAM  "unlocked:telegram"
#define MQTT_PAYLOAD_UNLOCKED_WEB       "unlocked:web"

// Interval between "weather" updates
#define WEATHER_UPDATE_DELAY 5
static byte weather_update_countdown = 0; // update right after boot
static byte weather_update_current_sec = -1;

// LCD: custom characters
#define CHAR_WAITING_CPOINT 1
#define CHAR_WAITING { B11111, B10001, B01010, B00100, B01010, B10001, B11111, B00000 }

#define CHAR_STANDBY_CPOINT 2
#define CHAR_STANDBY { B11111, B10001, B10101, B10101, B10101, B10001, B11111, B00000 }

#define CHAR_UNLOCK_CPOINT 3
#define CHAR_UNLOCK { B11011, B11001, B11011, B11011, B10101, B11011, B11111, B00000 }

#define CHAR_FAIL_CPOINT 4
#define CHAR_FAIL { B11111, B10101, B11011, B10101, B11111, B10001, B11111, B00000 }

