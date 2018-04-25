/*
RFM69 433Mhz transceiver code.
*/

#include <RFM69OOK.h>
#include <SPI.h>
#include <RFM69OOKregisters.h>
#include <ArduinoJson.h>

#include "decoders.h"
#include "encoders.h"
#include "protocols.h"
#include "simple_fifo.h"
#include "secrets.h"
#include "mqtt.h"

//#define DEBUG
#define DEBUG_LED   13 // RED LED
#define MAXDATA 66

#define RF69_FREQ     433.92

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

KakuAEncoder kakuAEncoder;

EncoderInfo ei_433[] = {
    //{ 5, "ORSC", &orsc },
    //{ 6, "CRES", &cres },
    //{ 7, "KAKU", &kaku },
    //{ 8, "XRF", &xrf },
    //{ 9, "HEZ", &hez },
    //{ 10, "ELRO", &elro },
    //{ 11, "FMGO", &fmgo },
    //{ 12, "SMK", &smk },
    //{ 13, "BYR", &byr },
    { 14, "KAKUA", &kakuAEncoder},
    { -1, 0, 0 }
};

KakuAProtocol kakuAProtocol;

ProtocolInfo pi[] = {
  { 14, "KAKUA", &kakuAProtocol },
  { -1, 0, 0 }
};

static portMUX_TYPE signalMux = portMUX_INITIALIZER_UNLOCKED;
typedef unsigned long microsUnit;
typedef SimpleFIFO<microsUnit, 255> pulseFIFO;
pulseFIFO fifo_433;

static void IRAM_ATTR signal433ChangedInt() {
  static volatile microsUnit last_433 = 0; // never accessed outside ISR's

    microsUnit now = micros();
    portENTER_CRITICAL_ISR(&signalMux);
    microsUnit w = now - last_433;
    if (w > 100) {
      last_433 = now;
      fifo_433.enqueue(w);
    }
    portEXIT_CRITICAL_ISR(&signalMux);
}

static void processDecodedData(DecoderInfo& di, const char *id) {
  static StaticJsonBuffer<512> jsonBuffer;

  // Fetch data
  byte size;
  const byte* data = di.decoder->getData(size);

  // Prepare MQTT message payload
  jsonBuffer.clear();
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["type"] = "receive";
  root["uptime"] = millis() / 1000;
  root["protocol"] = di.name;
  root["sender"] = id;
  JsonArray& dataArr = root.createNestedArray("data");
  for (int i = 0; i < size; i++) {
    dataArr.add(data[i]);
  }

  // Try to decode the protocol
  ProtocolInfo* ppi = pi;
  while (ppi->protocol) {
    if (ppi->typecode == di.typecode) {
      ppi->protocol->Decode(data, size, root);
      break;
    }
  }

  // Send MQTT message
  String payload;
  root.printTo(payload);
  publishMqttMessage(MQTT_RECEIVE_TOPIC, 2, true, payload.c_str());
  Serial.println(payload);

  // Reset decoder
  di.decoder->resetDecoder();
}

// Check for a new pulse and run the corresponding decoders for it
static void runPulseDecoders (DecoderInfo* pdi, pulseFIFO& fifo, const char *id) {
  // get next pulse with and reset it - need to protect against interrupts
  microsUnit p = 0;
  portENTER_CRITICAL_ISR(&signalMux);
  if (fifo.count() > 0) {
    p = fifo.dequeue();
  }
  portEXIT_CRITICAL_ISR(&signalMux);

    // if we had a pulse, go through each of the decoders
    if (p != 0) { 
      //Serial.print(p); Serial.println();
#ifdef DEBUG_LED
        digitalWrite(DEBUG_LED, 1);
#endif
        while (pdi->decoder != 0) {
            if (pdi->decoder->nextPulse(p)) {
              processDecodedData(*pdi, id);
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
#ifdef DEBUG_LED
  pinMode(DEBUG_LED, OUTPUT);
#endif

  // Reset RFM69

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

#ifdef DEBUG
  Serial.println("Initializing radio 433");
#endif
  
  pinMode(RFM69_DIO2, INPUT);

  radio433.initialize();
//  radio433.setBandwidth(OOK_BW_10_4);
//  radio433.setRSSIThreshold(-70);
//  radio433.setFixedThreshold(30);

  radio433.setRSSIThreshold(-50); // -50
  radio433.setFixedThreshold(45); // 45
  radio433.setSensitivityBoost(SENSITIVITY_BOOST_HIGH);
  radio433.setFrequencyMHz(433.9);
  //radio433.setFrequencyMHz(868.35);
  //radio433.setHighPower();
  //radio433.setMode(RF69OOK_MODE_RX);
  radio433.attachUserInterrupt(signal433ChangedInt);
  radio433.receiveBegin();

#ifdef DEBUG
  Serial.println(F("start 433"));
  radio433.readAllRegs();
  Serial.println("Setup complete");
#endif
}

void loopRFM69(const char *id) {
   runPulseDecoders(di_433, fifo_433, id); 
}

class rfm69OOKTransmitter : public OOKTransmitter {
public:
  virtual void on() {
    radio433.send(true);
  }

  virtual void off() {
    radio433.send(false);
  }
};

static rfm69OOKTransmitter transmitter;

void sendMessage(const String& protocol, const byte* msg, int msgLen) {
  EncoderInfo* pei = ei_433;
  while (pei->encoder) {
    if (protocol == pei->name) {
      // Turn off radio
      radio433.receiveEnd();

      Serial.print("Sending "); Serial.print(pei->name); Serial.println(" message");
      Serial.print("len: "); Serial.println(msgLen);

      radio433.transmitBegin();
      cli();
      transmitter.off();
      delay(5); // Give the "air" some time to get stable

      pei->encoder->encode(msg, msgLen, transmitter);

      transmitter.off();
      delay(5); // Give the "air" some time to get stable
      sei();
      radio433.transmitEnd();

      radio433.receiveBegin();
      break;
    }
    pei++;
  }
}

bool parseMessage(const String& protocol, const JsonObject &source, byte* msg, int &msgLen, int maxMsgLen) {
  ProtocolInfo* ppi = pi;
  while (ppi->protocol) {
    if (protocol == ppi->name) {
      if (ppi->protocol->Encode(source, msg,msgLen, maxMsgLen)) {
        return true;
      }
      const JsonArray& data = source["data"];
      if ((data.size() > 0) && (data.size() <= maxMsgLen)) {
        for (int i = 0; i < data.size(); i++) {
          msg[i] = data.get<byte>(i);
        }
        msgLen = data.size();
        return true;
      }
      return false;
    }
  }
  return false;
}
