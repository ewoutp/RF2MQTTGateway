#include <ArduinoJson.h>
#include "mqtt.h"
#include "secrets.h"
#include "rfm69_433.h"

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

static word lastHeartbeat = 0;
static const char * project_version = STRINGIZE_VALUE_OF(PROJECT_VERSION);
static const char * project_build = STRINGIZE_VALUE_OF(PROJECT_BUILD);

#define HEARTBEAT_INTERVAL (1000*60*2) // 2min

static void sendHeartbeat(const char *id) {
  static StaticJsonDocument<1024> doc;
  JsonObject root = doc.to<JsonObject>();

  // Set the values
  root["type"] = "heartbeat";
  root["sender"] = id;
  root["uptime"] = millis() / 1000;
  root["version"] = project_version;
  root["build"] = project_build;

  // Send MQTT message
  String payload;
  serializeJson(root, payload);

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
