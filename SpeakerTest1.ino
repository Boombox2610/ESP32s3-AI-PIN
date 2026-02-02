#include <driver/i2s.h>
#include <math.h>
 
// --- Pin Definitions ---
#define I2S_BCK 6
#define I2S_LRC 5
#define I2S_DOUT 7
 
// --- Configuration ---
#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0
#define WAVE_FREQ_HZ 440  // 440Hz = Musical Note 'A'
#define VOLUME 0.5        // 0.0 to 1.0 (Careful, 1.0 is VERY LOUD)
 
void setup() {
  Serial.begin(115200);
  Serial.println("Jai Prabhu");
  delay(500);
   
  // Configure I2S Driver
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo output
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };
   
  // Configure I2S Pins
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE // Not using input here
  };
 
  // Install and Start
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
   
  Serial.println("Speaker Test Started: Playing 440Hz Sine Wave");
}
 
void loop() {
  // Generate a sine wave buffer
  int16_t samples[128]; // Buffer for audio samples
  static float phase = 0;
   
  // Fill buffer with sine wave data
  for (int i = 0; i < 128; i += 2) {
    float output = sin(phase) * 32767 * VOLUME;
    phase += 2 * PI * WAVE_FREQ_HZ / SAMPLE_RATE;
     
    // Prevent phase overflow
    if (phase >= 2 * PI) phase -= 2 * PI;
 
    // Write same data to Left and Right channels (Stereo)
    samples[i] = (int16_t)output;     // Left
    samples[i+1] = (int16_t)output;   // Right
  }
 
  // Write buffer to I2S
  size_t bytes_written;
  i2s_write(I2S_NUM, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
}
