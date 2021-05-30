/** Project: RFID-based door lock
 * Libraries
 */

#pragma once

// Time formatting
#include <ctime>

// Protocols (built-in libraries)
#include <SPI.h>    // SPI (used by RFID reader)
#include <Wire.h>   // I2C (used by LCD)

// RC-522 RFID tag reader
// https://github.com/miguelbalboa/rfid
#include <MFRC522.h>

// LCD over I2C
// https://github.com/marcoschwartz/LiquidCrystal_I2C
#include <LiquidCrystal_I2C.h>

// DHT11 (Temperature + Humidity) sensor
// https://github.com/beegee-tokyo/DHTesp
#include <DHTesp.h>

// Internet time
// https://github.com/FWeinb/NTPClient
#include <NTPClient.h>

// Wi-Fi connection manager with web-based GUI
// https://github.com/tzapu/WiFiManager.git
#include <WiFiManager.h>

// MQTT Client
// http://pubsubclient.knolleary.net/
#include <PubSubClient.h>

