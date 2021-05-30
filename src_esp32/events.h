/** Project: RFID-based door lock
 * Event routines
 */

#include "config.h"
#include "helpers.h"

void mqttConnect();
void mqttCallback(char*, byte*, unsigned int);
void eventUnlockOK(byte);
void eventUnlockFailed();
void eventStandby();

////////////////////////////////////////////////////////////////////////////////

// (Re)connect to MQTT broker
void mqttConnect()
{
    isStandingBy = false;
    
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        lcdReset();
        lcdWriteLine(0, "  Waiting for");
        lcd.setCursor(0, 0); lcd.write(CHAR_WAITING_CPOINT);
        lcdWriteLine(1, "MQTT client...");

        // Create a random client ID (why?)
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str())) {
            mqttClient.subscribe(MQTT_TOPIC_UNLOCK_REMOTE);
            return;
        }
        else {
            delay(MQTT_RECONNECT_DELAY);
        }
    }
}

// React to remote unlock commands
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    // Manually convert the payload to string
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    // What does the string says?

    if (message.equals(MQTT_PAYLOAD_UNLOCK_BY_TELEGRAM)) {
        eventUnlockOK(UNLOCK_TELEGRAM);
        return;
    }

    if (message.equals(MQTT_PAYLOAD_UNLOCK_BY_WEB)) {
        eventUnlockOK(UNLOCK_WEB);
        return;
    }
}

// Unlock the door
void eventUnlockOK(byte unlockType = UNLOCK_CARD_OK)
{
    isStandingBy = false;

    // Write to MQTT + ThingSpeak
    switch (unlockType) {
        case UNLOCK_CARD_OK:
            mqttClient.publish(MQTT_TOPIC_UNLOCK_STATUS, MQTT_PAYLOAD_UNLOCKED_CARD_OK);
            break;
        case UNLOCK_TELEGRAM:
            mqttClient.publish(MQTT_TOPIC_UNLOCK_STATUS, MQTT_PAYLOAD_UNLOCKED_TELEGRAM);
            break;
        case UNLOCK_WEB:
            mqttClient.publish(MQTT_TOPIC_UNLOCK_STATUS, MQTT_PAYLOAD_UNLOCKED_WEB);
            break;
    }

    // LCD: Write status
    switch (unlockType) {
        case UNLOCK_TELEGRAM:
            lcdWriteLine(0, "  Telegram");
            lcd.setCursor(0, 0); lcd.write(CHAR_UNLOCK_CPOINT);
            break;
        case UNLOCK_WEB:
            lcdWriteLine(0, "  Web");
            lcd.setCursor(0, 0); lcd.write(CHAR_UNLOCK_CPOINT);
            break;
        case UNLOCK_CARD_OK:
            if (__DEBUG_SHOW_UID__()) {
                lcdWriteLine(0, "  " + __DEBUG_PRETTY_PRINT_UID__(CURRENT_CARD));
                lcd.setCursor(0, 0); lcd.write(CHAR_UNLOCK_CPOINT);
            }
            else {
                lcdWriteLine(0, "  Local card");
                lcd.setCursor(0, 0); lcd.write(CHAR_UNLOCK_CPOINT);
            }
            break;
    }
    lcdWriteLine(1, "Door close in ");
    
    // Buzzer: play tone
    buzzerAlert(true);

    // Trigger the relay to open the door
    digitalWrite(P_RELAY, HIGH);

    // Delay before retrigger
    int t = LCD_WAIT_OK;
    while (t)
    {
        lcd.setCursor(14, 1); lcd.print("  "); // >10s display bug
        lcd.setCursor(14, 1); lcd.print(t);
        delay(1000);
        t -= 1;
    }

    // Close the door
    digitalWrite(P_RELAY, LOW);
}

// Reject unlocking the door (invalid card)
void eventUnlockFailed()
{
    isStandingBy = false;

    // Write to MQTT
    mqttClient.publish(MQTT_TOPIC_UNLOCK_STATUS, MQTT_PAYLOAD_UNLOCKED_CARD_FAIL);

    // LCD: write status
    if (__DEBUG_SHOW_UID__()) {
        lcdWriteLine(0, "  " + __DEBUG_PRETTY_PRINT_UID__(CURRENT_CARD));
        lcd.setCursor(0, 0); lcd.write(CHAR_FAIL_CPOINT);
    }
    else {
        lcdWriteLine(0, "  Unlock failed");
        lcd.setCursor(0, 0); lcd.write(CHAR_FAIL_CPOINT);
    }
    lcdWriteLine(1, "Impostor!"); // lol

    // Buzzer: play tone
    buzzerAlert(false);

    // Delay before retry
    int t = LCD_WAIT_FAIL;
    while (t)
    {
        delay(1000);
        t -= 1;
    }
}

// Standby mode
void eventStandby()
{
    // Returned from other events: rewrite template strings
    // This is to prevent unnecessary rewrites on static strings
    if (!isStandingBy) {
        lcdReset();
        delayMicroseconds(100000); // 100ms delay for "extra assurance"
        lcdWriteLine(0, "--:--- --/-- --" + CHAR_DEGREE_SIGN); // time, date, temp
        lcdWriteLine(1, "  Insert card");
        lcd.setCursor(0, 1); lcd.write(CHAR_STANDBY_CPOINT);
        isStandingBy = true;
    }

    // Poll stuff
    pollDHT();
    ntp.update();

    char dt_buffer[13];
    time_t now = ntp.getEpochTime();
    tm ts = *localtime(&now);
    strftime(dt_buffer, 13, "%I:%M  %d/%m", &ts);
    dt_buffer[5] = ts.tm_hour > 12 ? 'p' : 'a';

    lcd.setCursor(0, 0); lcd.print(dt_buffer);

    // A dirty fix for when DHT11 just can't give readable temperature + humidity
    // Based on DHT11's guaranteed temperature + humidity range.
    if (dht_temp >= 0 && dht_temp <= 50) {
        lcd.setCursor(13, 0); lcd.print(int(dht_temp));
    }
    
    // Periodically push "weather" update
    pushWeatherUpdate(ts.tm_sec);
}

