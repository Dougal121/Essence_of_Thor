#pragma once

#include <Arduino.h>
#include <stdint.h>

enum MeshPacketType : uint8_t
{
    MESH_HEARTBEAT     = 1,
    MESH_TIME_SYNC     = 2,
    MESH_DATA          = 3,
    MESH_ACK           = 4,
    MESH_DISCOVER      = 5,
    MESH_DISCOVER_ACK  = 6,
    WAKEUP_AP          = 7
};

#pragma pack(push, 1)

/**
 * Core mesh packet used across ESP-NOW / RF transport.
 * Must remain byte-stable across all nodes.
 */
struct mesh_packet_t
{
    uint8_t  origin;        // Node ID of sender
    uint32_t msg_id;        // Unique message ID 16 bit node id and 16 bits message number...
    uint8_t  type;          // Packet type (DISCOVER, ACK, etc.)
    uint8_t  ttl;           // Hop limit
    uint8_t  rssi ;         // last rssi on recive of that node  
    uint32_t mc;           // timestamp / monotonic counter
    uint8_t  payload_len;   // actual payload size (<= sizeof(payload))
    uint8_t  payload[220];   // fixed payload buffer
    uint16_t crc;           // CRC16 of packet (excluding this field)

    // --- Helpers ---
    static uint16_t calcCRC(const mesh_packet_t& pkt);
    static void prepareForSend(mesh_packet_t& pkt);
    static bool validate(const mesh_packet_t& pkt);
};

#pragma pack(pop)