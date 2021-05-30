/** Project: RFID-based door lock
 * Helper functions
 */

#pragma once

#include "includes.h"
#include "config.h"

void buzzerAlert(bool);
void lcdClearLine(byte);
void lcdWriteLine(byte, const char*);
void lcdReset();
bool isValidCard();
bool writeToMQTT();

////////////////////////////////////////////////////////////////////////////////

// Sensitive setting.
bool __DEBUG_SHOW_UID__() {
    return (__DEBUG_SHOW_UID_ENABLED__ && digitalRead(__DEBUG_SHOW_UID_PIN__));
}

// Turn card UID into human-readable hex string
String __DEBUG_PRETTY_PRINT_UID__(byte card[]) {
    String r = "";
    for (int i = 0; i < UID_SIZE; i++) {
        int low_bits = card[i] & 0x0F;
        int high_bits = card[i] >> 4;
        r += H[high_bits];
        r += H[low_bits];
        if (i < 4 - 1) r += " ";
    }
    return r;
}

/** Play a tone depending on the auth. status reported by the card reader.
 * @params ok Auth. status reported by the card reader ("is the card valid?")
 * @returns void
 * 
 * ESP32 does not have native functions for buzzer tones (think tone() and noTone())
 * like AVR-based boards do. The nearest equivalent of this is done by using
 * the generic PWM LED controller (the "ledc" driver) for the ESP32 platform.
 * Code adapted from the following sources:
 *      https://github.com/espressif/arduino-esp32/issues/1720#issuecomment-625489288
 *      https://community.platformio.org/t/tone-not-working-on-espressif32-platform/7587/2
 */
void buzzerAlert(bool ok) {
    int l, f, d;
    if (ok) {
        l = BUZZER_OK_LOOP;
        f = BUZZER_OK_FREQ;
        d = BUZZER_OK_DELAY;
    }
    else {
        l = BUZZER_FAIL_LOOP;
        f = BUZZER_FAIL_FREQ;
        d = BUZZER_FAIL_DELAY;
    }

    for (int i = 0; i < l; i++) {
        ledcSetup(0, f, 8); // Channel 0, 8-bit resolution
        ledcAttachPin(P_BUZZER, 0);
        ledcWriteTone(0, f);
        delay(d);
        ledcWrite(0, 0);
        delay(BUZZER_INTER_BREAK);
    }
}

// Clear a line on screen, then move the cursor back to beginning of line
void lcdClearLine(byte line) {
    if (line < 0 || line >= LCD_HEIGHT) return;
    lcd.setCursor(0, line);
    for (int i = 0; i < LCD_WIDTH; i++) {
        lcd.setCursor(i, line);
        lcd.write(0x20);
    }
    lcd.setCursor(0, line);
}

// Clear and write a line (convenient function)
void lcdWriteLine(byte line, const char* text) {
    lcdClearLine(line);
    lcd.print(text);
}

// Clear and write a line (convenient function)
void lcdWriteLine(byte line, const String& text) {
    lcdClearLine(line);
    lcd.print(text);
}

// Clear the whole screen, then move the cursor to (0,0)
void lcdReset() {
    lcd.clear();
    lcd.setCursor(0, 0);
}

// Check current card UID against valid card list
bool isValidCard() {
    for (int card = 0; card < VALID_CARD_LIST_SIZE; card++) {
        bool thisValid = true;
        for (int i = 0; i < UID_SIZE; i++) {
            thisValid &= CURRENT_CARD[i] == VALID_CARDS[card][i];
        }
        if (thisValid) return true;
    }
    return false;
}

// Read the DHT sensor
void pollDHT() {
    dht_temp = dht.getTemperature();
    dht_humid = dht.getHumidity();
}

// Periodically push "weather" (temperature + humidity) update
void pushWeatherUpdate(int current_sec) {
    if (weather_update_countdown == 0) {
        char buffer[3];

        // Write to MQTT: temp.
        String(int(dht_temp)).toCharArray(buffer, 3);
        mqttClient.publish(MQTT_TOPIC_TEMPERATURE, buffer);
        
        // Delay for 500ms
        delayMicroseconds(500000);
        
        // Write to MQTT: humid.
        String(int(dht_humid)).toCharArray(buffer, 3);
        mqttClient.publish(MQTT_TOPIC_HUMIDITY, buffer);

        // Reset countdown
        weather_update_countdown = WEATHER_UPDATE_DELAY;
        return;
    }

    else if (current_sec != weather_update_current_sec) {
        // Count down until defined interval has passed
        weather_update_current_sec = current_sec;
        weather_update_countdown -= 1;
    }
}

