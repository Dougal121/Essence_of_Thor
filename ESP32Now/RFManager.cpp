#include "RFManager.h"
#include "EspNowMesh.h"
#include "MeshPacket.h"

RFManager* RFManager::_instance = nullptr;

static const uint8_t ESP_BCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const byte MAX_WIFI_TRIES = 45 ;
#define ESP32_BUILTIN_LED 2

bool RFManager::begin(uint32_t nodeId, const Config& cfg)
{
    bool bRet = false ; 
    _instance = this;

    _nodeId = nodeId;
    _defaultChannel = cfg.defaultChannel;
    _channel = cfg.defaultChannel;

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    Serial.println("Scanning for access point...");
    WiFi.setSleep(false);  
    _channel = getRouterChannel(cfg.nssid, _defaultChannel);
    Serial.println("[Channel Lock] Using channel: "+String(_channel));
    WiFi.setTxPower(WIFI_POWER_19_5dBm);    // Set WiFi RF power output to highest level  // 
    
    if ( cfg.cssid[0] == 0 || cfg.cssid[1] == 0 ){   // pick a default setup ssid if none
        sprintf(cfg.cssid,"Sanidi_%08X\0",cfg.chipid);
        sprintf(cfg.cpassword,"\0");
    }
    IPAddress MyIPC = IPAddress (192, 168, 4 , 1);   //5 +(chipid & 0x7f )
    Serial.printf("Asking for Soft AP on address: %s\n", MyIPC.toString().c_str());
    delay(100);
    WiFi.softAPConfig(MyIPC,MyIPC,IPAddress (255, 255, 255 , 0));  
    Serial.println("Starting access point...");
    Serial.print("Host SSID: ");
    Serial.println(cfg.cssid);
    if (( cfg.cpassword == nullptr ) || ( cfg.cpassword[0] == 0 ) || ( cfg.cpassword[0] == 0xff)){
        bRet = WiFi.softAP((char*)cfg.cssid,nullptr,_channel);                                      // no passowrd
        Serial.println("No Password [OPEN] " + String(bRet) );
    }else{
        bRet = WiFi.softAP((char*)cfg.cssid,(char*) cfg.cpassword,_channel);
        Serial.print("Password: >");
        Serial.print(cfg.cpassword);
        Serial.println("< [WPA2] " + String(bRet));
    }
    MyIPC = WiFi.softAPIP();  // get back the address to verify what happened
    Serial.printf("Soft AP IP Started on address: %s\n", MyIPC.toString().c_str());
    Serial.println("WiFi Channel " + String(WiFi.channel()));
    
    if ( cfg.lNetworkOptions != 0 ) {
        WiFi.config(cfg.IPStatic,cfg.IPGateway,cfg.IPMask,cfg.IPDNS ); 
    }  
    Serial.print("Attaching to Client SSID: ");
    Serial.println(cfg.nssid);
    Serial.print("Password: >");
    Serial.print(cfg.npassword);
    Serial.println("< ");

    if (( cfg.npassword == nullptr) || (cfg.npassword[0]==0)){                            // connect to unencrypted access point      
        WiFi.begin((char*)cfg.nssid,nullptr,_channel);     // connect to access point with encryption
    }else{
        WiFi.begin((char*)cfg.nssid, (char*)cfg.npassword,_channel);     // connect to access point with encryption
    }
    int j = 0 ;
    while (( WiFi.status() != WL_CONNECTED ) && ( j < MAX_WIFI_TRIES )) {
        j = j + 1 ;
        delay(500);
        Serial.print(".");
        digitalWrite(ESP32_BUILTIN_LED,!digitalRead(ESP32_BUILTIN_LED));
    } 
    if ( j >= MAX_WIFI_TRIES ) {
        WiFi.disconnect();
        Serial.println("");
        Serial.println("WiFi failed to connect");  
        _state = FAST_START;
    }else{
        Serial.println("");
        Serial.println("WiFi connected");  
        IPAddress MyIP =  WiFi.localIP() ;
        Serial.printf("IP address:  %s\n", MyIP.toString().c_str());   
        _state = LOCKED;
    }

    startESPNow();
    sendDiscover(3);

    bRet = true;
    return(bRet); 
}

void RFManager::loop()
{
    uint32_t now = millis();

    if (_apEnabled && (int32_t)(now - _apTimeout) >= 0)
    {
        WiFi.softAPdisconnect(true);
        _apEnabled = false;
    }

    switch (_state)
    {
        case FAST_START:
        {
            if (_discoverReply)
            {
                _state = LOCKED;
                break;
            }

            _state = ACTIVE_SCAN;
            break;
        }

        case ACTIVE_SCAN:
        {
            if (scanActiveChannels())
            {
                _state = LOCKED;
                break;
            }

            _state = PASSIVE_SCAN;
            break;
        }

        case PASSIVE_SCAN:
        {
            if (scanPassiveAP())
            {
                _state = ACTIVE_SCAN;
                break;
            }

            _state = FALLBACK;
            break;
        }

        case FALLBACK:
        {
            enterFallback();
            _state = FAST_START;
            break;
        }

        case LOCKED:
        default:
            break;
    }
}
void RFManager::attachMesh(EspNowMesh* mesh)
{
    _mesh = mesh;
}

int RFManager::getRouterChannel(const char* ssid, int iDefault)
{
    int n = WiFi.scanNetworks();
    int bestChannel = iDefault;
    int bestRSSI = -1000;
    for (int i = 0; i < n; i++)
    {
        if (WiFi.SSID(i) == ssid)
        {
            int ch = WiFi.channel(i);
            int rssi = WiFi.RSSI(i);
            Serial.printf( "[RF] Match %-16s CH:%2d RSSI:%4d\n", ssid, ch, rssi);
            if (rssi > bestRSSI)
            {
                bestRSSI = rssi;
                bestChannel = ch;
            }
        }
    }
    WiFi.scanDelete();
    if (bestRSSI > -1000)
    {
        Serial.printf("[RF] Selected CH:%d RSSI:%d\n", bestChannel,bestRSSI);
    }
    else
    {
        Serial.printf("[RF] SSID '%s' not found, using default CH:%d\n", ssid, iDefault);
    }
    return bestChannel;
}

void RFManager::setChannel(uint8_t ch)
{
    _channel = ch;

    WiFi.disconnect(true);
    delay(10);
    WiFi.setChannel(ch);
}

void RFManager::startESPNow()
{
    esp_now_init();
    esp_now_register_recv_cb(recvStatic);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, ESP_BCAST, 6);
    peer.channel = _channel;
    peer.encrypt = false;
    Serial.println("Start ESPNow Ch >"+String(_channel)+"<");
    esp_now_add_peer(&peer);
}

void RFManager::stopESPNow()
{
    esp_now_deinit();
}

void RFManager::sendDiscover(uint8_t  ttl = 1)
{
    mesh_packet_t pkt = {};

    pkt.origin = _nodeId;
    pkt.msg_id = nextMsgId() ; // 0
    pkt.type   = MESH_DISCOVER;
    pkt.ttl    = ttl;
    pkt.mc     = time(NULL);
    
    mesh_packet_t::prepareForSend(pkt);
    esp_now_send( ESP_BCAST, (uint8_t*)&pkt, sizeof(pkt));
    Serial.println("sendDiscover ");
}

void RFManager::sendDiscoverAck(const uint8_t* destMac, uint8_t ttl)
{
    mesh_packet_t pkt = {};

    pkt.origin = _nodeId;
    pkt.msg_id = nextMsgId();
    pkt.type   = MESH_DISCOVER_ACK;
    pkt.ttl    = ttl;
    pkt.mc     = time(nullptr);
    pkt.payload_len = 1;
    pkt.payload[0]  = _channel;

    mesh_packet_t::prepareForSend(pkt);

    esp_now_send(destMac, (uint8_t*)&pkt, sizeof(pkt));
    Serial.println("sendDiscoverAck ");

}

bool RFManager::scanActiveChannels()
{
    for (uint8_t ch = 1; ch <= 13; ch++)
    {
        stopESPNow();          // <-- important
        setChannel(ch);
        startESPNow();

        _discoverReply = false;
        sendDiscover();

        _discoverStart = millis();

        while (millis() - _discoverStart < 300)
        {
            if (_discoverReply)
            {
                _channel = ch;
                return true;
            }
            delay(1);
        }
    }
    return false;
}

bool RFManager::scanPassiveAP()
{
    int n = WiFi.scanNetworks();

    for (int i = 0; i < n; i++)
    {
        if (WiFi.SSID(i).indexOf("MeshGateway") >= 0)
        {
            setChannel(WiFi.channel(i));
            return true;
        }
    }
    WiFi.scanDelete();
    return false;
}

void RFManager::enterFallback()
{
    _channel = _defaultChannel;
    setChannel(_channel);
    startESPNow();
    sendDiscover();
}

void RFManager::onDiscoverReply(uint32_t gatewayId, uint8_t channel)
{
    _discoverReply = true;
    _gatewayId = gatewayId;
    _channel = channel;
}

void RFManager::recvStatic(const esp_now_recv_info_t* info,  const uint8_t* data, int len)
{
    if (_instance)
        _instance->recv(info, data, len);
}

void RFManager::recv( const esp_now_recv_info_t* info, const uint8_t* data,  int len)
{
//    Serial.println("RFManager::recv ");

    if (len != sizeof(mesh_packet_t))
        return;

    const mesh_packet_t* pkt = (const mesh_packet_t*)data;

    if (!mesh_packet_t::validate(*pkt))
        return;

    //
    // RFManager-specific packets
    //
    if (pkt->type == MESH_DISCOVER_ACK)
    {
        onDiscoverReply( pkt->origin, pkt->payload[0]);
    }

    //
    // forward everything to mesh
    //
    if (_mesh)
    {
        _mesh->receivePacket(info, data, len);
    }
}

void RFManager::onReceive(const uint8_t* data, int len)
{
    if (len != sizeof(mesh_packet_t))
        return;

    const mesh_packet_t* pkt = (const mesh_packet_t*)data;

    if (!mesh_packet_t::validate(*pkt))
        return; // corrupted packet

    // safe to use pkt now
    // 🔥 forward into mesh layer
    if (_mesh)
    {
        _mesh->injectRFPacket(*pkt);
    }    
}

void RFManager::setAPEnabled(bool en, const char* ssid, const char* pass)
{
    if (en && !_apEnabled)
    {
        strcpy(_ssid, ssid ? ssid : "NodeAP");

        WiFi.softAP(_ssid, pass, _channel);
        _apEnabled = true;
    }
    else if (!en && _apEnabled)
    {
        WiFi.softAPdisconnect(true);
        _apEnabled = false;
    }
}

void RFManager::triggerAP(uint32_t durationMs)
{
    setAPEnabled(true, "NodeConfig");
    _apTimeout = millis() + durationMs;
}

uint32_t RFManager::nextMsgId()
{
    _msgId++;
    if (_msgId == 0) {
        _msgId = 1;         // wrap protection
    }
    return _msgId;
}
/*
uint32_t nextMsgId()  // better for later
{
    return (uint32_t)(esp_random());
}
*/
void RFManager::setLastMsgId(uint32_t id)
{
    _msgId = id;
}