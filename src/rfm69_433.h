/*
RFM69 433Mhz transceiver code.
*/

#pragma once

void setupRFM69();
void loopRFM69(const char *id);

void sendMessage(const String& protocol, const byte* msg, int msgLen);