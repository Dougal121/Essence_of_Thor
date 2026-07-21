
void BackInTheBoxMemory(void){
  uint8_t i , j ;
  Serial.println(F("*** BackInTheBoxMemory Called ***"));  

//  ResetLoRaParams();
  
  ghks.lProgMethod = 1 ;
  ghks.cpufreq = 240 ;
  ghks.displaytimer = -1 ;
  ghks.ValveLogOptions = 0x82 ;
  ghks.meshChannel = 1 ; 
  ghks.iUseNetwork = 3 ;

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
    sprintf(pn.ph[i].Description , "Prog %2d\0" ,i+1 ) ;
    pn.ph[i].WDays = 0 ;
    for (j = 0 ; j < MAX_STARTS  ; j++ ) {   // NEW PROGRAMMING    ------   Clear all these
      if (( j == 0 ) && ( i == 0 )) {
        pn.ph[i].StartTime[j] = 800 ;        
      }else{
        pn.ph[i].StartTime[j] = -1 ;   // off     
      }
    }  
  }  
  ResetValveInfo(true);

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
      efert[i].BoardBit = 0x70 + i ;  // board 2
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
       efilter[i].BoardBit[j] = 0x80 + j ; // board 7 plus the offset for the bit
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
  ResetBoardInfo();
  sprintf(ghks.nssid,"************\0");  // put your default credentials in here if you wish
  sprintf(ghks.npassword,"********\0");  // put your default credentials in here if you wish
  

  sprintf(ghks.NodeName,"ESP32_%X\0",(uint32_t)chipid) ;

//  sprintf(ghks.NodeName,"ESP32_%02X%02X%02X%02X", (uint8_t)(chipid >> 24), (uint8_t)(chipid >> 16), (uint8_t)(chipid >> 8), (uint8_t)(chipid));

//  sprintf(ghks.cssid,"Project_THOR_%08X\0", chipid );
  sprintf(ghks.cssid,"Sanidi_%08X\0",chipid);
  sprintf(ghks.cpassword,"\0");
    
  ghks.lNodeAddress = (uint8_t) ((chipid >> 40) ^ (chipid >> 32) ^ (chipid >> 24) ^ (chipid >> 16) ^ (chipid >>  8) ^  chipid) & 0xff;
  if (( ghks.lNodeAddress == 0 ) || ( ghks.lNodeAddress == 255 ))
    ghks.lNodeAddress = 1 ; 

//  ghks.lNodeAddress = ( chipid & 0x7f ) + 1 ;
  ghks.lPulseTime = 20 ;
  ghks.lMaxDisplayValve = MAX_VALVE ;
//  sprintf(ghks.timeServer ,"au.pool.ntp.org\0"); 
  ghks.AutoOff_t = 0 ;
  ghks.localPort = 123 ;       // NTP port
  ghks.localPortCtrl = 8089 ;   // therse two need to be the same
  ghks.RemotePortCtrl= 8089 ;
  ghks.lVersion = MYVER ;
  ghks.SelfReBoot = 10080 ;
  ghks.lRebootTimeDay = 600 ;
  ghks.lDisplayOptions = 3 ;
  
  ghks.gmtOffset_sec = 10*3600; 
  ghks.daylightOffset_sec = 3600; 
  strncpy(ghks.timeServer, "au.pool.ntp.org\0", sizeof(ghks.timeServer));
  ghks.timeServer[sizeof(ghks.timeServer) - 1] = '\0';  // Ensure null termination
  strncpy(ghks.timePOSIXZone, "AEST-10AEDT,M10.1.0,M4.1.0/3\0", sizeof(ghks.timePOSIXZone));
  ghks.timePOSIXZone[sizeof(ghks.timePOSIXZone) - 1] = '\0';  // Ensure null termination
  
  ghks.SolPulsePower = 0 ;
  ghks.SolContPower = 0 ;
  
  ghks.RCIP = IPAddress(0,0,0,1);//  0 disables    1 local client net   2 SoftAP net   x.x.x.x  actual address  x.x.x.255 broadcast
/*  ghks.RCIP[0] = 0 ;
  ghks.RCIP[1] = 0 ; 
  ghks.RCIP[2] = 0 ;
  ghks.RCIP[3] = 1 ;  //  0 disables    1 local client net   2 SoftAP net   x.x.x.x  actual address  x.x.x.255 broadcast
  */
  ghks.lNetworkOptions = 0 ;     // DHCP 
  ghks.IPStatic = IPAddress(192,168,0,123);
/*  ghks.IPStatic[0] = 192 ;
  ghks.IPStatic[1] = 168 ;
  ghks.IPStatic[2] = 0 ;
  ghks.IPStatic[3] = 123 ;*/

  ghks.IPGateway = IPAddress(192,168,0,1);
/*  ghks.IPGateway[0] = 192 ;
  ghks.IPGateway[1] = 168 ;
  ghks.IPGateway[2] = 0 ;
  ghks.IPGateway[3] = 1 ;
*/
//  ghks.IPDNS = ghks.IPGateway ;
  ghks.IPDNS = IPAddress(8,8,4,4);
/*  ghks.IPDNS[0] = 8 ;
  ghks.IPDNS[1] = 8 ;
  ghks.IPDNS[2] = 4 ;
  ghks.IPDNS[3] = 4 ;
*/
  ghks.IPMask = IPAddress(255,255,255,0);
/*  ghks.IPMask[0] = 255 ;
  ghks.IPMask[1] = 255 ;
  ghks.IPMask[2] = 255 ;
  ghks.IPMask[3] = 0 ;
*/  
  ghks.meshChannel = 6 ;
  ResetLocalIO();
  ResetADCCalInfo();
  ResetSMTPInfo();
}

void ResetLocalIO(){
  for ( int i = 0 ; i < MAX_LOCAL_IO ; i++ ) {
    elocal.IOPin[i] = -1 ; // set to nothing 
#if defined(ESP32)    
    switch (i){
      case 0:
        elocal.IOPin[i] = 12 ;
      break;  
      case 1:
        elocal.IOPin[i] = 13 ;
      break;
      case 2:
        elocal.IOPin[i] = 2 ;
      break;
      case 3:
        elocal.IOPin[i] = 17 ;
      break;
      case 4:
        elocal.IOPin[i] = 23 ;
      break;
      case 5:
        elocal.IOPin[i] = 25 ;
      break;
      case 7:
        elocal.IOPin[i] = 34 ;
      break;
      case 8:
        elocal.IOPin[i] = 35 ;
      break;
      case 9:
        elocal.IOPin[i] = 36 ;
      break;
      case 10:
        elocal.IOPin[i] = 39 ;
      break;
    } 
    
#elif defined(ESP8266)    
    if (( i > 5 ) && ( i < 12 )){
      if ( i == 6 ) {
        elocal.IOPin[i] = 16 ;   // map out the memory bus pins by default            
      }else{
        elocal.IOPin[i] = 17 ;   // map out the memory bus pins by default    
      }
    }else{
      elocal.IOPin[i] = i ; 
    }
#endif           
  }
}

void ResetBoardInfo(){
  for (int i = 0 ; i < MAX_BOARDS ; i++) {
    if ( i < 8 ) {
      eboard[i].Address = 0x20 + i ;
    }else{
      eboard[i].Address = 0x30 + i ;      
    }
    for (int j = 0 ; j < MAX_EXTRA_BOARD_BYTES ; j++) {
      eboard[i].Extra[j] = 0 ;    // set these all to zero initally 
    }
    if ( i < 5 ) {
      eboard[i].Type = 1 ;       // MCP      
      eboard[i].Translate = 1 ;  // Use translation for this card
    }else{
      eboard[i].Type = 0 ;   // set them all to 0 - PCF8574
      if (i == 5 ){
        eboard[i].Type = 4 ;   // HC595
        eboard[i].Address = 0 ;
      }
      if (i == 6 ){
        eboard[i].Type = 5 ;   // RTU Modbus
        eboard[i].Address = 2 ; 
      }
      eboard[i].Translate = 0 ;
    }
    if ( i == 0x0c ){
      eboard[i].Type = 3 ;  // none as display is present at 0x3C
    }
    if ( i == 0x0f ){
      eboard[i].Type = 2 ;  // Configure Local IO
    }
  }
}

void ResetValveInfo(bool bDoubleActing){
  for (int i = 0 ; i < MAX_VALVE  ; i++ ) {   // setup the defaults in the 3 volitile structs that support data comin back from eeprom
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
    if (bDoubleActing){
      if ( i < 8 ) {
        evalve[i].OnCoilBoardBit = i*2  ;
        evalve[i].OffCoilBoardBit = (i*2) + 1 ;
      }else{
        evalve[i].OnCoilBoardBit = 0x10 + ((i-8) * 2 ) ;
        evalve[i].OffCoilBoardBit = 0x10 + ((i-8) * 2 + 1 ) ;                
      }
    }else{
        evalve[i].OnCoilBoardBit = i  ;
        evalve[i].OffCoilBoardBit = i ;
    }
    evalve[i].Fertigate = 0 ;
    evalve[i].bEnable = true ;
    evalve[i].Flowrate = 1.0 ;
   
    evalve[i].FeedbackBoardBit = 0xa0 + i ;  // board 10
    evalve[i].OnOffPolPulse = 0x44 ;
    
    evalve[i].Filter = 0 ;
    evalve[i].Node = 0 ;
    evalve[i].Valve = 0 ;
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

    ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
//    ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
    ghks.gmtOffset_sec = constrain(ghks.gmtOffset_sec , -12*3600 , 12*3600 );
    ghks.daylightOffset_sec = constrain(ghks.daylightOffset_sec , -2*3600 , 2*3600 );
    ghks.localPort = constrain(ghks.localPort,1,65535);
    ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    if ( ghks.iUseNetwork > 7 ){
      ghks.iUseNetwork = 7 ;
    }
    ghks.iUseNetwork = constrain(ghks.iUseNetwork ,0,7);   
    ghks.iFreq = constrain(ghks.iFreq,9150,9285);
    ghks.lPulseTime = constrain(ghks.lPulseTime,10,255);
    
    if ( year(ghks.AutoOff_t) < 2000 ){
       ghks.AutoOff_t = now();
    }
    ghks.lProgMethod = constrain(ghks.lProgMethod,0,1);
    ghks.iSpread = constrain(ghks.iSpread,5,12);
    ghks.iTXPower = constrain(ghks.iTXPower,2,20);
    
    ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,255);
    if ( ghks.lMaxDisplayValve <= 0 ){
      ghks.lMaxDisplayValve = MAX_VALVE ;
    }
    ghks.lMaxDisplayValve = constrain(ghks.lMaxDisplayValve,2,MAX_VALVE);

    if (ghks.lVersion  == MYVER_NEW){
      eeAddress = PROG_BASE_NEW ;  // 192 which is 48 * sizeof(float)      
    }else{
      eeAddress = PROG_BASE ;  // 192 which is 48 * sizeof(float)
    }
/*    
    if ( isnan(ghks.ADC_Cal_Ofs) || isinf(ghks.ADC_Cal_Ofs) ){
      ghks.ADC_Cal_Ofs = 0.0 ;
    }
    if ( isnan(ghks.ADC_Cal_Mul) || isinf(ghks.ADC_Cal_Mul) ){
      ghks.ADC_Cal_Mul = 1.0 ;
    }
    ghks.ADC_Input_PIN1 = constrain(ghks.ADC_Input_PIN1,-1,MaxPinPort);       // 
    ghks.ADC_Input_PIN2 = constrain(ghks.ADC_Input_PIN2,-1,MaxPinPort);       //
*/
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
    EEPROM.get(eeAddress,SMTP);
    eeAddress += sizeof(SMTP) ;
    EEPROM.get(eeAddress,adcs);
    eeAddress += sizeof(adcs) ;
    
    Serial.println("Final VPFF EEPROM adress " +String(eeAddress));   
//    sprintf(ghks.nssid,"TP-LINK_52FC8C\0");    // dougals office
//    sprintf(ghks.npassword,"0052FC8C\0");

  }else{
    ghks.lVersion  =  MYVER_NEW ;
    EEPROM.put(0,ghks);
    eeAddress = sizeof(ghks) ;
    Serial.println("write - ghks structure size " +String(eeAddress));   

    eeAddress = PROG_BASE_NEW ;
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
    EEPROM.put(eeAddress,SMTP);
    eeAddress += sizeof(SMTP) ;
    EEPROM.put(eeAddress,adcs);
    eeAddress += sizeof(adcs) ;
    
    Serial.println("Final EEPROM Save adress " +String(eeAddress));   

    EEPROM.commit();                                                       // save changes in one go ???
    bSaveReq = 0 ;
  }
}

