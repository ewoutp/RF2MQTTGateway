/*
RFM69 433Mhz transceiver code.
*/

#pragma once

void setupRFM69();
void loopRFM69();

void sendMessage(const String& protocol, const byte* msg, int msgLen);