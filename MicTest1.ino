#include <Arduino.h>
#include <driver/i2s.h>
 
// INMP441 Microphone pins
#define I2S_MIC_SERIAL_CLOCK     5   // SCK
#define I2S_MIC_LEFT_RIGHT_CLOCK 6  // WS  
#define I2S_MIC_SERIAL_DATA      4   // SD
 
// Recording settings
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024
#define RECORD_SECONDS 5
 
// I2S configuration
i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};
 
i2s_pin_config_t i2s_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA
};
 
void setup() {
  Serial.begin(115200);
  delay(1000);
   
  Serial.println("\n==================================");
  Serial.println("     INMP441 Microphone Test");
  Serial.println("==================================");
   
  Serial.println("\nPin Configuration:");
  Serial.printf("  SCK (BCLK): GPIO%d\n", I2S_MIC_SERIAL_CLOCK);
  Serial.printf("  WS (LRC):   GPIO%d\n", I2S_MIC_LEFT_RIGHT_CLOCK);
  Serial.printf("  SD (DATA):  GPIO%d\n", I2S_MIC_SERIAL_DATA);
  Serial.printf("  Sample Rate: %d Hz\n", SAMPLE_RATE);
   
  Serial.println("\nAvailable Tests:");
  Serial.println("  1. Type '1' - Quick noise detection (3 seconds)");
  Serial.println("  2. Type '2' - Raw data monitor (10 samples)");
  Serial.println("  3. Type '3' - Amplitude test");
  Serial.println("  4. Type '4' - Continuous monitoring");
  Serial.println("\nType number to start test...");
}
 
void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
     
    switch(command) {
      case '1':
        testNoiseDetection();
        break;
      case '2':
        testRawData();
        break;
      case '3':
        testAmplitude();
        break;
      case '4':
        testContinuous();
        break;
      case 'p':
        printPinDiagram();
      case '\n':
      case '\r':
        break;
      default:
        Serial.println("\nInvalid command. Type 1-5");
    }
  }
  delay(10);
}
 
// Test 1: Quick noise detection
void testNoiseDetection() {
  Serial.println("\n=== Test 1: Quick Noise Detection ===");
  Serial.println("Listening for 3 seconds...");
  Serial.println("Make noise or speak into the microphone");
   
  // Initialize I2S
  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
    Serial.println("ERROR: Failed to install I2S driver");
    return;
  }
   
  if (i2s_set_pin(I2S_NUM_0, &i2s_pins) != ESP_OK) {
    Serial.println("ERROR: Failed to set I2S pins");
    i2s_driver_uninstall(I2S_NUM_0);
    return;
  }
   
  delay(200); // Allow I2S to stabilize
   
  int32_t sample;
  size_t bytesRead;
  unsigned long startTime = millis();
  int audioEvents = 0;
  int maxAmplitude = 0;
   
  while (millis() - startTime < 3000) {
    if (i2s_read(I2S_NUM_0, (char*)&sample, sizeof(sample), &bytesRead, 100) == ESP_OK) {
      if (bytesRead == sizeof(sample)) {
        // Convert 32-bit to 16-bit
        int16_t sample16bit = (int16_t)(sample >> 8);
        int amplitude = abs(sample16bit);
         
        if (amplitude > 100) { // Noise threshold
          audioEvents++;
          if (amplitude > maxAmplitude) {
            maxAmplitude = amplitude;
          }
           
          if (audioEvents <= 3) {
            Serial.printf("  Audio detected! Value: %d\n", sample16bit);
          }
        }
      }
    }
  }
   
  i2s_driver_uninstall(I2S_NUM_0);
   
  Serial.println("\n--- Results ---");
  Serial.printf("  Audio events detected: %d\n", audioEvents);
  Serial.printf("  Maximum amplitude: %d\n", maxAmplitude);
   
  if (audioEvents == 0) {
    Serial.println("  ❌ FAIL: No audio detected");
    Serial.println("  Check: Wiring, power, microphone orientation");
  } else if (audioEvents < 5) {
    Serial.println("  ⚠️  WARNING: Low audio sensitivity");
    Serial.println("  Check: Distance from sound source");
  } else {
    Serial.println("  ✅ PASS: Microphone working");
  }
}
 
// Test 2: Raw data monitoring
void testRawData() {
  Serial.println("\n=== Test 2: Raw Data Monitoring ===");
  Serial.println("Displaying raw I2S data (10 samples)...");
   
  // Initialize I2S
  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
    Serial.println("ERROR: Failed to install I2S driver");
    return;
  }
   
  if (i2s_set_pin(I2S_NUM_0, &i2s_pins) != ESP_OK) {
    Serial.println("ERROR: Failed to set I2S pins");
    i2s_driver_uninstall(I2S_NUM_0);
    return;
  }
   
  delay(200);
   
  Serial.println("\nRaw 32-bit samples:");
  int32_t sample;
  size_t bytesRead;
  int samplesRead = 0;
   
  while (samplesRead < 500) {
    if (i2s_read(I2S_NUM_0, (char*)&sample, sizeof(sample), &bytesRead, 100) == ESP_OK) {
      if (bytesRead == sizeof(sample)) {
        samplesRead++;
         
        // Convert to 16-bit for readability
        int16_t sample16bit = (int16_t)(sample >> 8);
         
        Serial.printf("  Sample %d: Raw=0x%08X (%10d) -> 16-bit=%6d",
                     samplesRead, sample, sample, sample16bit);
         
        if (sample == 0) {
          Serial.println(" [ZERO - Check connections]");
        } else if (abs(sample16bit) < 100) {
          Serial.println(" [QUIET]");
        } else {
          Serial.println(" [OK]");
        }
         
        delay(50); // Slow down for readability
      }
    }
  }
   
  i2s_driver_uninstall(I2S_NUM_0);
   
  Serial.println("\nAnalysis:");
  Serial.println("  • All zeros: Check power and wiring");
  Serial.println("  • Small values (<100): Normal for quiet environment");
  Serial.println("  • Large values: Microphone is picking up sound");
}
 
// Test 3: Amplitude test
void testAmplitude() {
  Serial.println("\n=== Test 3: Amplitude Test ===");
  Serial.println("Clap or speak loudly near the microphone...");
   
  // Initialize I2S
  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
    Serial.println("ERROR: Failed to install I2S driver");
    return;
  }
   
  if (i2s_set_pin(I2S_NUM_0, &i2s_pins) != ESP_OK) {
    Serial.println("ERROR: Failed to set I2S pins");
    i2s_driver_uninstall(I2S_NUM_0);
    return;
  }
   
  delay(100);
   
  Serial.println("Listening for 5 seconds...");
  Serial.println("Make different sounds to test response");
   
  int32_t sample;
  size_t bytesRead;
  unsigned long startTime = millis();
  int minAmplitude = 32767;
  int maxAmplitude = 0;
  int totalSamples = 0;
  int quietSamples = 0;
   
  while (millis() - startTime < 5000) {
    if (i2s_read(I2S_NUM_0, (char*)&sample, sizeof(sample), &bytesRead, 100) == ESP_OK) {
      if (bytesRead == sizeof(sample)) {
        totalSamples++;
        int16_t sample16bit = (int16_t)(sample >> 8);
        int amplitude = abs(sample16bit);
         
        if (amplitude < minAmplitude) minAmplitude = amplitude;
        if (amplitude > maxAmplitude) maxAmplitude = amplitude;
         
        if (amplitude < 50) quietSamples++;
         
        // Show live amplitude every 100 samples
        if (totalSamples % 100 == 0) {
          Serial.printf("  Amplitude: %6d (Min: %6d, Max: %6d)\n", 
                       amplitude, minAmplitude, maxAmplitude);
        }
      }
    }
  }
   
  i2s_driver_uninstall(I2S_NUM_0);
   
  Serial.println("\n--- Results ---");
  Serial.printf("  Samples analyzed: %d\n", totalSamples);
  Serial.printf("  Minimum amplitude: %d\n", minAmplitude);
  Serial.printf("  Maximum amplitude: %d\n", maxAmplitude);
  Serial.printf("  Quiet samples (<50): %d (%.1f%%)\n", 
                quietSamples, (quietSamples * 100.0) / totalSamples);
   
  if (maxAmplitude < 1000) {
    Serial.println("  ⚠️  Low sensitivity - try speaking louder");
  } else if (maxAmplitude > 20000) {
    Serial.println("  ⚠️  Very high amplitude - microphone may be too close");
  } else {
    Serial.println("  ✅ Good amplitude range");
  }
}
 
// Test 4: Continuous monitoring
void testContinuous() {
  Serial.println("\n=== Test 4: Continuous Monitoring ===");
  Serial.println("Type 's' to stop monitoring");
  Serial.println("Monitoring audio levels...");
   
  // Initialize I2S
  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
    Serial.println("ERROR: Failed to install I2S driver");
    return;
  }
   
  if (i2s_set_pin(I2S_NUM_0, &i2s_pins) != ESP_OK) {
    Serial.println("ERROR: Failed to set I2S pins");
    i2s_driver_uninstall(I2S_NUM_0);
    return;
  }
   
  delay(200);
   
  int32_t sample;
  size_t bytesRead;
  unsigned long lastDisplay = 0;
  bool monitoring = true;
   
  Serial.println("\nFormat: [Time] Amplitude | Visual");
  Serial.println("----------------------------------");
   
  while (monitoring) {
    // Check for stop command
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == 's' || c == 'S') {
        monitoring = false;
        break;
      }
    }
     
    // Read sample
    if (i2s_read(I2S_NUM_0, (char*)&sample, sizeof(sample), &bytesRead, 10) == ESP_OK) {
      if (bytesRead == sizeof(sample)) {
        int16_t sample16bit = (int16_t)(sample >> 8);
        int amplitude = abs(sample16bit);
         
        // Display every 100ms
        if (millis() - lastDisplay > 100) {
          lastDisplay = millis();
           
          // Create visual representation
          String visual = "";
          int bars = map(amplitude, 0, 32767, 0, 20);
          for (int i = 0; i < bars; i++) {
            visual += "█";
          }
           
          Serial.printf("[%5d ms] %6d | %s\n", millis(), amplitude, visual.c_str());
        }
      }
    }
  }
   
  i2s_driver_uninstall(I2S_NUM_0);
  Serial.println("\nMonitoring stopped");
}
 
 
// Helper function to print pin diagram
void printPinDiagram() {
  Serial.println("\nINMP441 Pinout:");
  Serial.println("  ┌─────────────┐");
  Serial.println("  │ INMP441     │");
  Serial.println("  │             │");
  Serial.println("  │ 1: L/R (WS) │─── GPIO32");
  Serial.println("  │ 2: SD       │─── GPIO35");
  Serial.println("  │ 3: SCK      │─── GPIO33");
  Serial.println("  │ 4: VDD      │─── 3.3V");
  Serial.println("  │ 5: GND      │─── GND");
  Serial.println("  └─────────────┘");
}
