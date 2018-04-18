/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include <ArduinoJson.h>
#include "mqtt.h"
#include "secrets.h"

static word lastHeartbeat = 0;

#define HEARTBEAT_INTERVAL (1000*60*2) // 2min

static void sendHeartbeat(const char *id) {
  static StaticJsonBuffer<512> jsonBuffer;

  // Prepare MQTT message payload
  jsonBuffer.clear();
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["type"] = "heartbeat";
  root["sender"] = id;
  root["uptime"] = millis() / 1000;

  // Send MQTT message
  String payload;
  root.printTo(payload);

  publishMqttMessage(MQTT_RECEIVE_TOPIC, 0, false, payload.c_str());
}

void setupHeartbeat() {
}

void loopHeartbeat(const char *id) {
  static word now;
  now = millis();
  if ((lastHeartbeat == 0) || ((now - lastHeartbeat) > HEARTBEAT_INTERVAL)) {
    if (isMqttConnected()) {
      sendHeartbeat(id);
      lastHeartbeat = now;
    }
  }
}
