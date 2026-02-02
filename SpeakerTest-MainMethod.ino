#include <WiFi.h>
#include <HTTPClient.h>
#include "AudioFileSourceHTTPStream.h" // Streams directly from URL
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// Your NEW Pin Map (To avoid conflicts with the Mic)
#define I2S_BCK 6
#define I2S_LRC 5
#define I2S_DOUT 7
//https://www.learningcontainer.com/wp-content/uploads/2020/02/Kalimba.mp3

// WiFi Credentials
const char* ssid     = "Maru_2g";
const char* password = "Jio123456";

// Audio Objects
AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStream *file;
AudioOutputI2S *out;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Jai Prabhu - OceanLabz Method");

  // 1. Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // 2. Setup Audio Output to I2S DAC (MAX98357A)
  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCK, I2S_LRC, I2S_DOUT);
  out->SetGain(0.1); // Start low (0.0 to 4.0)

  // 3. Connect to a Direct MP3 URL
  file = new AudioFileSourceHTTPStream("https://www.learningcontainer.com/wp-content/uploads/2020/02/Kalimba.mp3");
  
  // 4. Start the Generator
  mp3 = new AudioGeneratorMP3();
  if (!mp3->begin(file, out)) {
    Serial.println("MP3 initialization failed!");
  } else {
    Serial.println("Playback started!");
  }
}

void loop() {
  // Required for the OceanLabz / ESP8266Audio method
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.println("MP3 Done");
    delay(1000);
  }
}
