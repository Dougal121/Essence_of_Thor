// ================= EspNowMesh.h =================
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <sys/time.h>
#include <time.h>
#include <cstring>
#include <ctime>

#include "MeshPacket.h"   // <-- REQUIRED
//#include <cstdint>

#define MAX_TX_QUEUE 4
#define MAX_MESH_NODES 32
#define MESH_TTL_DEFAULT 5
#define HEARTBEAT_SEC 60
#define TIMESYNC_SEC 300
#define NODE_TIMEOUT 180

typedef struct {
  int32_t  node;
  int8_t   rssi_rx;
  int8_t   rssi_tx;
  time_t   last_rx;
  time_t   last_tx;
  uint8_t  type_rx;
  uint8_t  type_tx;
  uint32_t total_packets;
  uint32_t acked_packets;
  uint8_t mac[6];       
} mesh_node_t;

static const uint8_t ESP_BROADCAST[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

class EspNowMesh {
public:
  bool begin(uint32_t nodeId, bool isGateway , uint8_t channel);
  void loop();
	bool begin() {
	  uint32_t nodeId = ESP.getEfuseMac() & 0xFFFFFFFF;
	  return begin(nodeId, false , 6);
	}

  bool sendDiscover();
  int  discoverMeshChannel();

  bool sendData(uint8_t *data, uint8_t len);
  void sendHeartbeat();
  void sendTimeSync();

  uint32_t getTotalTX();
  uint32_t getTotalRX();
  uint32_t getTotalRelay();

  bool getTXBlink();
  bool getRXBlink();
  bool getRelayBlink();

  int  checkNodes();
  void clearNodeList();
  int8_t getNodeRSSI(int32_t node);
//  void onData(void (*cb)(mesh_packet_t &pkt));
  void onData(int (*cb)(const uint8_t* data, int len));
  void receivePacket(const esp_now_recv_info_t* info, const uint8_t* data, int len);
  void injectRFPacket(const mesh_packet_t& pkt);
  String getNodeListHtml();
private:
  static void _onReceiveStatic(const esp_now_recv_info_t *info, const uint8_t *data, int len);
  void _onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len);
    
  void updateNode(uint32_t node, int8_t rssi, int8_t rssitx,const uint8_t *mac, uint8_t  type);
  void updateNodeTX(uint32_t node, time_t tx, uint8_t  type);
  void sendAck(const mesh_packet_t &pkt);
  
  String macToString(const uint8_t *mac);
  void queueRebroadcast(const mesh_packet_t& pkt);

  uint32_t nextMsgId();
  bool alreadySeen(uint32_t id);
  uint16_t calcCRC(const mesh_packet_t &pkt);
  bool validCRC(const mesh_packet_t &pkt);


  static EspNowMesh *_instance;

  uint32_t _nodeId;
  bool _isGateway;
  uint32_t _msgCounter;
  bool _discoverReply = false;
  uint8_t _discoverChannel = 0 ; 
  bool _discoverGateway = false ;
  uint32_t _totaltx = 0;
  uint32_t _totalrx = 0;
  uint32_t _totalrelay = 0;
  bool      _bBlinkRX = false ;
  bool      _bBlinkRelay = false ;
  bool      _bBlinkTX = false ;

  mesh_node_t _nodes[MAX_MESH_NODES];

  static const int SEEN_CACHE = 32;
  uint32_t _seen[SEEN_CACHE];
  uint8_t  _seenIndex;

  mesh_packet_t _txQueue[MAX_TX_QUEUE];
  uint32_t      _txDue[MAX_TX_QUEUE];   // ms
  volatile int  _txCount = 0;  

  portMUX_TYPE meshMux = portMUX_INITIALIZER_UNLOCKED;
  volatile bool _heartbeatSeen = false;

  bool apEnabled = false;
  uint32_t apTimeoutMs = 0;
  uint8_t gChannel = 1 ;

// void (*_dataCb)(mesh_packet_t &pkt);
  int (*_dataCb)(const uint8_t* data, int len);
};

