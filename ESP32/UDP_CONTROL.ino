unsigned long sendCTRLpacket(IPAddress address){
int j ;  
byte packetBuffer[ 10+(MAX_VALVE*6)];           //buffer to hold outgoing packets  
//  Serial.println("sending CTRL packet...");
                    
  memset(packetBuffer, 0, 10+(MAX_VALVE*6));    // set all bytes in the buffer to 0
  packetBuffer[0] = 0xff;                       // broadcast as all stations
  packetBuffer[1] = 0xff;  // 
  packetBuffer[2] = 0xff;  // 
  packetBuffer[3] = 0xff;  // 
  
/*  packetBuffer[4]  = highByte(word((lNodeAddress & 0xffff0000)>>16)) ;   // from this station node
  packetBuffer[5]  = lowByte(word((lNodeAddress & 0xffff0000)>>16)) ;
  packetBuffer[6]  = highByte(word(lNodeAddress & 0xffff)) ;
  packetBuffer[7]  = lowByte(word(lNodeAddress & 0xffff)) ;
*/  
  packetBuffer[8]  = 52;   // command
  packetBuffer[9]  = MAX_VALVE;    //  valves to follow

  for ( int i  = 0 ;  i < MAX_VALVE ; i++ ){
    j = i * 6 ;
    packetBuffer[10+j] = highByte(vvalve[i].lTTG) ;
    packetBuffer[11+j] = lowByte(vvalve[i].lTTG) ;
    packetBuffer[12+j] = highByte(vvalve[i].lATTG) ;
    packetBuffer[13+j] = lowByte(vvalve[i].lATTG) ;
    packetBuffer[14+j] = evalve[i].Node ;
    packetBuffer[15+j] = evalve[i].Valve ;
  }

  ctrludp.beginPacket(address, ghks.RemotePortCtrl);      // Send control data to the remote port - Broadcast ???
  ctrludp.write(packetBuffer, 10+(MAX_VALVE*6));
  ctrludp.endPacket();
}


unsigned long processCtrlUDPpacket(long lSize){
int i , j ;  
unsigned long highWord ;
unsigned long lowWord ;
unsigned long lAddress ;
byte packetBuffer[10+(MAX_VALVE*6)];           //buffer to hold incomming packets  

//  Serial.println(F("Process Ctrl Packet "));
  memset(packetBuffer, 0, 10+(MAX_VALVE*6));
  ctrludp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  highWord = word(packetBuffer[0], packetBuffer[1]);
  lowWord = word(packetBuffer[2], packetBuffer[3]);
  lAddress = highWord << 16 | lowWord;

  if (( ghks.lNodeAddress == lAddress) || ( lAddress == 0xffffffff )){ // is our address or a braodcast
      switch(packetBuffer[8] ){ // command byte
        case 52:
          j = packetBuffer[9] ; // number of valves to be read
          for (i = 0 ; i < MAX_VALVE && ( i < j ) ; i++ ){
            if (( packetBuffer[14+(i*6)] == ghks.lNodeAddress ) && ( packetBuffer[15+(i*6)] == i )){
              lowWord = word(packetBuffer[10+(i*6)], packetBuffer[11+(i*6)]);  // recieve all the ttgs for the valves
              vvalve[i].lTTG = lowWord ;
              highWord = word(packetBuffer[12+(i*6)], packetBuffer[13+(i*6)]);  // recieve all the ttgs for the valves
              vvalve[i].lATTG = highWord ;
            }
          }
        break;
      }
  }
  while (ctrludp.available()){  // clean out the rest of the packet and dump overboard
    ctrludp.read(packetBuffer, sizeof(packetBuffer));  
  }
}

