/** Project: RFID-based door lock
 * Part of the course HCMUS-PHY00006-18CLC-KTPM2 "Internet of Things"
 * 
 * This is part 1 of 2 of the project: code for the physical door lock.
 * Part 2 of 2 (server-side stuff) is available as a Node-RED flow.
 * See the report file alongside this code for context.
 *
 * Members (3):
 *      18127124 Hoang Xuan Kiet (initial versions)
 *      18127126 Huynh Duc Le (hardware layout)
 *      18127221 Bui Van Thien (hardware layout; bugfixes and enhancements)
 * 
 */

#include "includes.h"
#include "config.h"
#include "helpers.h"
#include "events.h"

////////////////////////////////////////////////////////////////////////////////

void setup() {
    // Serial.begin(115200);

    // The internal LED is used as init indicator.
    pinMode(P_INTERNAL_LED, OUTPUT);
    digitalWrite(P_INTERNAL_LED, 1);

    // Init. LCD
    Wire.begin(P_LCD_SDA, P_LCD_SCK);
    lcd.init();
    lcdReset();

    // LCD: preferences
    lcd.noCursor();
    lcd.noAutoscroll();
    lcd.setBacklight(1);

    // LCD: add custom characters
    lcd.createChar(CHAR_WAITING_CPOINT, new byte[8] CHAR_WAITING);
    lcd.createChar(CHAR_STANDBY_CPOINT, new byte[8] CHAR_STANDBY);
    lcd.createChar(CHAR_UNLOCK_CPOINT, new byte[8] CHAR_UNLOCK);
    lcd.createChar(CHAR_FAIL_CPOINT, new byte[8] CHAR_FAIL);

    // Init. Wi-Fi and try to connect to previous AP.
    // If failed, the manager will kick in.
    lcdWriteLine(0, "  Waiting for");
    lcd.setCursor(0, 0); lcd.write(CHAR_WAITING_CPOINT);
    lcdWriteLine(1, "Wi-Fi...");
    WiFi.mode(WIFI_STA);
    if (!wifiManager.autoConnect()) {
        ESP.restart();
        delay(1000);
    }

    // Init. MQTT client + ThingSpeak
    mqttClient.setCallback(mqttCallback);
    mqttConnect();
    mqttClient.publish(MQTT_TOPIC_POWER_ON, "1");

    // Init. NTP
    ntp.begin();

    // Init. DHT11
    dht.setup(P_DHT11, DHTesp::DHT11);

    // Init. Buzzer + Motion sensor + Relay
    pinMode(P_BUZZER, OUTPUT);
    pinMode(P_RELAY, OUTPUT);
    digitalWrite(P_RELAY, LOW);

    // Sensitive pins
    pinMode(__DEBUG_SHOW_UID_PIN__, INPUT);
    if (__DEBUG_RESET_WIFI_ENABLED__) pinMode(__DEBUG_RESET_WIFI_PIN__, INPUT);

    // Init. RC-522
    SPI.begin();
    mfrc522.PCD_Init();

    // Turn off the internal LED to indicate init stage is done.
    digitalWrite(P_INTERNAL_LED, 0);
}

////////////////////////////////////////////////////////////////////////////////

void loop()
{
    lcd.noBlink();
    eventStandby();

    // If the reset-wifi pin is set high, clear Wi-Fi setting on next boot
    if (digitalRead(__DEBUG_RESET_WIFI_PIN__)) {
        WiFi.disconnect(false, true);
        delay(1000);
        ESP.restart();
    }

    // Detect if the MQTT connection is dropped; retry if needed
    if (!mqttClient.connected()) mqttConnect();
    mqttClient.loop();

    // Dump RC522 status
    // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    // Look for new cards || Select one of the cards
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
        return;

    // Read the current card's UID
    for (byte i = 0; i < 4; i++)
    {
        CURRENT_CARD[i] = 0x0; // just in case
        CURRENT_CARD[i] = mfrc522.uid.uidByte[i];
    }

    // Check the card
    if (isValidCard()) { eventUnlockOK(); } else { eventUnlockFailed(); }
}

