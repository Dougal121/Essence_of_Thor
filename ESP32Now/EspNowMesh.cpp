#include "EspNowMesh.h"

// ================= EspNowMesh.cpp =================

EspNowMesh* EspNowMesh::_instance = nullptr;

bool EspNowMesh::begin(uint32_t nodeId, bool isGateway , uint8_t channel) {
  _instance = this;
  _nodeId = nodeId;
  _isGateway = isGateway;
  _msgCounter = 0;
  _seenIndex = 0;
  _dataCb = nullptr;
 
//  setCpuFrequencyMhz(80);  do this elsewhere ?
  clearNodeList();
  return true;
}

void EspNowMesh::loop() {
  static unsigned long lastHb = 0;
  static unsigned long lastTs = 0;

  unsigned long nowms = millis();

  if (nowms - lastHb > HEARTBEAT_SEC * 1000UL) {
    sendHeartbeat();
    lastHb = nowms;
  }

  if (_isGateway && (nowms - lastTs > TIMESYNC_SEC * 1000UL)) {
    sendTimeSync();
    lastTs = nowms;
  }

  portENTER_CRITICAL(&meshMux);
  for (int i = 0; i < _txCount; ) {
    if ((int32_t)(nowms - _txDue[i]) >= 0) {
      esp_now_send(ESP_BROADCAST, (uint8_t*)&_txQueue[i], sizeof(mesh_packet_t));      
      for (int j = i; j < (_txCount - 1); j++) {
        _txQueue[j] = _txQueue[j + 1];
        _txDue[j]   = _txDue[j + 1];
      }
      _txCount--;
    } else {
      i++;
    }
  }
  portEXIT_CRITICAL(&meshMux);
}

uint32_t EspNowMesh::getTotalTX(){
  return(_totaltx);
}
uint32_t EspNowMesh::getTotalRX(){
  return(_totalrx);
}
uint32_t EspNowMesh::getTotalRelay(){
  return(_totalrelay);
}

bool EspNowMesh::getTXBlink(){
  bool bRet = _bBlinkTX;
  _bBlinkTX = false ; 
  return(bRet);
}
bool EspNowMesh::getRXBlink(){
  bool bRet = _bBlinkRX;
  _bBlinkRX = false ; 
  return(bRet);
}
bool EspNowMesh::getRelayBlink(){
  bool bRet = _bBlinkRelay;
  _bBlinkRelay = false ; 
  return(bRet);  
}


void EspNowMesh::receivePacket( const esp_now_recv_info_t* info, const uint8_t* data, int len)
{
    _onReceive(info, data, len);
}

void EspNowMesh::onData(int (*cb)(const uint8_t* data, int len)) {
  _dataCb = cb;
}

void EspNowMesh::queueRebroadcast(const mesh_packet_t& pkt)
{
  if (_txCount >= MAX_TX_QUEUE) 
    return;
  portENTER_CRITICAL(&meshMux);
  _txQueue[_txCount] = pkt;
  _txDue[_txCount] = millis() + random(10, 50);
  _txCount++;
  portEXIT_CRITICAL(&meshMux);
}

int EspNowMesh::discoverMeshChannel()
{
    Serial.println("[MESH] Starting discovery");

    for (uint8_t ch = 1; ch <= 13; ch++)
    {
        Serial.printf("[MESH] Trying CH %u\n", ch);
        esp_now_deinit();
        delay(20);
//        WiFi.mode(WIFI_STA);
        WiFi.disconnect(true);
        delay(20);
        WiFi.setChannel(ch);
        delay(10);
        if (esp_now_init() != ESP_OK)
        {
            Serial.println("[MESH] esp_now_init failed");
            continue;
        }
        esp_now_register_recv_cb(_onReceiveStatic);

        esp_now_peer_info_t peer = {};
        memset(peer.peer_addr, 0xFF, 6);
        peer.channel = ch;
        peer.encrypt = false;
        esp_now_add_peer(&peer);
        _discoverReply = false;
        sendDiscover();
        uint32_t t0 = millis();

        while ((uint32_t)(millis() - t0) < 300)
        {
            delay(1);
            if (_discoverReply)
            {
                Serial.printf( "[MESH] Found gateway %08X on CH %u\n",_discoverGateway, ch);
                return ch;
            }
        }
    }
    Serial.println("[MESH] Discovery failed");
    return -1;
}

void EspNowMesh::_onReceiveStatic(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (_instance)
    _instance->_onReceive(info, data, len);
}

void EspNowMesh::injectRFPacket(const mesh_packet_t& pkt)
{
    // reuse your existing pipeline
    _onReceive(nullptr, (const uint8_t*)&pkt, sizeof(pkt));
}

void EspNowMesh::_onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  bool rebroadcast = true ;  
   _totalrx++;
   _bBlinkRX = true ;
  if (len != sizeof(mesh_packet_t)) {
    Serial.println("EspNowMesh::_onReceive length wrong");
    return;
  }
  mesh_packet_t pkt;
  memcpy(&pkt, data, sizeof(pkt));

  Serial.println("EspNowMesh::_onReceive MAC " + macToString(info->src_addr)+ " ID " + String(pkt.msg_id)+ " Packet Type "+String(pkt.type));

  if (!validCRC(pkt)){ 
    Serial.println(">>> EspNowMesh::_onReceive Invalid CRC");
    return;
  }
  if (alreadySeen(pkt.msg_id)) {
    Serial.println(">>> EspNowMesh::_onReceive already seen " + String(pkt.msg_id));
    return;
  }

  int8_t rssi = info->rx_ctrl->rssi;
  updateNode(pkt.origin, rssi, pkt.rssi, info->src_addr,pkt.type);

   switch (pkt.type) {
    case MESH_DISCOVER:   // 5
      if (_isGateway)
      {
          mesh_packet_t reply = {};
          reply.origin = _nodeId;
          reply.msg_id = nextMsgId();
          reply.type   = MESH_DISCOVER_ACK;
          reply.ttl    = 0;
          reply.mc     = time(NULL);
          reply.rssi = rssi ; 
          mesh_packet_t::prepareForSend(reply);
          esp_now_send(info->src_addr, (uint8_t*)&reply, sizeof(reply));
          updateNodeTX(pkt.origin, reply.mc, reply.type );
      }
      break;

    case MESH_DISCOVER_ACK:  // 6
      _discoverReply   = true;
      _discoverChannel = WiFi.channel();
      _discoverGateway = pkt.origin;
      break;    

    case WAKEUP_AP:    // 7 
//        startAP();
      break;

    case MESH_HEARTBEAT:     // 1
      _heartbeatSeen = true;    
      break;

    case MESH_TIME_SYNC:    // 2
      if (!_isGateway) {
        time_t now = time(NULL);
        long diff = labs((long)pkt.mc - (long)now);  // if not a gateway and more than 5 secnds from master clock set the clock
        if (diff > 5 || now < 1577836800) {
          struct timeval tv;
          tv.tv_sec = pkt.mc;
          tv.tv_usec = 0;
          settimeofday(&tv, nullptr);   
        }
      }
      break;

    case MESH_DATA:   //   3
      if (_dataCb)          // _dataCb(pkt);
        pkt.rssi = rssi ;   // send back our signal strength
        if ( _dataCb(pkt.payload, pkt.payload_len) != 0 ){
          sendAck(pkt);       // send the packet ack only if it was actually processed
          rebroadcast = false;     
          Serial.println("EspNowMesh::_onReceive processed != 0 ");     
        }
      break;

    case MESH_ACK:           // 4
      rebroadcast = false;
      break;
  }
  if (rebroadcast && (pkt.ttl>0)){
    pkt.ttl-- ; 
    _totalrelay++ ;
    _bBlinkRelay = true;
    mesh_packet_t::prepareForSend(pkt); // ttl changed so update
    queueRebroadcast(pkt);
    Serial.println("EspNowMesh::_onReceive Relay Packet");
  }  // que to send again
  return;
}

bool EspNowMesh::sendData(uint8_t *data, uint8_t len) {
  if (len > sizeof(((mesh_packet_t*)0)->payload)) 
    return false;

  mesh_packet_t pkt = {};
  pkt.origin = _nodeId;
  pkt.msg_id = nextMsgId();
  pkt.type = MESH_DATA;
  pkt.ttl = MESH_TTL_DEFAULT;
  pkt.mc = time(NULL);
  pkt.rssi = getNodeRSSI(_nodeId); // send how the node appears if available
  pkt.payload_len = len;
  memcpy(pkt.payload, data, len);
//  pkt.crc = calcCRC(pkt);
  _totaltx++ ;
  _bBlinkTX = true ;
  alreadySeen(pkt.msg_id);
  mesh_packet_t::prepareForSend(pkt);
  return esp_now_send(ESP_BROADCAST, (uint8_t*)&pkt, sizeof(pkt)) == ESP_OK;
}

void EspNowMesh::sendHeartbeat() {
  mesh_packet_t pkt = {};
  pkt.origin = _nodeId;
  pkt.msg_id = nextMsgId();
  pkt.type = MESH_HEARTBEAT;
  pkt.ttl = MESH_TTL_DEFAULT;
  pkt.mc = time(NULL);
  alreadySeen(pkt.msg_id);
  mesh_packet_t::prepareForSend(pkt);
  esp_now_send(ESP_BROADCAST, (uint8_t*)&pkt, sizeof(pkt));
  Serial.println("EspNowMesh::sendHeartbeat ");
}

void EspNowMesh::sendTimeSync() {
  mesh_packet_t pkt = {};
  pkt.origin = _nodeId;
  pkt.msg_id = nextMsgId();
  pkt.type = MESH_TIME_SYNC;
  pkt.ttl = MESH_TTL_DEFAULT + 1;
  pkt.mc = time(NULL);
  alreadySeen(pkt.msg_id);
  mesh_packet_t::prepareForSend(pkt);
  esp_now_send(ESP_BROADCAST, (uint8_t*)&pkt, sizeof(pkt));
  Serial.println("EspNowMesh::sendHeartbeat ");
}

void EspNowMesh::sendAck(const mesh_packet_t &rx) {
  mesh_packet_t pkt = {};
  pkt.origin = _nodeId;
  pkt.msg_id = nextMsgId(); //rx.msg_id;
  pkt.type = MESH_ACK;
  pkt.ttl = 1;
  pkt.mc = time(NULL);
//  pkt.crc = calcCRC(pkt);
  alreadySeen(pkt.msg_id);
  mesh_packet_t::prepareForSend(pkt);  
  esp_now_send(ESP_BROADCAST, (uint8_t*)&pkt, sizeof(pkt));
  updateNodeTX(pkt.origin, pkt.mc, pkt.type );
}

void EspNowMesh::updateNodeTX(uint32_t node, time_t tx, uint8_t  type) {
  for (int i = 0; i < MAX_MESH_NODES; i++) {
    if (_nodes[i].node == (int32_t)node) {
      _nodes[i].last_tx = tx;
      _nodes[i].type_tx = type;
      _nodes[i].total_packets++;
      return;
    }
  }  
}

void EspNowMesh::updateNode(uint32_t node, int8_t rssi, int8_t rssitx, const uint8_t *mac, uint8_t  type) {
  int freeIdx = -1;
  time_t now = time(NULL);

  for (int i = 0; i < MAX_MESH_NODES; i++) {
    if (_nodes[i].node == (int32_t)node) {
      _nodes[i].rssi_rx = rssi;
      _nodes[i].rssi_tx = rssitx;
      _nodes[i].last_rx = now;
      _nodes[i].type_rx = type; 
      if ((type==MESH_ACK)||(type==MESH_DISCOVER_ACK)) 
        _nodes[i].acked_packets++;     
      _nodes[i].total_packets++;
       memcpy(_nodes[i].mac, mac, 6);
      return;
    }
    if (_nodes[i].node < 0 && freeIdx < 0) {
      freeIdx = i;
    }
  }

  if (freeIdx >= 0) {
    _nodes[freeIdx].node = (int32_t)node;
    _nodes[freeIdx].rssi_rx = rssi;
    _nodes[freeIdx].rssi_tx = rssitx;    
    _nodes[freeIdx].last_rx = now;
    _nodes[freeIdx].total_packets = 1;
    _nodes[freeIdx].acked_packets = 0;
    memcpy(_nodes[freeIdx].mac, mac, 6);
  }
}

int8_t EspNowMesh::getNodeRSSI(int32_t node) {
  int8_t iRet = 0 ; 
  for (int i = 0; i < MAX_MESH_NODES; i++) {
    if (_nodes[i].node == (int32_t)node) 
      iRet = _nodes[i].rssi_rx;
  }
  return(iRet);
}

void EspNowMesh::clearNodeList() {
  memset(_seen, 0, sizeof(_seen));
  _seenIndex = 0;
  for (int i = 0; i < MAX_MESH_NODES; i++) {
    _nodes[i].node = -1;
    _nodes[i].rssi_rx = 0;
    _nodes[i].rssi_tx = 0;
    _nodes[i].last_rx = 0;
    _nodes[i].last_tx = 0;
    _nodes[i].total_packets = 0;
    _nodes[i].acked_packets = 0;
  }
}

String EspNowMesh::macToString(const uint8_t *mac)
{
    char buf[18];
    snprintf(buf, sizeof(buf),"%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

String PacketTypeToText(uint8_t  type)
{
  switch(type){
    case 1:
      return("Heatbeat");
    break;
    case 2:
      return("Time Sync");
    break;
    case 3:
      return("Data");
    break;
    case 4:
      return("Ack");
    break;
    case 5:
      return("Discover");
    break;
    case 6:
      return("Discover Ack");
    break;
    case 7:
      return("Wakeup AP");
    break;
    default:
      return(">>Unknown<<");
    break;
  }
}


String formatTime(time_t t)
{
    if (t < 1577836800)        // Before 2020
        return "&gt;NA&lt;";   // >NA<
    time_t now = time(nullptr);        
//    if (now > 1577836800 && t > (now + (10L * 365 * 24 * 3600)))   // More than 10 years in the future
//        return "&gt;NA&lt;";
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return String(buf);
}
String formatDeltaTime(time_t t)
{
  if (t < 1577836800)        // Before 2020
    return "&gt;NA&lt;";   // >NA<
  time_t now = time(NULL);        
  return (String(now - t));
}

String EspNowMesh::getNodeListHtml()
{
    String s;

    s += "<table border='1 title='Mesh Nodes'>";
    s += "<tr>";
    s += "<th rowspan=2>Node</th>";
    s += "<th rowspan=2>MAC<br>Address</th>";
    s += "<th colspan=2RSSI</th>";
    s += "<th colspan=2>Packets</th>";
    s += "<th colspan=3>Last RX </th>";
    s += "<th colspan=3>Last TX</th>";
    s += "</tr>";
    s += "<tr>";
    s += "<th>RX</th>";
    s += "<th>TX</th>";
    s += "<th>Total</th>";
    s += "<th>Acked</th>";
    s += "<th colspan=2>Time   &Delta;T</th>";
    s += "<th>Type </th>";
    s += "<th colspan=2>Time   &Delta;T</th>";
    s += "<th>Type</th>";
    s += "</tr>";

 

    for (int i = 0; i < MAX_MESH_NODES; i++)
    {
        if (_nodes[i].node >= 0)
        {
            s += "<tr align=center>";
            s += "<td>" + String(_nodes[i].node) + "</td>";
            s += "<td>" + macToString(_nodes[i].mac) + "</td>";
            s += "<td>" + String(_nodes[i].rssi_rx) + "</td>";
            s += "<td>" + String(_nodes[i].rssi_tx) + "</td>";
           
            s += "<td>" + String(_nodes[i].total_packets) + "</td>";
            s += "<td>" + String(_nodes[i].acked_packets) + "</td>";

            s += "<td>" + String(formatTime(_nodes[i].last_rx)) + "</td>";
            s += "<td>" + String(formatDeltaTime(_nodes[i].last_rx)) + "</td>";
            s += "<td>" + String(PacketTypeToText(_nodes[i].type_rx)) + "</td>";

            s += "<td>" + String(formatTime(_nodes[i].last_tx)) + "</td>";
            s += "<td>" + String(formatDeltaTime(_nodes[i].last_tx)) + "</td>";
            s += "<td>" + String(PacketTypeToText(_nodes[i].type_tx)) + "</td>";
            s += "</tr>";
        }
    }

    s += "</table>";

    return s;
}

int EspNowMesh::checkNodes() {
  int dead = 0;
  time_t t = time(NULL);

  for (int i = 0; i < MAX_MESH_NODES; i++) {
    if (_nodes[i].node >= 0 && (t - _nodes[i].last_rx) > NODE_TIMEOUT) {
      _nodes[i].node = -1;
      dead++;
    }
  }
  return dead;
}


uint32_t EspNowMesh::nextMsgId() {
  return (_nodeId << 16) | (_msgCounter++ & 0xFFFF);
}

bool EspNowMesh::alreadySeen(uint32_t id) {
  for (int i = 0; i < SEEN_CACHE; i++) {
    if (_seen[i] == id) 
      return true;
  }
  _seen[_seenIndex++] = id;
  _seenIndex %= SEEN_CACHE;
  return false;
}


bool EspNowMesh::validCRC(const mesh_packet_t &pkt)
{
    mesh_packet_t temp = pkt;
    uint16_t received = temp.crc;
    temp.crc = 0;
    return mesh_packet_t::calcCRC(temp) == received;
}


