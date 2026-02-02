#include "Arduino.h"
#include "Audio.h" // The ESP32-audioI2S library
#include "WiFi.h"

// Your NEW Pin Map (To avoid conflicts with the Mic)
#define I2S_BCK 6
#define I2S_LRC 5
#define I2S_DOUT 7
Audio audio;

void setup() {
    Serial.begin(115200);
    delay(1000); // Give the S3 a moment to settle
    Serial.println("Jai Prabhu");

    // Connect to WiFi first
    WiFi.begin("Maru_2g", "Jio123456");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // Setup Speaker pins
    audio.setPinout(I2S_BCK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12); // Range 0-21

    // Start audio stream
    audio.connecttohost("http://stream.zeno.fm/0r0xa792kwzuv"); 
}

void loop() {
    audio.loop(); // Required for processing
}
