/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include "mqtt.h"
#include "wifi_conn_mgmt.h"

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("Beginning ...");

  setupWifi();
  Serial.println("setupWifi completed.");
  setupMqtt();
  Serial.println("setupMqtt completed.");

  connectToWifi();
  Serial.println("connectToWifi completed.");
}

void loop() {
}