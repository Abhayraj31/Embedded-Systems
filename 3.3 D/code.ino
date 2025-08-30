#include <WiFiNINA.h>
#include <PubSubClient.h>

// ---- Wi-Fi & MQTT ----
const char* WIFI_SSID    = "a";             // <--- your WiFi SSID
const char* WIFI_PASS    = "qwertyui";      // <--- your WiFi password
const char* STUDENT_NAME = "Abhayraj";      // <--- your name

const char* MQTT_SERVER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;

const char* TOPIC_WAVE = "SIT210/wave";
const char* TOPIC_PAT  = "SIT210/pat";

// ---- Pins ----
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int LED_PIN  = 6;

// ---- Distance thresholds ----
const int WAVE_CM = 25;   // Wave detected if <= 25 cm
const int PAT_CM  = 10;   // Pat detected if <= 10 cm

WiFiClient wifi;
PubSubClient mqtt(wifi);

unsigned long lastPublish = 0;
const unsigned long cooldown = 1500; // ms between publishes

// ---- Measure distance in cm using HC-SR04 ----
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout ~30ms
  if (duration == 0) return 9999; // no echo
  return duration / 58.0;         // convert to cm
}

// ---- LED patterns ----
void blinkWave() { 
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(150);
    digitalWrite(LED_PIN, LOW);  delay(150);
  }
}

void blinkPat() { 
  digitalWrite(LED_PIN, HIGH); delay(400);
  digitalWrite(LED_PIN, LOW);  delay(150);
  digitalWrite(LED_PIN, HIGH); delay(150);
  digitalWrite(LED_PIN, LOW);  delay(150);
  digitalWrite(LED_PIN, HIGH); delay(400);
  digitalWrite(LED_PIN, LOW);  delay(150);
  digitalWrite(LED_PIN, HIGH); delay(150);
  digitalWrite(LED_PIN, LOW);  
}

// ---- MQTT callback ----
void handleMessage(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  if (t == TOPIC_WAVE) blinkWave();
  if (t == TOPIC_PAT)  blinkPat();
}

// ---- Setup ----
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);

  // Connect Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Setup MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(handleMessage);

  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect("nano33iot_client")) {
      Serial.println("connected");
      mqtt.subscribe(TOPIC_WAVE);
      mqtt.subscribe(TOPIC_PAT);
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqtt.state());
      delay(1000);
    }
  }
}

// ---- Loop ----
void loop() {
  mqtt.loop(); 

  float cm = getDistanceCM();
  unsigned long now = millis();

  if (now - lastPublish > cooldown) {
    if (cm <= PAT_CM) {
      mqtt.publish(TOPIC_PAT, STUDENT_NAME);
      Serial.println("Published PAT");
      lastPublish = now;
    }
    else if (cm <= WAVE_CM) {
      mqtt.publish(TOPIC_WAVE, STUDENT_NAME);
      Serial.println("Published WAVE");
      lastPublish = now;
    }
  }
}
