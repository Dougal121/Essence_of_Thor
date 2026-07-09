#ifndef HC595_16_H
#define HC595_16_H
#pragma once
#include <Arduino.h>
#include <cstdint>

#if defined(ESP32)
#define MaxPinPort  40
#elif defined(ESP8266)
#define MaxPinPort  18
#endif  

class HC595_16
{
private:
    uint16_t state = 0;

    int pinData;
    int pinClock;
    int pinLatch;
    int pinOE;
    
public:
    HC595_16(int data = 14, int clk = 13, int latch = 12, int oe = 5);

    void begin(int data = 14, int clk = 13, int latch = 12, int oe = 5);
//    void begin();
    void setPins(int data = 14, int clk = 13, int latch = 12, int oe = 5);

    void setBit(uint8_t bit, bool value);

    void update();

    void clearOutputs();
};

#endif