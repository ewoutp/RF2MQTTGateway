#pragma once

class OOKTransmitter {
public:
  virtual void on() = 0;
  virtual void off() = 0;
};


/// This is the general base class for implementing OOK encoders.
class EncodeOOK {
public:
  // Encode the given message to the given transmitter.
  // Returns true on success, false otherwise.
  virtual bool encode(const byte *msg, int msgLen, OOKTransmitter& tx) = 0;
};

// 433 MHz encoders

/// OOK encoder for Klik-Aan-Klik-Uit type A devices.
class KakuAEncoder : public EncodeOOK {
public:
  virtual bool encode(const byte *msg, int msgLen, OOKTransmitter& tx) {
    const byte* p = msg;
    int bit = 0x01;
    int bitCount = 32;
    if (msgLen == 4) {
        bitCount = 32;
    } else if (msgLen = 5) {
        bitCount = 36;
    } else {
        return false;
    }
    startPulse(tx);
    for (int i = 0; i < bitCount; i++) {
      if ((*p & bit) == bit) {
          bit1(tx);
      } else {
          bit0(tx);
      }
      if (bit == 0x80) {
          bit = 0x01;
          p++;
      } else {
          bit <<= 1;
      }
    }
    stopPulse(tx);
    return true;
  }

private:
  const int T = 275;

  void startPulse(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T*9);
  }

  void bit1(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T*3);
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T);
  }

  void bit0(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T);
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T*3);
  }

  void stopPulse(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T);
      tx.off();
      delayMicroseconds(T*40);
  }
};

typedef struct {
    int typecode;
    const char* name;
    EncodeOOK* encoder;
} EncoderInfo;
