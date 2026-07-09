#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

class EspNowMesh;  // forward declare

class RFManager
{
public:

    enum State
    {
        BOOT,
        FAST_START,
        ACTIVE_SCAN,
        PASSIVE_SCAN,
        LOCKED,
        FALLBACK
    };

    struct Config
    {
        uint64_t chipid ; 
        uint8_t lNetworkOptions = 0 ;
        uint8_t defaultChannel = 6;
        uint32_t discoverTimeoutMs = 300;
        bool enableAP = false;
        const char* nssid = nullptr;
        const char* npassword = nullptr;
        char* cssid = nullptr;
        char* cpassword = nullptr;
        IPAddress IPStatic;
        IPAddress IPGateway; 
        IPAddress IPMask;
        IPAddress IPDNS;
    };
//    void begin(uint32_t nodeId, uint8_t myChannel);
    bool begin(uint32_t nodeId, const Config& cfg);
    void loop();

    void setAPEnabled(bool en, const char* ssid = nullptr, const char* pass = nullptr);
    void triggerAP(uint32_t durationMs);

    uint8_t getChannel() const { return _channel; }
    bool isLocked() const { return _state == LOCKED; }

    // discovery hook from ESP-NOW
    void onDiscoverReply(uint32_t gatewayId, uint8_t channel);
    void onReceive(const uint8_t* data, int len);
    void attachMesh(EspNowMesh* mesh);

private:

    void setChannel(uint8_t ch);
    int  getRouterChannel(const char* ssid , int iDefault);    
    
    void startESPNow();
    void stopESPNow();

    void sendDiscover(uint8_t  ttl );
    void sendDiscoverAck(const uint8_t* destMac,uint8_t  ttl );

    bool tryDiscoverOnChannel(uint8_t ch);
    bool scanActiveChannels();
    bool scanPassiveAP();
    uint32_t nextMsgId();
    void setLastMsgId(uint32_t id);
    void enterFallback();

    static void recvStatic(const esp_now_recv_info_t* info, const uint8_t* data, int len);

    void recv(const esp_now_recv_info_t* info, const uint8_t* data, int len);
//private:
   
    EspNowMesh* _mesh = nullptr;
    uint32_t _msgId = 1;
    static RFManager* _instance;

    State _state = BOOT;

    uint32_t _nodeId = 0;

    uint8_t _channel = 6;
    uint8_t _defaultChannel = 6;

    bool _discoverReply = false;
    uint32_t _gatewayId = 0;

    uint32_t _discoverStart = 0;

    bool _apEnabled = false;
    uint32_t _apTimeout = 0;

    char _ssid[32];
    char _pass[32];
};