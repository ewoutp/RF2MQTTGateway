/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include "mqtt.h"
#include "wifi_conn_mgmt.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  setupWifi();
  setupMqtt();

  connectToWifi();
}

void loop() {
}