#pragma once

#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

void connectToMqtt();

void setupMqtt();

void stopMqttReconnect();

void publishMqttMessage(const char *topic, uint8_t qos, bool retain, const char *payload);
