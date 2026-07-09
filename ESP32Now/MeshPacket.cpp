#include "MeshPacket.h"

/**
 * CRC-16 (Modbus style)
 * Polynomial: 0xA001
 *
 * IMPORTANT:
 * CRC is calculated over the packet EXCLUDING the crc field itself.
 */
uint16_t mesh_packet_t::calcCRC(const mesh_packet_t& pkt)
{
    uint16_t crc = 0xFFFF;
    // We explicitly exclude the CRC field at the end
    const uint8_t* data = reinterpret_cast<const uint8_t*>(&pkt);
    constexpr size_t crcOffset = sizeof(mesh_packet_t) - sizeof(pkt.crc);
    for (size_t i = 0; i < crcOffset; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/**
 * Prepares packet for transmission
 */
void mesh_packet_t::prepareForSend(mesh_packet_t& pkt)
{
    pkt.crc = 0;                 // IMPORTANT: zero before calculation
    pkt.crc = calcCRC(pkt);
}

/**
 * Validates received packet integrity
 */
bool mesh_packet_t::validate(const mesh_packet_t& pkt)
{
    mesh_packet_t temp = pkt;
    uint16_t receivedCRC = temp.crc;

    temp.crc = 0;
    uint16_t calc = calcCRC(temp);

    return (calc == receivedCRC);
}