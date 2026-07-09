#include <Arduino.h>
#include "HC595_16.h"

HC595_16::HC595_16(int data , int clk, int latch , int oe )
{
    pinData  = data;
    pinClock = clk;
    pinLatch = latch;
    pinOE    = oe;
}

void HC595_16::setPins(int data , int clk, int latch , int oe )
{
    begin( data  , clk , latch  , oe );
}

void HC595_16::begin(int data  , int clk , int latch  , int oe )
{
    pinData  = data;
    pinClock = clk;
    pinLatch = latch;
    pinOE    = oe;

    if ((data  >= 0) && (data < MaxPinPort ))
        pinData  = data;
    if ((clk   >= 0) && (clk < MaxPinPort ))
        pinClock = clk;
    if ((latch >= 0) && (latch < MaxPinPort ))
        pinLatch = latch;
    if ((oe    >= 0) && (oe < MaxPinPort ))
        pinOE    = oe;

    pinMode(pinData, OUTPUT);
    pinMode(pinClock, OUTPUT);
    pinMode(pinLatch, OUTPUT);
    pinMode(pinOE, OUTPUT);
    digitalWrite(pinData, LOW);   // 
    digitalWrite(pinClock, LOW);   //
    digitalWrite(pinLatch, LOW);   // 
    digitalWrite(pinOE, LOW);   // Enable outputs (active LOW on most 74HC595 setups)
    state = 0;
    update();
}

void HC595_16::setBit(uint8_t bit, bool value)
{
    if (bit > 15) return; // safety guard for 16-bit register

    if (value)
        state |= (1UL << bit);
    else
        state &= ~(1UL << bit);
}

void HC595_16::update()
{
    digitalWrite(pinOE, LOW);   // Enable outputs (active LOW on most 74HC595 setups)    
    uint8_t iHB = (state >> 8) & 0xFF;
    uint8_t iLB = state & 0xFF;
    // shift out high byte first then low byte (MSB-first chain)
    shiftOut(pinData, pinClock, MSBFIRST, iHB);
    shiftOut(pinData, pinClock, MSBFIRST, iLB);
    digitalWrite(pinData,LOW);
    digitalWrite(pinLatch, HIGH); // now latch it in
    digitalWrite(pinLatch, LOW);
//    Serial.println("HC595Update " + String(state,HEX)+ " Data " +String(pinData)+ " Clk "+String(pinClock)+ " OE "+String(pinOE));
}

void HC595_16::clearOutputs()
{
    state = 0;
    update();
}