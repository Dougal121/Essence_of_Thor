#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>

// Optional future TCP stub
#include <WiFi.h>

class ModbusBackend
{
public:
    enum TransportType
    {
        RTU = 0,
        TCP = 1   // stub only for now
    };

    struct WriteRequest
    {
        uint8_t slave;
        uint16_t addr;
        bool state;
    };

    ModbusBackend();

    // --- init ---
    bool beginRTU(HardwareSerial& serial, int baud, int dePin);

    // --- future ---
    bool beginTCP(const char* host, uint16_t port); // stub

    // --- IO API ---
    void writeCoil(uint8_t slave, uint16_t addr, bool state);

    // --- service loop ---
    void loop();
    void flush();

private:

    // =========================
    // Queue system
    // =========================
    static const int QUEUE_SIZE = 64;

    WriteRequest queue[QUEUE_SIZE];
    volatile int head;
    volatile int tail;

    bool queuePush(const WriteRequest& req);
    bool queuePop(WriteRequest& req);

    // =========================
    // RTU (Walker Modbus)
    // =========================
    HardwareSerial* rs485Serial;
    int dePin;

    ModbusMaster node;

    static void preTransmission();
    static void postTransmission();

    static ModbusBackend* instance; // for static callbacks

    // =========================
    // Transport
    // =========================
    TransportType transport;

    // =========================
    // TCP STUB (future)
    // =========================
    const char* tcpHost;
    uint16_t tcpPort;

    bool sendTCP(const WriteRequest& req);   // stub
};