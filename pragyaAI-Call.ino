/*
 Pragya Mesh Voice Pin — FIXED FOR CROWPANEL 1.54
 ESP32-S3 CrowPanel Pinout:
 - Menu Button: GPIO 2  <-- We use this as the trigger
 - Exit Button: GPIO 1
 - Rotary Switch: Up (6), Down (4), OK (5)
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ---- WIFI ----
const char* WIFI_SSID     = "JF2GS44";
const char* WIFI_PASSWORD = "9653434203";

// ---- GPIO (CrowPanel physical Menu Button is on GPIO 2) ----
#define BUTTON_PIN  2 

// ---- Pragya Mesh ----
String NODE_ID = "voice-pin-001";
String SUBMIT_URL = "http://tawateideas.com/pragya/pragya_mesh2/mesh_submittask.php";
String POLL_URL   = "http://www.tawateideas.com/pragya/pragya_mesh2/mesh_getresults_single.php";

// ===== UTIL =====
void connectWiFi() {
  Serial.print("WiFi: ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println(" connected");
}

// ====== SUBMIT TASK ======
String submitTask(const String &prompt) {
  HTTPClient client;
  client.begin(SUBMIT_URL);
  client.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;
  //doc["node_id"] = NODE_ID;
  doc["model"] = "Model1";
  doc["prompt"]  = prompt;

  String body;
  serializeJson(doc, body);

  int code = client.POST(body);

  Serial.print("return code: ");
  Serial.print(code);
  
  if (code <= 0) {
    Serial.println("❌ POST failed");
    return "";
  }

  String response = client.getString();
  client.end();

  StaticJsonDocument<256> res;
  deserializeJson(res, response);

  if (!res.containsKey("task_id")) return "";

  return res["task_id"].as<String>();
}

// ====== POLL RESULT ======
String fetchResult(const String &taskId) {
  if (taskId.length() == 0) return "";

  while (true) {
    HTTPClient client;
    client.begin(POLL_URL + "?task_id=" + taskId);
    int code = client.GET();

    if (code > 0) {
      String raw = client.getString();

      StaticJsonDocument<1024> doc;
      deserializeJson(doc, raw);

      if (doc.containsKey("status")) {
        String s = doc["status"].as<String>();

        if (s == "success") {
          client.end();
          return doc["result"].as<String>();
        }
      }
    }

    client.end();
    Serial.println("⏳ Waiting...");
    delay(1200);
  }
}

// ====== AUDIO MOCK ======
String recordSpeech() {
  Serial.println("🎙️ [fake STT] Recording...");
  delay(1500);

  // Raw health data
  String heartRate = R"([
  {"time":"06:00", "hr":48, "bp":"108/64"},
  {"time":"06:15", "hr":50, "bp":"110/66"},
  {"time":"06:30", "hr":49, "bp":"109/65"},
  {"time":"07:00", "hr":90, "bp":"120/70"},
  {"time":"07:10", "hr":120, "bp":"128/72"},
  {"time":"07:20", "hr":158, "bp":"140/76"},
  {"time":"07:30", "hr":115, "bp":"130/72"},
  {"time":"07:40", "hr":165, "bp":"142/78"},
  {"time":"07:50", "hr":105, "bp":"124/70"},
  {"time":"08:15", "hr":65, "bp":"118/68"},
  {"time":"08:30", "hr":58, "bp":"112/66"},
  {"time":"09:00", "hr":52, "bp":"108/64"}
  ])";

  // ENGINEERING THE PROMPT:
  // 1. We ask for "Raw JSON" to avoid conversation.
  // 2. We explicitly say "No Markdown" to prevent the response from including ```json
  // 3. We define the specific keys required.
  String prompt = "Analyze this health data. "
                  "Return ONLY a raw JSON object. "
                  "Do not use Markdown formatting or code blocks. "
                  "Do not include any intro text. "
                  "Required JSON format: {\"Criticality\": \"number/100\", \"Diet\": \"Good/Bad\", \"Improvement\": \"advice under 5 words\"}. "
                  "Data: " + heartRate;

  return prompt;
}

void speakText(const String &txt) {
  Serial.println("🔊 [fake TTS] Speaking: " + txt);
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  
  // CrowPanel buttons are active LOW (connected to GND when pressed)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  connectWiFi();
  Serial.println("✨ CrowPanel Ready - press MENU button on the back");
}

// ====== MAIN LOOP ======
void loop() {
  // Check if button is pressed (LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Basic debounce
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("\n🎤 Listening...");

      String speech = recordSpeech();
      if (speech.length() < 2) {
        Serial.println("❌ No speech captured");
        return;
      }

      Serial.println("👉 User said: " + speech);

      String taskId = submitTask(speech);
      Serial.println("📮 Submitted: " + taskId);

      String reply = fetchResult(taskId);
      Serial.println("🤖 Mesh replied: " + reply);

      speakText(reply);

      Serial.println("✨ Ready for next\n");
      
      // Wait for button release
      while(digitalRead(BUTTON_PIN) == LOW) delay(10);
    }
  }

  delay(50);
}
