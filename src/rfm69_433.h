/*
RFM69 433Mhz transceiver code.
*/

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

void setupRFM69();
void loopRFM69(const char *id);

bool parseMessage(const String& protocol, const JsonObject &source, byte* msg, int &msgLen, int maxMsgLen);
void sendMessage(const String& protocol, const byte* msg, int msgLen);
