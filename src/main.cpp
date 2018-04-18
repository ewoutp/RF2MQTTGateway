/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/

#include "mqtt.h"
#include "rfm69_433.h"
#include "wifi_conn_mgmt.h"

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println("Beginning ...");

  setupRFM69();

  setupWifi();
  Serial.println("setupWifi completed.");
  setupMqtt();
  Serial.println("setupMqtt completed.");

  connectToWifi();
  Serial.println("connectToWifi completed.");
}

void loop() {
  loopRFM69();
}