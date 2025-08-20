#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>

// ---- WiFi ----
#define WIFI_SSID      "a"
#define WIFI_PASSWORD  "qwertyui"

// ---- MQTT (HiveMQ Cloud) ----
#define MQTT_SERVER    "fefd78f005324fada645cddae5dd67ca.s1.eu.hivemq.cloud"   // from HiveMQ
#define MQTT_PORT      8883
#define MQTT_TOPIC     "light"
#define MQTT_USER      "hivemq.webclient.1755621657430"              // from HiveMQ credentials
#define MQTT_PASSWORD  "*&w3RE0HS$94FbKjazy;"              // from HiveMQ credentials

// ---- Sensor ----
BH1750 lightMeter;

// TLS client for HiveMQ Cloud
WiFiSSLClient wifiSSLClient;
PubSubClient mqttClient(wifiSSLClient);

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ArduinoNano33IoT", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Start BH1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 ready");
  } else {
    Serial.println("Error initializing BH1750!");
    while (1);
  }

  // Connect WiFi
  connectWiFi();

  // Set MQTT broker
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  // Format message and publish
  char msg[50];
  snprintf(msg, sizeof(msg), "Light: %.2f lx", lux);
  mqttClient.publish(MQTT_TOPIC, msg);

  delay(5000); // publish every 5 seconds
}


// [
//     {
//         "id": "email1",
//         "type": "e-mail",
//         "z": "44f0dcec5ab511b6",
//         "server": "smtp.gmail.com",
//         "port": "465",
//         "authtype": "BASIC",
//         "saslformat": false,
//         "token": "oauth2Response.access_token",
//         "secure": true,
//         "tls": true,
//         "name": "abhayrajsinghbhatia@gmail.com",
//         "dname": "Send Email",
//         "x": 750,
//         "y": 240,
//         "wires": []
//     },
//     {
//         "id": "c0ca3beaf8ee0f7b",
//         "type": "global-config",
//         "env": [],
//         "modules": {
//             "node-red-node-email": "3.1.0"
//         }
//     }
// ]