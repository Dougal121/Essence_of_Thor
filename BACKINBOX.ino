void BackInTheBoxMemory(){
  uint8_t i , j ;

  ghks.lProgMethod = 1 ;
  for (i = 0 ; i < MAX_SHIFTS  ; i++ ) {   // NEW PROGRAMMING   ------   Clear all these
    pn.sh[i].Program = 0 ;
    if ( i == 0 ){
      pn.sh[i].RunTime = 120 ;          
      pn.sh[i].ValveNo = 1 ;
    }else{
      pn.sh[i].RunTime = 0 ;    
      pn.sh[i].ValveNo = 0 ;
    }
  }  
  for (i = 0 ; i < MAX_PROGRAM_HEADER  ; i++ ) {   // Clear all these
    sprintf(pn.ph[i].Description , "Prog %2d\0" ,i ) ;
    pn.ph[i].WDays = 0 ;
    for (j = 0 ; j < MAX_STARTS  ; j++ ) {   // NEW PROGRAMMING    ------   Clear all these
      if (( j == 0 ) && ( i == 0 )) {
        pn.ph[i].StartTime[j] = 800 ;        
      }else{
        pn.ph[i].StartTime[j] = -1 ;   // off     
      }
    }  
  }  
  
  for (i = 0 ; i < MAX_VALVE  ; i++ ) {   // setup the defaults in the 3 volitile structs that support data comin back from eeprom
    sprintf(evalve[i].description,"Valve%02d",i+1);   
    if ( i == 2 ){
        evalve[i].TypeMaster = 0 ;      
//      evalve[i].TypeMaster = 0x80 ;
    }else{
/*      if (( i == 0 ) || ( i == 1 ) ) {
        evalve[i].TypeMaster = 0x43 ;              
      }else{  */
//        evalve[i].TypeMaster = 3 ;      
        evalve[i].TypeMaster = 0 ;      
//      }
    }
//    evalve[i].lDTTG = 60 + ( 100 * i ) ;
    evalve[i].lDTTG = 120 ;
/*    
    if ( i < 6 ) {
      evalve[i].OnCoilBoardBit = i*2  ;
      evalve[i].OffCoilBoardBit = (i*2) + 1 ;
      evalve[i].Fertigate = 1 ;
      evalve[i].bEnable = true ;
      evalve[i].Flowrate = 1.0 ;
    }else{
      evalve[i].Fertigate = 0 ;
      evalve[i].bEnable = false ;
      evalve[i].Flowrate = 0 ;
      evalve[i].OnCoilBoardBit = 0x50 + ((i-6) % 16 ) ;
      evalve[i].OffCoilBoardBit = 0x50 + ((i-6) % 16 ) ;        
    }
*/    
    if ( i < 8 ) {
      evalve[i].OnCoilBoardBit = i*2  ;
      evalve[i].OffCoilBoardBit = (i*2) + 1 ;
    }else{
      evalve[i].OnCoilBoardBit = 0x10 + ((i-8) * 2 ) ;
      evalve[i].OffCoilBoardBit = 0x10 + ((i-8) * 2 + 1 ) ;                
    }
    evalve[i].Fertigate = 0 ;
    evalve[i].bEnable = true ;
    evalve[i].Flowrate = 1.0 ;
   
    evalve[i].FeedbackBoardBit = 0xa0 + i ;  // board 10

    evalve[i].Filter = 0 ;
    evalve[i].Node = 0 ;
    evalve[i].Valve = 0 ;
  }

  for (i = 0 ; i < MAX_FERT ; i++ ) {
    sprintf(efert[i].description,"Tank%02d",i+1);   
    efert[i].AType = 0x10 ;  
    efert[i].CurrentQty = 800.0  ;   // - ( 1.0 * i ) ;
    efert[i].MaxQty = 800.0 ;         // - (2.0 * i ) ;
    efert[i].OnTime = 1 ;
    efert[i].BaseFlow = 1.0 ;
    efert[i].PumpRate = 0.1 ;
//    if ( i < 2 ){
      efert[i].DaysEnable = 0xff ;
      efert[i].BoardBit = 0x20 + i ;  // board 2
      efert[i].OffTime = 15 ;  
/*    }else{
      efert[i].DaysEnable = 0x0 ;
      efert[i].BoardBit = 0xe0 | i ;      
      efert[i].OffTime = 240 ;  
    }  */
  }

  for (i = 0 ; i < MAX_FILTER ; i++ ) {
    efilter[i].Enable = false ;
    efilter[i].Canisters = 3 ;    
    efilter[i].bPol = 0xff ;
    for ( j = 0 ; j < MAX_CANISTERS ; j++){
       efilter[i].BoardBit[j] = 0x70 + j ; // board 7 plus the offset for the bit
    }
    efilter[i].WashTime = 180 ;
    efilter[i].WashDelay = 3600 ;
//    Serial.println(String(efilter[i].WashDelay));

  }
  
  for (i = 0 ; i < MAX_PROGRAM ; i++ ){ 
    for ( j = 0 ; j < MAX_VALVE ; j++ ){
//      vp[j].p[i].starttime = ( j * 100 ) + (i * 15 ) ;   
//      vp[j].p[i].runtime = 15 * j ;   
      vp[j].p[i].starttime = 800 ;   
      vp[j].p[i].runtime = 480 ;   
/*      if (( i == 0 ) && ( j == 1 )){
        vp[j].p[i].wdays = 0xff ;   
      }else{
        vp[j].p[i].wdays = 0x00 ;   
      }  */
      vp[j].p[i].wdays = 0x00 ;   
    }
  }

  for (i = 0 ; i < MAX_BOARDS ; i++) {
    if ( i < 5 ) {
      eboard[i].Type = 1 ;       // MCP      
      eboard[i].Translate = 1 ;  // Use translation for this card
    }else{
      eboard[i].Type = 0 ;   // set them all to 0 - PCF8574
      eboard[i].Translate = 0 ;
    }
    if ( i < 8 ) {
      eboard[i].Address = 0x20 + i ;
    }else{
      eboard[i].Address = 0x30 + i ;      
    }
    if ( i == 0x0c ){
      eboard[i].Type = 3 ;  // none as display is present at 0x3C
    }
  }
  sprintf(ghks.nssid,"************\0");  // put your default credentials in here if you wish
  sprintf(ghks.npassword,"********\0");  // put your default credentials in here if you wish
  

  sprintf(ghks.NodeName,"Prickle Patch\0") ;

  sprintf(ghks.cpassword,"\0");
  
  ghks.fTimeZone = 10.0 ;
  ghks.lNodeAddress = ESP.getChipId() & 0xff ;
  ghks.lPulseTime = 20 ;
  ghks.lMaxDisplayValve = MAX_VALVE ;
  sprintf(ghks.timeServer ,"au.pool.ntp.org\0"); 
  ghks.AutoOff_t = 0 ;
  ghks.localPortCtrl = 8088 ;
  ghks.RemotePortCtrl= 8089 ;
  ghks.lVersion = MYVER ;
  
  ghks.RCIP[0] = 192 ;
  ghks.RCIP[1] = 168 ; 
  ghks.RCIP[2] = 2 ;
  ghks.RCIP[3] = 255 ;
  
  ghks.lNetworkOptions = 0 ;     // DHCP 
  ghks.IPStatic[0] = 192 ;
  ghks.IPStatic[1] = 168 ;
  ghks.IPStatic[2] = 0 ;
  ghks.IPStatic[3] = 123 ;

  ghks.IPGateway[0] = 192 ;
  ghks.IPGateway[1] = 168 ;
  ghks.IPGateway[2] = 0 ;
  ghks.IPGateway[3] = 1 ;

  ghks.IPDNS = ghks.IPGateway ;

  ghks.IPMask[0] = 255 ;
  ghks.IPMask[1] = 255 ;
  ghks.IPMask[2] = 255 ;
  ghks.IPMask[3] = 0 ;

  for ( i = 0 ; i < 16 ; i++ ) {
    if (( i > 5 ) && ( i < 12 )){
      if ( i == 6 ) {
        elocal.IOPin[i] = 16 ;   // map out the memory bus pins by default            
      }else{
        elocal.IOPin[i] = 17 ;   // map out the memory bus pins by default    
      }
    }else{
      elocal.IOPin[i] = i ; 
    }
  }
}


void LoadParamsFromEEPROM(bool bLoad){
long lTmp ;  
int i ;
int j ;
int bofs ,ofs ;
int eeAddress ;

  if ( bLoad ) {
    EEPROM.get(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("read - ghks structure size " +String(eeAddress));   

    ghks.lPulseTime = constrain(ghks.lPulseTime,10,1000);
    ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
    ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
    ghks.localPort = constrain(ghks.localPort,1,65535);
    ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    if ( year(ghks.AutoOff_t) < 2000 ){
       ghks.AutoOff_t = now();
    }
    ghks.lProgMethod = constrain(ghks.lProgMethod,0,1);
    ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,1);
    if ( ghks.lMaxDisplayValve <= 0 ){
      ghks.lMaxDisplayValve = MAX_VALVE ;
    }
    ghks.lMaxDisplayValve = constrain(ghks.lMaxDisplayValve,2,MAX_VALVE);

    eeAddress = PROG_BASE ;  // 192 which is 48 * sizeof(float)
    EEPROM.get(eeAddress,evalve);
    eeAddress += sizeof(evalve) ;
    EEPROM.get(eeAddress,vp);
    eeAddress += sizeof(vp) ;
    efertAddress = eeAddress ;
    EEPROM.get(eeAddress,efert);
    eeAddress += sizeof(efert) ;
    EEPROM.get(eeAddress,efilter);
    eeAddress += sizeof(efilter) ;
    EEPROM.get(eeAddress,eboard);
    eeAddress += sizeof(eboard) ;
    EEPROM.get(eeAddress,elocal);
    eeAddress += sizeof(elocal) ;
    EEPROM.get(eeAddress,pn);
    eeAddress += sizeof(pn) ;
    
    Serial.println("Final VPFF EEPROM adress " +String(eeAddress));   
    
  }else{
    ghks.lVersion  = MYVER ;
    EEPROM.put(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("write - ghks structure size " +String(eeAddress));   

    eeAddress = PROG_BASE ;
    EEPROM.put(eeAddress,evalve);
    eeAddress += sizeof(evalve) ;
    EEPROM.put(eeAddress,vp);
    eeAddress += sizeof(vp) ;    
    EEPROM.put(eeAddress,efert);
    eeAddress += sizeof(efert) ;
    EEPROM.put(eeAddress,efilter);
    eeAddress += sizeof(efilter) ;
    EEPROM.put(eeAddress,eboard);
    eeAddress += sizeof(eboard) ;
    EEPROM.put(eeAddress,elocal);
    eeAddress += sizeof(elocal) ;
    EEPROM.put(eeAddress,pn);
    eeAddress += sizeof(pn) ;
    
    Serial.println("Final EEPROM Save adress " +String(eeAddress));   

    EEPROM.commit();                                                       // save changes in one go ???
    bSaveReq = 0 ;
  }
}

