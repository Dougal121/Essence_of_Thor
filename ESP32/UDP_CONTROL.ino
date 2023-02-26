unsigned long sendCTRLpacket(){
int j ;  
int ii ;
IPAddress ctrlIP ;
cnc_t  cnc ;  

  Serial.println("CTRL packet called...");

//    Serial.println("WiFi Available...");
                      
    memset(packetBuffer, 0, sizeof(packetBuffer));    // set all bytes in the buffer to 0
    cnc.cmd = 42 ;
    cnc.snode = (uint8_t)( ghks.lNodeAddress & 0xff ) ;
    cnc.mc = now() ; // set master clock
    ii = 0 ;
    for ( int i  = 0 ;  i < MAX_VALVE ; i++ ){
      if (( evalve[i].Node != ghks.lNodeAddress ) && ( evalve[i].Node != 0 ) && (( evalve[i].Valve & 0x80 ) == 0x00 ) && (ii < 16)) {           // && ( evalve[i].Valve != 0 )
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
    if ( ii > 0 ){                                                                         // there are valves to be uplinked
//      WiFi.hostByName(address, ghks.RCIP );
      if (WiFi.isConnected())  {
        if  (( ghks.RCIP[0] == 0 ) &&  ( ghks.RCIP[1] == 0 ) && ( ghks.RCIP[2] == 0 ) && ( ghks.RCIP[3] == 0 )){
          Serial.println("No DNS - no point sending NTP to 0.0.0.0 ");      
        }else{
          snprintf(buff, BUFF_MAX, "%u.%u.%u.%u\0",ghks.RCIP[0], ghks.RCIP[1], ghks.RCIP[2], ghks.RCIP[3]);
          Serial.print("Sending CTRL packet - IP address: "+String(ghks.RemotePortCtrl)+" ->  ");
          Serial.println(buff);
          ctrludp.beginPacket( buff, ghks.RemotePortCtrl);                                 // Send control data to the remote port - Broadcast ???
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
}


unsigned long processCtrlUDPpacket(long lSize){
int i , j ;  
int ii ;
byte packetBuffer[16];
cnc_t  cnc ;  

//  Serial.println("Process Uplinked data...");      
  memset(&cnc, 0, sizeof(cnc));
  ctrludp.read((byte *)&cnc, sizeof(cnc)); // read the packet into the buffer

  switch(cnc.cmd ){ // command byte
    case 42:
      j = MAX_VALVE ;
      if ( j > cnc.valves ) 
        j = cnc.valves ;
//      Serial.println("Command 42...");      
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
          }
        }
      }
      break;
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

  if (bLoRa) {                                                      // check we LoRa working 
    packetSize = LoRa.parsePacket();
    if (packetSize){
      Serial.println("Process Uplinked LoRa data " + String(packetSize) + " bytes");      
      if ( packetSize ==  sizeof(cnc_ack) ){
        Serial.println("LoRa CNC ACK packet recieved RSSI: " + String(LoRa.packetRssi())+ " SNR "+String(LoRa.packetSnr()) + " FE " + String(LoRa.packetFrequencyError()));
        LoRa.readBytes((byte *)&cnc_ack, sizeof(cnc_ack));                                       // read the packet into the structure
        while (LoRa.available()){                                                                // clean out the rest of the packet and dump overboard
          LoRa.readBytes(packetBuffer, sizeof(packetBuffer));  
        }       
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
        while (LoRa.available()){                                                                // clean out the rest of the packet and dump overboard
          LoRa.readBytes(packetBuffer, sizeof(packetBuffer));  
        }       
        cnc_ack.cmd = cnc.cmd ;    
        switch(cnc.cmd ){ // command byte
          case 42:
            j = MAX_VALVE ;
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
        bSendLoRaCNCACK = true ; 
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

