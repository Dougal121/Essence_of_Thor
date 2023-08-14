unsigned long sendCTRLpacket(){
int j ;  
int ii ;
IPAddress ctrlIP ;
cnc_t  cnc ;  
String strDestIP ;

  Serial.println("CTRL packet called...");

//    Serial.println("WiFi Available...");
                      
    memset(packetBuffer, 0, sizeof(packetBuffer));    // set all bytes in the buffer to 0
    cnc.cmd = 42 ;
    cnc.snode = (uint8_t)( ghks.lNodeAddress & 0xff ) ;
    cnc.mc = now() ; // set master clock
    ii = 0 ;
    for ( int i  = 0 ;  i < MAX_VALVE ; i++ ){
      if (( evalve[i].Node != ghks.lNodeAddress ) && ( evalve[i].Node != 0 ) && (( evalve[i].Valve & 0x80 ) == 0x00 ) && (ii < MAX_CNC)) {           // && ( evalve[i].Valve != 0 )
        cnc.cv[ii].lTTG = vvalve[i].lTTG ;
        cnc.cv[ii].lATTG = vvalve[i].lATTG ;
        cnc.cv[ii].lTTC = vvalve[i].lTTC ;
        cnc.cv[ii].Fertigate = evalve[i].Fertigate ;
        cnc.cv[ii].Flowrate = evalve[i].Flowrate ;
        cnc.cv[ii].Node = evalve[i].Node ;
        cnc.cv[ii].ValveNo = ( evalve[i].Valve -1 ) & 0x7f ;                               // top bit it indicate this recieves data
        ii++ ;                                                                             // increment for next time
      }
    }
    cnc.valves = ii ;
    cnc.crc = calculateCRC16(&cnc,sizeof(cnc)-2);
    if ( ii > 0 ){                                                                         // there are valves to be uplinked
//      WiFi.hostByName(address, ghks.RCIP );
      if (WiFi.isConnected())  {
        if  (( ghks.RCIP[0] == 0 ) &&  ( ghks.RCIP[1] == 0 ) && ( ghks.RCIP[2] == 0 ) && ( ghks.RCIP[3] == 0 )){
          Serial.println("No DNS - no point sending NTP to 0.0.0.0 ");      
        }else{
          strDestIP = GetCNCIP();
//          snprintf(buff, BUFF_MAX, "%u.%u.%u.%u\0",ghks.RCIP[0], ghks.RCIP[1], ghks.RCIP[2], ghks.RCIP[3]);
          Serial.print("Sending CTRL packet - IP port - address: "+String(ghks.RemotePortCtrl)+" ->  ");
          Serial.println(strDestIP);
          ctrludp.beginPacket( strDestIP.c_str() , ghks.RemotePortCtrl);                                 // Send control data to the remote port - Broadcast ???
          ctrludp.write((byte *)&cnc, sizeof(cnc));
          ctrludp.endPacket();
        }
      }
      else{
        Serial.println("No WiFi for uplink ???");
      }      
      if ( bLoRa ){
        Serial.println("Sending LoRa CNC packet " + String( sizeof(cnc))+ " bytes");
        LoRa.beginPacket();
//          LoRa.print("test 123");
        LoRa.write((byte *)&cnc, sizeof(cnc));   
//        LoRaLastRssi = LoRa.packetRssi();                             // Get RSSI
//        LoRaLastSnr = LoRa.packetSnr();
//        LoRaLastFrequencyError = LoRa.packetFrequencyError ();        
        LoRa.endPacket();      
        Serial.println("LoRa CNC packet sent");
      }
      for ( int j  = 0 ;  j < MAX_VALVE ; j++ ){
        if (( evalve[j].Node != ghks.lNodeAddress ) && ( evalve[j].Node != 0 ) && (( evalve[j].Valve & 0x80 ) == 0x00 ) ) {       // && ( evalve[i].Valve != 0 )
          for (int i = 0 ; i < MAX_REM_LIST ; i++ ) {                                                                             // setup the defaults in the remote nodes list
            if (( remlist[i].node == evalve[j].Node ) || ( remlist[i].node == -1 )) {
              remlist[i].txt =  now() ;     
              remlist[i].node = evalve[j].Node ;       
              i = MAX_REM_LIST ;     
            }
          }          
        }
      }
    }
}


unsigned long processCtrlUDPpacket(long lSize){
int i , j ;  
int ii ;
byte packetBuffer[16];
unsigned long timediff ;
cnc_t  cnc ;  
wet_t  cmp ;
uint16_t  crc ;
String strDestIP ;
IPAddress ReturnIP ;

  Serial.println("Process Uplinked WiFi data " + String(lSize) + " bytes");  
  if ( lSize == sizeof(wet_t) ){
    memset(&cmp, 0, sizeof(cmp));
    ctrludp.read((byte *)&cmp, sizeof(cmp));                               // read the packet into the buffer
    crc = calculateCRC16(&cmp,sizeof(cmp)-2);     
    if (crc == cmp.crc){                                                   // ensure valid command and packet crc
      if ( cmp.cmd == 43 ){                                                // someting different
        j = MAX_CNC ;
        if ( j > cmp.valves ) 
          j = cmp.valves ;                                                 // only do valid ones
        for (int i = 0 ; i < j ; i++ ) {                                   // setup the defaults in the remote nodes list
          if (( cmp.mp[i].ValveNo < MAX_VALVE ) && ( cmp.mp[i].ValveNo >= 0)  && (cmp.mp[i].Node == ghks.lNodeAddress) ) {   //   vvalve
            vvalve[cmp.mp[i].ValveNo].dblMoisture = cmp.mp[i].Wet ;
            vvalve[cmp.mp[i].ValveNo].dblTemp = cmp.mp[i].Temp ;
            vvalve[cmp.mp[i].ValveNo].dblDepth = cmp.mp[i].Depth ;
            vvalve[cmp.mp[i].ValveNo].mpTime = cmp.mc ;
          }
        }        
        Serial.println("UDP moisture Packet processed");              
      }
    }else{
      Serial.println("CRC Failed on UDP moisture Packet");              
    }
    return(2);  
  }
  
  if ( lSize ==  sizeof(cnc_ack) ){
    ctrludp.read((byte *)&cnc_ack, sizeof(cnc_ack));                                  // read the packet into the buffer
    crc = calculateCRC16(&cnc_ack,sizeof(cnc_ack)-2);     
    if (crc == cnc_ack.crc){                                                          // ensure valid command and packet crc
      if ( cnc_ack.cmd == 42 ){
        for (int i = 0 ; i < MAX_REM_LIST ; i++ ) {                                   // setup the defaults in the remote nodes list
          if (( remlist[i].node = cnc_ack.snode ) || ( remlist[i].node = -1 )){
            remlist[i].node = cnc_ack.snode ;
            remlist[i].TxRssi =  cnc_ack.Rssi  ;
            remlist[i].TxSnr = cnc_ack.Snr ;
            remlist[i].txt =  cnc_ack.mc ;     
            remlist[i].RxRssi = WiFi.RSSI()  ;
            remlist[i].RxSnr =  0 ;
            remlist[i].rxt =  now() ;     
            remlist[i].total = cnc_ack.total ;
            remlist[i].uplinked = cnc_ack.uplinked ;   
            remlist[i].TotalPackets++ ;
            i = MAX_REM_LIST ;                                  // exit as we have found the target  
          }
        }
      }
    }else{
      Serial.println("CRC Failed on UDP CNC_ACK Packet");      
    }
  }else{   // if not ack packet then it must be cnc
    
    memset(&cnc, 0, sizeof(cnc));
    memset(&cnc_ack, 0, sizeof(cnc));
    cnc_ack.Rssi = WiFi.RSSI() ;      // start to build the ackknolode packet 
    cnc_ack.Snr = 0 ;
    cnc_ack.FE = 0 ;
    cnc_ack.snode = ghks.lNodeAddress ;
    cnc_ack.mc = now() ;
    
    ctrludp.read((byte *)&cnc, sizeof(cnc)); // read the packet into the buffer
    crc = calculateCRC16(&cnc,sizeof(cnc)-2);
    ReturnIP = ctrludp.remoteIP(); 
    
    cnc_ack.cmd = cnc.cmd ;
    if ( crc == cnc.crc ){
      switch(cnc.cmd ){ // command byte
        case 42:
          j = MAX_CNC ;
          if ( j > cnc.valves ) 
            j = cnc.valves ;
          cnc_ack.total = j ;
          timediff = abs(cnc.mc - now());
          if ((( timediff > 30 ) && ( timediff < SECS_PER_DAY )) || ( year() < 2020 )){   // the 2020 exception is for startup
            setTime((time_t)cnc.mc);   // set node to the master clock
            snprintf(buff, BUFF_MAX, "Time set from master clock %d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());          
            Serial.println( String(buff)) ;                   
          }              
          for ( int i  = 0 ;  i < j ; i++ ){ // only process live ones
            ii = ( cnc.cv[i].ValveNo & 0x7f ) ; 
            Serial.println("Valve " + String(ii) + " Uplink (42) status " + String(evalve[ii].Valve));      
            if (( ii >= 0 ) && (ii < MAX_CNC ) && ((evalve[ii].Valve & 0x80) != 0)){  // valve to accept uplink and valve number in range
              if ( ghks.lNodeAddress == cnc.cv[i].Node ){
                if ( evalve[ii].bEnable ){
                  vvalve[ii].lATTG = cnc.cv[i].lATTG ;
                } 
                vvalve[ii].lTTG = cnc.cv[i].lTTG ; 
                vvalve[ii].lTTC = cnc.cv[i].lTTC ; 
                evalve[ii].Fertigate = cnc.cv[i].Fertigate ;
                evalve[ii].Flowrate = cnc.cv[i].Flowrate ;
                Serial.println("Uplinked data accepted Valve No " + String(ii));      
                cnc_ack.uplinked++ ;
              }
            }
          }
          break;
      }
      snprintf(buff, BUFF_MAX, "%u.%u.%u.%u\0",ReturnIP[0], ReturnIP[1], ReturnIP[2], ReturnIP[3]);    // return the acknologe to the sender      
      cnc_ack.crc = calculateCRC16(&cnc_ack,sizeof(cnc_ack)-2);                                        // dont forget to add the crc the crc !    
      ctrludp.beginPacket( buff, ghks.RemotePortCtrl);                                                 // Send control data to the remote port - Broadcast ???
      ctrludp.write((byte *)&cnc_ack, sizeof(cnc_ack));
      ctrludp.endPacket();
    }else{
      Serial.println("CRC Failed on UDP CNC Packet");
    }
  }

  while (ctrludp.available()){  // clean out the rest of the packet and dump overboard
    ctrludp.read(packetBuffer, sizeof(packetBuffer));  
  } 
}

unsigned long DoLaRaStuff(){
cnc_t  cnc ;  
int j ;
int ii ;
byte packetBuffer[16];
int packetSize = 0 ;
unsigned long timediff ;
uint16_t     crc ;

  if (bLoRa) {                                                      // check we LoRa working 
    packetSize = LoRa.parsePacket();
    if (packetSize){
      Serial.println("Process Uplinked LoRa data " + String(packetSize) + " bytes");      
      if ( packetSize ==  sizeof(cnc_ack) ){
        Serial.println("LoRa CNC ACK packet recieved RSSI: " + String(LoRa.packetRssi())+ " SNR "+String(LoRa.packetSnr()) + " FE " + String(LoRa.packetFrequencyError()));
        LoRa.readBytes((byte *)&cnc_ack, sizeof(cnc_ack));                                       // read the packet into the structure
        crc = calculateCRC16(&cnc_ack,sizeof(cnc_ack)-2);
        while (LoRa.available()){                                                                // clean out the rest of the packet and dump overboard
          LoRa.readBytes(packetBuffer, sizeof(packetBuffer));  
        }       
        
        if (crc == cnc_ack.crc){           // ensure valid command and packet crc
          if ( cnc_ack.cmd == 42 ){
            for (int i = 0 ; i < MAX_REM_LIST ; i++ ) {                 // setup the defaults in the remote nodes list
              if (( remlist[i].node = cnc_ack.snode ) || ( remlist[i].node = -1 )){
                remlist[i].node = cnc_ack.snode ;
                remlist[i].TxRssi =  cnc_ack.Rssi  ;
                remlist[i].TxSnr = cnc_ack.Snr ;
                remlist[i].txt =  cnc_ack.mc ;     
                remlist[i].RxRssi =   LoRa.packetRssi()  ;
                remlist[i].RxSnr =  LoRa.packetSnr() ;
                remlist[i].rxt =  now() ;     
                remlist[i].total = cnc_ack.total ;
                remlist[i].uplinked = cnc_ack.uplinked ;   
                remlist[i].TotalPackets++ ;
                i = MAX_REM_LIST ;                                  // exit as we have found the target  
              }
            }
            LoRaCheck();
          }
        }
        else{
          Serial.println("CRC Failed on LoRa CNC_ACK Packet");          
        }
        
      }else{
        memset(&cnc, 0, sizeof(cnc));
        memset(&cnc_ack, 0, sizeof(cnc));
        cnc_ack.Rssi = LoRa.packetRssi();        // start to build the ackknolode packet 
        cnc_ack.Snr = LoRa.packetSnr();
        cnc_ack.FE = LoRa.packetFrequencyError();
        cnc_ack.snode = ghks.lNodeAddress ;
        cnc_ack.mc = now() ;
        LoRaLastRssi = LoRa.packetRssi();                             // Get RSSI etc     
        LoRaLastSnr = LoRa.packetSnr();
        LoRaLastFrequencyError = LoRa.packetFrequencyError();
        Serial.println("LoRa CNC packet recieved RSSI: " + String(LoRaLastRssi)+ " SNR "+String(LoRaLastSnr) + " FE " + String(LoRaLastFrequencyError));
        LoRa.readBytes((byte *)&cnc, sizeof(cnc));                                               // read the packet into the structure
        crc = calculateCRC16(&cnc,sizeof(cnc)-2);
        while (LoRa.available()){                                                                // clean out the rest of the packet and dump overboard
          LoRa.readBytes(packetBuffer, sizeof(packetBuffer));  
        }       
        cnc_ack.cmd = cnc.cmd ;    
        if ( crc == cnc.crc ){
          switch(cnc.cmd ){ // command byte
            case 42:
              j = MAX_CNC ;
              if ( j > cnc.valves ) 
                j = cnc.valves ;
              cnc_ack.total = j ;
              Serial.println("Command 42...");      
              timediff = abs(cnc.mc - now());
              if ((( timediff > 30 ) && ( timediff < SECS_PER_DAY )) || ( year() < 2020 )){   // the 2020 exception is for startup
                setTime((time_t)cnc.mc);   // set node to the master clock
                snprintf(buff, BUFF_MAX, "Time set from master clock %d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());          
                Serial.println( String(buff)) ;                   
              }
              for ( int i  = 0 ;  i < j ; i++ ){ // only process live ones
                ii = ( cnc.cv[i].ValveNo & 0x7f ) ; 
                Serial.println("Valve " + String(ii) + " Uplink (42) status " + String(evalve[ii].Valve));      
                if (( ii >= 0 ) && (ii < MAX_VALVE ) && ((evalve[ii].Valve & 0x80) != 0)){  // valve to accept uplink and valve number in range
                  if ( ghks.lNodeAddress == cnc.cv[i].Node ){
                    if ( evalve[ii].bEnable ){
                      vvalve[ii].lATTG = cnc.cv[i].lATTG ;
                    } 
                    vvalve[ii].lTTG = cnc.cv[i].lTTG ; 
                    vvalve[ii].lTTC = cnc.cv[i].lTTC ; 
                    evalve[ii].Fertigate = cnc.cv[i].Fertigate ;
                    evalve[ii].Flowrate = cnc.cv[i].Flowrate ;
                    Serial.println("Uplinked data accepted Valve No " + String(ii));      
                    cnc_ack.uplinked++ ;
                  }
                }
              }
              break;
          }   
          cnc_ack.crc = calculateCRC16(&cnc_ack,sizeof(cnc_ack)-2);
          bSendLoRaCNCACK = true ;
        } else{
          Serial.println("CRC Failed on LoRa CNC Packet");          
        }
      }
    }
  }
}


bool  ClearRemoteNodeList(void){
  for (int i = 0 ; i < MAX_REM_LIST ; i++ ) {   // setup the defaults in the remote nodes list
    remlist[i].node = -1 ;
    remlist[i].RxRssi = 0  ;
    remlist[i].RxSnr = 0 ;
    remlist[i].TxRssi = 0  ;
    remlist[i].TxSnr = 0 ;
    remlist[i].rxt = 0 ;
    remlist[i].txt = 0 ;
    remlist[i].total = 0 ;
    remlist[i].uplinked = 0 ; 
    remlist[i].TotalPackets = 0 ;
  }
  return (true);
}

int LoRaCheck(void){
  long timediff1 ;
  long timediff2 ;
  int errcnt = 0 ; 
  strLoRaResults = "" ;  // clear them

  for (int i = 0 ; i < MAX_REM_LIST ; i++ ) {   // setup the defaults in the remote nodes list
    if (remlist[i].node > -1 ){
      if ((year(remlist[i].txt) > 2020 ) && (year(remlist[i].rxt)>2020)){
        timediff1 = abs(remlist[i].txt - now());
        timediff2 = abs(remlist[i].rxt - now());
        if (( timediff1 > SMTP.iLoRaTimeOut ) || ( timediff2 > SMTP.iLoRaTimeOut )) {
          strLoRaResults += "Node " + String(remlist[i].node) + " not responeding \r\n" ;
          errcnt++ ; 
        }
      }
    }
  }  
  if ( errcnt == 0 ){
    bLoRaGood = true ; 
  }  
  return(errcnt);
}


uint16_t calculateCRC16(const void* data, size_t size)
{
    const uint16_t polynomial = 0x1021;
    uint16_t crc = 0xFFFF;

    const byte* byteData = static_cast<const byte*>(data);
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= (static_cast<unsigned short>(byteData[i]) << 8);
        for (int j = 0; j < 8; ++j)
        {
            crc = ((crc & 0x8000) ? ((crc << 1) ^ polynomial) : (crc << 1));
        }
    }
    return crc;
}


String GetCNCIP (void)
{
  if (( ghks.RCIP[0] == 0 ) && ( ghks.RCIP[1] == 0 ) && ( ghks.RCIP[2] == 0 )){
    switch(ghks.RCIP[3]){
      case 2:
        snprintf(buff, BUFF_MAX, "%u.%u.%u.255\0", MyIPC[0], MyIPC[1], MyIPC[2]);    
      break;
      default:
        snprintf(buff, BUFF_MAX, "%u.%u.%u.255\0", MyIP[0], MyIP[1], MyIP[2]);    
      break;
    }
  }else{
      snprintf(buff, BUFF_MAX, "%u.%u.%u.%u\0",ghks.RCIP[0], ghks.RCIP[1], ghks.RCIP[2], ghks.RCIP[3]);    
  }
  return(String(buff));
}

