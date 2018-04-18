/*
RFM69 433Mhz transceiver code.
*/

#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>
#include "decoders.h"
#include "simple_fifo.h"
#include "secrets.h"
#include "mqtt.h"

#define DEBUG
#define MAXDATA 66

#define RF69_FREQ     433.9

#define RFM69_DIO2    27 // "A"
#define RFM69_CS      32 // "D"
#define RFM69_RST     15 // "C"
#define RFM69_IRQ     14 // "E"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ)

RFM69OOK radio433(RFM69_CS, RFM69_DIO2, true, digitalPinToInterrupt(RFM69_DIO2));

OregonDecoder orsc;
CrestaDecoder cres;
KakuDecoder kaku;
KakuADecoder kakuA; //WvD
XrfDecoder xrf;
HezDecoder hez;
FlamingoDecoder fmgo;
SmokeDecoder smk;
ByronbellDecoder byr;
ElroDecoder elro;

DecoderInfo di_433[] = {
    //{ 5, "ORSC", &orsc },
    //{ 6, "CRES", &cres },
    //{ 7, "KAKU", &kaku },
    //{ 8, "XRF", &xrf },
    //{ 9, "HEZ", &hez },
    //{ 10, "ELRO", &elro },
    //{ 11, "FMGO", &fmgo },
    //{ 12, "SMK", &smk },
    //{ 13, "BYR", &byr },
    { 14, "KAKUA", &kakuA },
    { -1, 0, 0 }
};

typedef SimpleFIFO<word, 8> pulseFIFO;
pulseFIFO fifo_433;
word last_433; // never accessed outside ISR's

static void signal433ChangedInt() {
  word now = micros();
  word w = now - last_433;
  last_433 = now;
  fifo_433.enqueue(w);
}

// Outgoing data buffer for RF12
//byte packetBuffer [MAXDATA], packetFill;

static void processDecodedData(DecoderInfo& di) {
  // Fetch data
  byte size;
  const byte* data = di.decoder->getData(size);

  // Prepare MQTT message payload
  String s("{\"protocol\":\"");
  s += di.name;
  s += "\",\"data\":[";
  for (int i = 0; i < size; i++) {
    if (i > 0) {
      s.concat(',');
    }
    s.concat(data[i]);
  }
  s += "]}";

  // Send MQTT message
  publishMqttMessage(MQTT_TOPIC, 2, true, s.c_str());

  // Reset decoder
  di.decoder->resetDecoder();
}

// Check for a new pulse and run the corresponding decoders for it
static void runPulseDecoders (DecoderInfo* pdi, pulseFIFO& fifo) {
    // get next pulse with and reset it - need to protect against interrupts
    cli();
    word p = 0;
    if (fifo.count() > 0) {
      p = fifo.dequeue();
    }
    sei();

    // if we had a pulse, go through each of the decoders
    if (p != 0) { 
      //Serial.print(p); Serial.println();
#ifdef DEBUG_LED
        digitalWrite(DEBUG_LED, 1);
#endif
        while (pdi->decoder != 0) {
            if (pdi->decoder->nextPulse(p)) {
              processDecodedData(*pdi);
            }
            ++pdi;
        }
#ifdef DEBUG_LED
        digitalWrite(DEBUG_LED, 0);
#endif
    }
}

void setupRFM69() {
  // Configure pins
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  pinMode(RFM69_DIO2, INPUT);
  pinMode(RFM69_CS, OUTPUT);

  // Reset RFM69

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  Serial.println("Initializing radio 433");
  
  pinMode(RFM69_DIO2, INPUT);

  radio433.initialize();
//  radio433.setBandwidth(OOK_BW_10_4);
  radio433.setRSSIThreshold(-70);
  radio433.setFixedThreshold(30);
  //radio433.setSensitivityBoost(SENSITIVITY_BOOST_HIGH);
  radio433.setFrequencyMHz(433.9);
  //radio433.setFrequencyMHz(868.35);
  //radio433.setHighPower();
  //radio433.setMode(RF69OOK_MODE_RX);
  radio433.attachUserInterrupt(signal433ChangedInt);
  radio433.receiveBegin();

  Serial.println(F("start 433"));
  radio433.readAllRegs();

  Serial.println("Setup complete");
}

void loopRFM69() {
   runPulseDecoders(di_433, fifo_433); 
}
