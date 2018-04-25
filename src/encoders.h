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
    int bit = 0x80;
    int bitCount = 32;
    if (msgLen == 4) {
        bitCount = 32;
    } else if (msgLen = 5) {
        bitCount = 36;
    } else {
        return false;
    }
    //for (int repeat = 0; repeat < 2; repeat++) {
        startPulse(tx);
        for (int i = 0; i < bitCount; i++) {
        if ((*p & bit) == bit) {
            bit1(tx);
        } else {
            bit0(tx);
        }
        if (bit == 0x01) {
            bit = 0x80;
            p++;
        } else {
            bit >>= 1;
        }
        }
        stopPulse(tx);
    //}
    return true;
  }

private:
  const int T1 = 275;
  const int T4 = 1100;
  const int T8 = 2200;

  void startPulse(OOKTransmitter& tx) {
      // Now begin the real start pulse
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T8);
  }

  void bit1(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T4);
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T1);
  }

  void bit0(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T1);
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T4);
  }

  void stopPulse(OOKTransmitter& tx) {
      tx.on();
      delayMicroseconds(T1);
      tx.off();
      delayMicroseconds(T1);
  }
};

typedef struct {
    int typecode;
    const char* name;
    EncodeOOK* encoder;
} EncoderInfo;
