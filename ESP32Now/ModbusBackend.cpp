#include "ModbusBackend.h"

// ======================================================
// static instance pointer (for RS485 callbacks)
// ======================================================
ModbusBackend* ModbusBackend::instance = nullptr;

// ======================================================
// Constructor
// ======================================================
ModbusBackend::ModbusBackend()
{
    head = 0;
    tail = 0;

    rs485Serial = nullptr;
    dePin = -1;

    transport = RTU;

    tcpHost = nullptr;
    tcpPort = 502;

    instance = this;
}

// ======================================================
// RTU init (Walker ModbusMaster)
// ======================================================
bool ModbusBackend::beginRTU(HardwareSerial& serial, int baud, int dePin_)
{
    rs485Serial = &serial;
    dePin = dePin_;

    transport = RTU;

    pinMode(dePin, OUTPUT);
    digitalWrite(dePin, LOW);

    rs485Serial->begin(baud, SERIAL_8N1);

    // ModbusMaster uses slave "node"
    node.begin(1, *rs485Serial); // default slave, overridden per call

    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

    return true;
}

// ======================================================
// TCP stub init
// ======================================================
bool ModbusBackend::beginTCP(const char* host, uint16_t port)
{
    tcpHost = host;
    tcpPort = port;

    transport = TCP;

    // STUB ONLY
    // Future:
    // - WiFiClient socket
    // - Modbus TCP framing layer

    return true;
}

// ======================================================
// Public API
// ======================================================
void ModbusBackend::writeCoil(uint8_t slave, uint16_t addr, bool state)
{
    WriteRequest req;
    req.slave = slave;
    req.addr = addr;
    req.state = state;

    queuePush(req);
}

// ======================================================
// Main loop (non-blocking IO pump)
// ======================================================
void ModbusBackend::loop()
{
    WriteRequest req;

    int budget = 4;

    while (budget-- && queuePop(req))
    {
        if (transport == RTU)
        {
            // RTU execution
            node.begin(req.slave, *rs485Serial);

            uint8_t result = node.writeSingleCoil(req.addr, req.state);

            // optional diagnostics hook
            // if (result != node.ku8MBSuccess) log error
        }
        else
        {
            sendTCP(req); // stub
        }
    }
}

// ======================================================
// Flush queue
// ======================================================
void ModbusBackend::flush()
{
    WriteRequest req;

    while (queuePop(req))
    {
        if (transport == RTU)
        {
            node.begin(req.slave, *rs485Serial);
            node.writeSingleCoil(req.addr, req.state);
        }
        else
        {
            sendTCP(req);
        }
    }
}

// ======================================================
// TCP stub (future implementation)
// ======================================================
bool ModbusBackend::sendTCP(const WriteRequest& req)
{
    // TODO:
    // - WiFiClient connection
    // - MBAP header construction OR ModbusTCP library
    // - request framing

    (void)req;
    return false;
}

// ======================================================
// RS485 direction control (critical for stability)
// ======================================================
void ModbusBackend::preTransmission()
{
    if (instance && instance->dePin >= 0)
        digitalWrite(instance->dePin, HIGH);
}

void ModbusBackend::postTransmission()
{
    if (instance && instance->dePin >= 0)
        digitalWrite(instance->dePin, LOW);
}

// ======================================================
// Queue push
// ======================================================
bool ModbusBackend::queuePush(const WriteRequest& req)
{
    int next = (head + 1) % QUEUE_SIZE;

    if (next == tail)
        return false; // full

    queue[head] = req;
    head = next;

    return true;
}

// ======================================================
// Queue pop
// ======================================================
bool ModbusBackend::queuePop(WriteRequest& req)
{
    if (tail == head)
        return false;

    req = queue[tail];
    tail = (tail + 1) % QUEUE_SIZE;

    return true;
}