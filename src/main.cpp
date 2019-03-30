/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include "heartbeat.h"
#include "mqtt.h"
#include "rfm69_433.h"
#include "wifi_conn_mgmt.h"

static word lastHeartbeat = 0;
static char id[16];

#define REBOOT_INTERVAL_MS ((unsigned long)(1000*3600*24*2)) // Reboot every 2 days

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("Beginning ...");

  uint64_t chipid = ESP.getEfuseMac();
  sprintf(id, "%04X", (uint16_t)(chipid>>32)); //print High 2 bytes
  sprintf(id+4, "%08X", (uint32_t)chipid);//print Low 4bytes.

  setupHeartbeat();
  setupRFM69();

  setupWifi();
  Serial.println("setupWifi completed.");
  setupMqtt(id);
  Serial.println("setupMqtt completed.");

  connectToWifi();
  Serial.println("connectToWifi completed.");
}

void loop() {
  loopHeartbeat(id);
  loopRFM69(id);

  if (millis() > REBOOT_INTERVAL_MS) {
    ESP.restart();
  }
}