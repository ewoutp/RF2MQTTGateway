/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include <ArduinoJson.h>

#include "mqtt.h"
#include "secrets.h"
#include "rfm69_433.h"

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

bool isMqttConnected() {
  return mqttClient.connected();
}

static void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe(MQTT_TRANSMIT_TOPIC, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

static void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

static void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);

  static StaticJsonBuffer<512> jsonBuffer;
  jsonBuffer.clear();
  JsonObject &root = jsonBuffer.parseObject(payload, len);
  const String& protocol = root["protocol"];
  int repeat = root["repeat"] | 2;

  byte msgBuf[64];
  int msgLen;

  if (parseMessage(protocol, root, msgBuf, msgLen, sizeof(msgBuf))) {
    sendMessage(protocol, msgBuf, msgLen);
    if (repeat > 1) {
      sendMessage(protocol, msgBuf, msgLen);
    }
  } else {
    Serial.println("Unsupported message");
  }
}

static void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setupMqtt(const char *id) {
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

  mqttClient.setClientId(id);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void stopMqttReconnect() {
  xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void publishMqttMessage(const char *topic, uint8_t qos, bool retain, const char *payload) {
  mqttClient.publish(topic, qos, retain, payload) ;
}
