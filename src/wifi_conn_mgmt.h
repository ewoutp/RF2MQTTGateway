#pragma once

#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// connectToWifi begins connecting to WIFI.
void connectToWifi();

// setupWifiManager creates a WifiManager and configures Wifi from
// eeprom, or creates a portal to allow a user to configure it.
void setupWifi();
