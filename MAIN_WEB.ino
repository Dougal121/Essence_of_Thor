void SerialOutParams(){
String message ;
   
  message = "Web Request URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  Serial.println(message);
  message = "";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  Serial.println(message);
}

void SendHTTPHeader(){
  String strTmp = "" ;
  server.sendHeader(F("Server"),F("ESP8266-on-ice"),false);
  server.sendHeader(F("X-Powered-by"),F("Dougal-1.0"),false);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  String message = F("<!DOCTYPE HTML>\r\n");
  message += "<head><title>Team Trouble - Irrigation Controler " + String(Toleo) + "</title>\r\n";
  message += F("<meta name=viewport content='width=320, auto inital-scale=1'>\r\n");
  message += F("</head><body><html lang='en'><center><h3>\r\n");   
  if ( ( ghks.ADC_Alarm_Mode & 0x80 ) != 0 ){
    strTmp = String(ADC_Value) + " " + String(ghks.ADC_Unit) ;
  }else{
    strTmp = "" ;
  }
  message += String(WiFi.RSSI()) + " (dBm) <a title='click for home / refresh' href='/'>"+String(ghks.NodeName)+"</a> " + strTmp + "</h3>\r\n";
  server.sendContent(message) ;         
}



void SendHTTPPageFooter(){
String message =  F("<br><a href='/?command=3'>Valve Setup Page 1</a>.. <a href='/?command=4'>Valve Setup Page 2</a><br>\r\n") ;
  message += F("<a href='/fert'>Fertigation Control Page 1</a>.. <a href='/fert?command=5'>Fertigation Setup Page 2</a> <br> <a href='/filt'>Filter Setup</a><br>\r\n")  ;   
  message += F("<br><a href='/?command=1'>Load Parameters from EEPROM</a><br><br><a href='/?command=667'>Reset Memory to Factory Default</a><br>\r\n");
  message += F("<a href='/?command=665'>Sync UTP Time</a><br><a href='/stime'>Manual Time Set</a><br><a href='/scan'>I2C Scan</a><br>\r\n");
  message += F("<a href='/iosc'>Database I/O Scan</a><br><a href='/iolocal'>Local I/O Mapping</a><br>\r\n") ;     
  server.sendContent(message) ;  
  message = "" ;       

  message += "<a href='/?reboot=" + String(lRebootCode) + "'>Reboot</a><br>\r\n";
//  message += F("<a href='/?command=668'>Save Fert Current QTY</a><br>\r\n");
  message += F("<a href='/eeprom'>EEPROM Memory Contents</a><br>\r\n");
  message += F("<a href='/rtceeprom'>RTC EEPROM Memory Contents (Valve Log Area)</a><br>\r\n");
  message += F("<a href='/valvelog'>Valve Log</a><br>\r\n");
  message += F("<a href='/setup'>Node Setup</a><br>\r\n");
  message += F("<a href='/email'>Email Setup</a><br>\r\n");
  message += F("<a href='/info'>Node Infomation</a><br>\r\n");
  message += F("<a href='/btest'>Relay Board Test</a><br>\r\n");
  message += F("<a href='/vsss'>view volatile memory structures</a><br>\r\n");
  message += F("<a href='/fertque'>view fertigation data que</a><br>\r\n");
  server.sendContent(message) ;  
  message = "" ;       
  
  if (bPrevConnectionStatus) {
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
  }else{
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);
  }
  message += "<a href='http://" + String(buff) + ":81/update'>OTA Firmware Update</a><br>\r\n";  
  message += F("<a href='https://github.com/Dougal121/Essence_of_Thor'>Source at GitHub</a><br>\r\n");  
  message += "<a href='http://" + String(buff) + "/backup'>Backup / Restore Settings</a><br><br>\r\n";  
  snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
  message += "Computer Uptime <b>"+String(buff)+"</b> (day:hr:min) <br>\r\n" ;
  message += F("</body></html>");
  server.sendContent(message) ;  
  message = "" ;       
}


void handleNotFound(){
  String message = F("Seriously - No way DUDE\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
//  Serial.print(message);
}

 
void handleRoot() {
  boolean currentLineIsBlank = true;
  tmElements_t tm;
  long  i = 0 ;
  int ii  ;
  int iProgNum = 0;
  int j ;
  int k , kk , iTmp ;
  boolean bExtraValve = false ;
  uint8_t iPage = 0 ;
  int iAType = 0 ;
  boolean bDefault = true ;
//  int td[6];
  long lTmp ; 
  String MyCheck , MyColor , MyNum , MyCheck2 , MyCheck3 ;
  byte mac[6];
  String message ;

//  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
//    bSaveReq = 1 ;
    i = String(server.argName(j)).indexOf("prgm");
    if (i != -1){  // look at the program number else it stays zero
      iProgNum = String(server.arg(j)).toInt() ;  
      if (( iProgNum < 0 )){
        iProgNum = 0 ;
      }
      if ( iProgNum >= MAX_PROGRAM ){
        iProgNum = MAX_PROGRAM - 1 ;        
      }
    }
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 1:  // load values
          LoadParamsFromEEPROM(true);
//          Serial.println("Load from EEPROM");
        break;
        case 2: // Save values
          LoadParamsFromEEPROM(false);
//          Serial.println("Save to EEPROM");
        break;
        case 3: // valve setup
          bExtraValve = true ;  // makes the option active
          iPage = 1 ;
        break;
        case 4: // 
          bExtraValve = true ;  // makes the option active
          iPage = 2 ;
        break;
        case 5: // Fertigation
          iPage = 2 ;
        break;
        case 8: //  Cold Reboot
          ESP.reset() ;
        break;
        case 9: //  Warm Reboot
          ESP.restart() ;
        break;
        case 0: // all stop
          for ( ii = 0 ; ii < ghks.lMaxDisplayValve ; ii++){         // handle all the valve control commands for any and all valves
            vvalve[ii].bOnOff = true ;                               // turn on to ensure a pulse is generated 
            vvalve[ii].lTTG = 0 ;                                    // reset this to zero 
          }          
        break;
        case 42:
          if ( iTestMode == -1 ) {
            iTestMode = 0 ;            
          }else{
            iTestMode = -1 ;            
          }
        break;
        case 667: // wipe the memory to factory default
          BackInTheBoxMemory();
        break;
        case 69: // wipe the memory to factory default
          BackInTheBoxMemory();
          LoadParamsFromEEPROM(false);
        break;
        case 665:
          bManSet = true ;
          sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour  
        break;
        case 668:
          ii = SaveCurrentQty(false);
//          Serial.println(String(ii) + " Fert records updated");
        break;
        case 998:
          ReadValveLogsFromEEPROM();
          iValveLogTTG = (ghks.ValveLogOptions & 0x1f) * 10 + 20 ; // read data drop eemprom and over right memory
        break;
        case 997:
          WriteValveLogsToEEPROM();
          iValveLogTTG = (ghks.ValveLogOptions & 0x1f) * 10 + 20 ; // write data from memory and over write eeprom data
        break;
        case 999:
          ZeroValveLogsMemory(15);
        break;           
        case 121:
          ResetSMTPInfo();
        break;
        case 669:
          bSentADCAlarmEmail = false ;
        break;
      }  
    }
    i = String(server.argName(j)).indexOf("reboot");
    if (i != -1){  // 
      if (( lRebootCode == String(server.arg(j)).toInt() ) && (lRebootCode>0 )){  // stop the phone browser being a dick and retry resetting !!!!
         IndicateReboot() ;        
      }
    }
    for ( ii = 0 ; ii < MAX_FILTER ; ii++){  // handle all the filter control arguments
      i = String(server.argName(j)).indexOf("fx" + String(ii));  // this is a parser cheat
      if (i != -1){  // 
        efilter[ii].Enable = 0 ; // wipe all the data - "shoot them all and let god sort them out"
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "d" );
      if (i != -1){  // wash delay
        efilter[ii].WashDelay = String(server.arg(j)).toInt() ;
        if ( efilter[ii].WashDelay < 0 ) {
          efilter[ii].WashDelay = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "t" );
      if (i != -1){  // wash time
        efilter[ii].WashTime = String(server.arg(j)).toInt() ;
        if ( efilter[ii].WashTime < 0 ) {
          efilter[ii].WashTime = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "n" );
      if (i != -1){  // canisters
        efilter[ii].Canisters = String(server.arg(j)).toInt() ;
        if ( efilter[ii].Canisters > 8 ){
          efilter[ii].Canisters = 8 ;
        }
        if ( efilter[ii].Canisters < 0 ){
          efilter[ii].Canisters = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f"+String(ii)+"p" );
      if (i != -1){  // polarity
        efilter[ii].bPol = 0xff & String(server.arg(j)).toInt()  ;
        if ( efilter[ii].bPol > 1 ) {
          efilter[ii].bPol = 1 ;
        }
        if ( efilter[ii].bPol < -1 ) {
          efilter[ii].bPol = -1 ;
        }
      }      
      i = String(server.argName(j)).indexOf("fe" + String(ii) );
      if (i != -1){  // polarity
        if ( String(server.arg(j)).length() == 2 ){
          efilter[ii].Enable = 1 ;            
        }
      }      
      for ( kk = 0 ; kk < MAX_CANISTERS ; kk++ ) {
        i = String(server.argName(j)).indexOf("f"+String(ii)+"a"+String(kk));
        if (i != -1){  // for each output
          efilter[ii].BoardBit[kk] = ( efilter[ii].BoardBit[kk] & 0x0f ) | (( 0x0f & String(server.arg(j)).toInt()) << 4 ) ;
        }  
        i = String(server.argName(j)).indexOf("f"+String(ii)+"b"+String(kk));
        if (i != -1){  // for each output
          efilter[ii].BoardBit[kk] = ( efilter[ii].BoardBit[kk] & 0xf0 ) | ( 0x0f & String(server.arg(j)).toInt()) ;
        }  
      }
            
    }
    for ( ii = 0 ; ii < MAX_FERT ; ii++){                             // handle all the fertigation control arguments
      i = String(server.argName(j)).indexOf("xnum" + String(ii));  // this is a parser cheat
      if (i != -1){  // 
        if ( iPage == 0 ){
          efert[ii].DaysEnable = 0 ; // wipe all the data - "shoot them all and let god sort them out"
        }
      }      
      i = String(server.argName(j)).indexOf("xdes" + String(ii));
      if (i != -1){  // Current quantity in system   ( Dont pump if < 0 )
        String(server.arg(j)).toCharArray(efert[ii].description , sizeof(efert[ii].description)) ;        
      }        
      i = String(server.argName(j)).indexOf("xaty" + String(ii));
      if (i != -1){  // 
        efert[ii].AType &= 0x0f ;
        efert[ii].AType |= ( String(server.arg(j)).toInt() << 4 ) ;
      }        
      i = String(server.argName(j)).indexOf("xatm" + String(ii));
      if (i != -1){  // 
        iAType = String(server.arg(j)).toInt()   ;
        iAType = constrain(iAType,0,MAX_FERT );
        efert[ii].AType &= 0xf0 ;
        efert[ii].AType |= ((uint8_t) iAType & 0x0f ) ;
      }        
/*      i = String(server.argName(j)).indexOf("xmty" + String(ii));
      if (i != -1){  // 
        efert[ii].AType = String(server.arg(j)).toInt()  ;
      }*/        
      i = String(server.argName(j)).indexOf("xcqt" + String(ii));
      if (i != -1){  // Current quantity in system   ( Dont pump if < 0 )
        efert[ii].CurrentQty = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xmqt" + String(ii));
      if (i != -1){  // Max quantity - used to reset qty
        efert[ii].MaxQty = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xont" + String(ii));
      if (i != -1){  // On time basis
        efert[ii].OnTime = String(server.arg(j)).toInt() ;
      }        
      i = String(server.argName(j)).indexOf("xoft" + String(ii));
      if (i != -1){  // Off time basis
        efert[ii].OffTime = String(server.arg(j)).toInt() ;
      }        
      i = String(server.argName(j)).indexOf("xbas" + String(ii));
      if (i != -1){  // pump base rate
        efert[ii].BaseFlow = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xpmr" + String(ii));
      if (i != -1){  // pump rate
        efert[ii].PumpRate = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xbba" + String(ii));
      if (i != -1){  // board address
        efert[ii].BoardBit = ( efert[ii].BoardBit & 0x0F ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
      }        
      i = String(server.argName(j)).indexOf("xbbb" + String(ii));
      if (i != -1){  // board bit
        efert[ii].BoardBit = ( efert[ii].BoardBit & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
      }
      for ( kk = 0 ; kk < 8 ; kk++ ) {
        i = String(server.argName(j)).indexOf("x"+String(ii)+"dw"+String(kk));
        if (i != -1){  //  
          if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
            efert[ii].DaysEnable |= ( 0x1 << kk )  ;
          }
        }                
      }              
    }
    for ( ii = 0 ; ii < ghks.lMaxDisplayValve ; ii++){  // handle all the valve control commands for any and all valves
      MyNum = String(ii) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }      
      i = String(server.argName(j)).indexOf("vfbt" + MyNum);
      if (i != -1){  // on valve control number
        evalve[ii].FeedbackBoardBit = ( evalve[ii].FeedbackBoardBit & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
        evalve[ii].Fertigate = 0 ;  // reset these at same time as on page 2 bet to the left
        evalve[ii].Filter = 0 ;
//        Serial.println("FeedbackBoardBit" + String(evalve[ii].FeedbackBoardBit) );  
      }        
      i = String(server.argName(j)).indexOf("vdes" + MyNum);
      if (i != -1){  // on valve control number
        String(server.arg(j)).toCharArray(evalve[ii].description , sizeof(evalve[ii].description)) ;        
      }        
      i = String(server.argName(j)).indexOf("vfad" + MyNum);
      if (i != -1){  // on valve address
        evalve[ii].FeedbackBoardBit = ( evalve[ii].FeedbackBoardBit & 0x0f ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
//        Serial.println("FeedbackBoardBit" + String(evalve[ii].FeedbackBoardBit) );  
      }        
      i = String(server.argName(j)).indexOf("nflo" + MyNum);
      if (i != -1){  // on valve address
        evalve[ii].Flowrate = String(server.arg(j)).toFloat() ;
//        Serial.println("Flowrate" + String(evalve[ii].Flowrate) );  
      }        
      i = String(server.argName(j)).indexOf("vmas" + MyNum);
      if (i != -1){  // on valve address
        evalve[ii].TypeMaster = 0x3f & String(server.arg(j)).toInt()   ;
//        Serial.println("TypeMaster C " + String(evalve[ii].TypeMaster) );  
      }        
      i = String(server.argName(j)).indexOf("vtya" + MyNum);
      if (i != -1){  // valve type / domestic ? 
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          evalve[ii].TypeMaster |= ( 0x200 )  ;
//          Serial.println("TypeMaster M " + String(evalve[ii].TypeMaster) );  
        }
      }                
      i = String(server.argName(j)).indexOf("vtyd" + MyNum);
      if (i != -1){  // valve type / domestic ? 
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          evalve[ii].TypeMaster |= ( 0x100 )  ;
//          Serial.println("TypeMaster M " + String(evalve[ii].TypeMaster) );  
        }
      }                
      i = String(server.argName(j)).indexOf("vtym" + MyNum);
      if (i != -1){  // valve type / master ? 
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          evalve[ii].TypeMaster |= ( 0x80 )  ;
//          Serial.println("TypeMaster M " + String(evalve[ii].TypeMaster) );  
        }
      }                
      i = String(server.argName(j)).indexOf("vtyf" + MyNum);
      if (i != -1){  // valve type / feedback 
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          evalve[ii].TypeMaster |= ( 0x40 )  ;
//          Serial.println("TypeMaster F " + String(evalve[ii].TypeMaster) );  
        }
      }                
      
      i = String(server.argName(j)).indexOf("vnum" + MyNum);    // not used ??? done at   "fvbt"
      if (i != -1){  // this is a cheat that allows check box resets
        if (!bExtraValve){
          evalve[ii].bEnable = false ;
        }
        if ( iPage == 2 ){
          evalve[ii].Fertigate = 0 ;
          evalve[ii].Filter = 0 ;
        }
      }        
      for ( kk = 0 ; kk < 8 ; kk++ ) {  // fertigation and prolly filters
        i = String(server.argName(j)).indexOf("v" + MyNum + "fg"+String(kk));
        if (i != -1){  //  
          if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
            evalve[ii].Fertigate |= ( 0x1 << kk )  ;
          }
        }
        i = String(server.argName(j)).indexOf("v" + MyNum + "fs"+String(kk));
        if (i != -1){  //  
          if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
            evalve[ii].Filter |= ( 0x1 << kk )  ;
          }
        }
      }
      i = String(server.argName(j)).indexOf("vben" + MyNum);
      if (i != -1){  // enable for automatic ie programs - does not effect manual
        if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
          evalve[ii].bEnable = true ;
        }
      }        
      i = String(server.argName(j)).indexOf("dttg" + MyNum);
      if (i != -1){  // this be the default TTG for when you press ON
        iTmp = String(server.arg(j)).toInt() ;
        iTmp = constrain(iTmp,0,MAX_MINUTES);
        evalve[ii].lDTTG = iTmp ;
      }        
      i = String(server.argName(j)).indexOf("vttg" + MyNum);
      if (i != -1){  // this is the manual TTG
        iTmp = String(server.arg(j)).toInt() ;
        iTmp = constrain(iTmp,0,MAX_MINUTES);
        vvalve[ii].lTTG = iTmp ;
      }        
      i = String(server.argName(j)).indexOf("vcon" + MyNum);
      if (i != -1){  // on valve control number
        evalve[ii].OnCoilBoardBit = ( evalve[ii].OnCoilBoardBit & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
        bExtraValve = true ; // keeps the option active
      }        
      i = String(server.argName(j)).indexOf("vpon" + MyNum);
      if (i != -1){  // on valve polatity and pulse time
        evalve[ii].OnOffPolPulse = ( evalve[ii].OnOffPolPulse & 0x8f ) | ( 0x70 & (( String(server.arg(j)).toInt()) << 4 ))  ;
//        Serial.println("OnOffPolPulse" + String(evalve[ii].OnOffPolPulse) );  
      }        
      i = String(server.argName(j)).indexOf("vppon" + MyNum);
      if (i != -1){  // on valve polatity and pulse time
        evalve[ii].OnOffPolPulse = ( evalve[ii].OnOffPolPulse & 0x7f ) | ( 0x80 & (( String(server.arg(j)).toInt()) << 7 ))  ;
//        Serial.println("OnOffPolPulse" + String(evalve[ii].OnOffPolPulse) );  
      }        
      i = String(server.argName(j)).indexOf("vaon" + MyNum);
      if (i != -1){  // on valve address
        evalve[ii].OnCoilBoardBit = ( evalve[ii].OnCoilBoardBit & 0x0f ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
//        Serial.println("OnCoilBoardBit" + String(evalve[ii].OnCoilBoardBit) );  
      }        
      i = String(server.argName(j)).indexOf("vcof" + MyNum);
      if (i != -1){  // off valve control number
        evalve[ii].OffCoilBoardBit = ( evalve[ii].OffCoilBoardBit & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
//        Serial.println("OffCoilBoardBit" + String(evalve[ii].OffCoilBoardBit) );  
        bExtraValve = true ;
      }        
      i = String(server.argName(j)).indexOf("vpof" + MyNum);
      if (i != -1){  // off valve polatity and pulse time
        evalve[ii].OnOffPolPulse = ( evalve[ii].OnOffPolPulse & 0xF8 ) | ( 0x07 &  String(server.arg(j)).toInt())  ;
//        Serial.println("OnOffPolPulse" + String(evalve[ii].OnOffPolPulse) );  
      }        
      i = String(server.argName(j)).indexOf("vppof" + MyNum);
      if (i != -1){  // off valve polatity and pulse time
        evalve[ii].OnOffPolPulse = ( evalve[ii].OnOffPolPulse & 0xF7 ) | ( 0x08 &  (String(server.arg(j)).toInt()<<3))  ;
//        Serial.println("OnOffPolPulse" + String(evalve[ii].OnOffPolPulse) );  
      }        
      i = String(server.argName(j)).indexOf("vaof" + MyNum);
      if (i != -1){  // off valve address
        evalve[ii].OffCoilBoardBit = ( evalve[ii].OffCoilBoardBit & 0x0f ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
//        Serial.println("OffCoilBoardBit" + String(evalve[ii].OffCoilBoardBit) );  
      }        
      i = String(server.argName(j)).indexOf("ncon" + MyNum);
      if (i != -1){  // Valve Control Node
        evalve[ii].Node =  String(server.arg(j)).toInt() ;
//        Serial.println("Remote Node Address" + String(evalve[ii].Node) );  
      }        
      
      i = String(server.argName(j)).indexOf("ncom" + MyNum);
      if (i != -1){  // Valve Control Node
        evalve[ii].Valve =  String(server.arg(j)).toInt() & 0x7f ;
      }     
      i = String(server.argName(j)).indexOf("ncul" + MyNum);
      if (i != -1){  // Valve Control Node
         if ( String(server.arg(j)).length() == 2 ){ // this relies on the one above coming first which it usu
            evalve[ii].Valve += 0x80  ;
         }
      }     
         
      i = String(server.argName(j)).indexOf("vsea" + MyNum);
      if (i != -1){  
        if ( String(server.arg(j)).toInt() == 0 ){
          vvalve[ii].bOnOff = true ;   // turn on to ensure a pulse is generated 
          vvalve[ii].lTTG = 0 ;     // reset this to zero 
        }else{
          vvalve[ii].bOnOff = false ;     // turn off to ensure a pulse is generated
          vvalve[ii].lTTG = evalve[ii].lDTTG ; // set this to the default
        }
      }        
      i = String(server.argName(j)).indexOf("vx" + MyNum);
      if (i != -1){  // 
        vp[ii].p[iProgNum].wdays = 0 ; // wipe all the data - "shoot them all and let god sort them out"
      }
      for ( k = 0 ; k < MAX_PROGRAM ; k++ ) {
        i = String(server.argName(j)).indexOf("v" + MyNum + "st"+String(k));
        if (i != -1){  
          lTmp = String(server.arg(j)).toInt() ;
          lTmp = constrain(lTmp,0,2359);
          vp[ii].p[k].starttime = lTmp ; 
        }        
        i = String(server.argName(j)).indexOf("v" + MyNum + "rt"+String(k));
        if (i != -1){  
          lTmp = String(server.arg(j)).toInt() ;
          lTmp = constrain(lTmp,0,10800);
          vp[ii].p[k].runtime = lTmp ; 
        }        
        for ( kk = 0 ; kk < 8 ; kk++ ) {  // day of week and maste enable
          i = String(server.argName(j)).indexOf("v" + MyNum + "dw"+String(k)+"day"+String(kk));
          if (i != -1){  // 
            if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
              vp[ii].p[k].wdays |= ( 0x1 << kk )  ;
            }
          }                
        }
      }
    }

    i = String(server.argName(j)).indexOf("stime");
    if (i != -1){  // 
      tm.Year = (String(server.arg(j)).substring(0,4).toInt()-1970) ;
      tm.Month =(String(server.arg(j)).substring(5,7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8,10).toInt()) ;
      tm.Hour =(String(server.arg(j)).substring(11,13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14,16).toInt()) ;
      tm.Second = 0 ;
      setTime(makeTime(tm));    
      if ( hasRTC ){
        tc.sec = second();     
        tc.min = minute();     
        tc.hour = hour();   
        tc.wday = dayOfWeek(makeTime(tm));            
        tc.mday = day();  
        tc.mon = month();   
        tc.year = year();       
        DS3231_set(tc);                       // set the RTC as well
        rtc_status = DS3231_get_sreg();       // get the status
        DS3231_set_sreg(rtc_status & 0x7f ) ; // clear the clock fail bit when you set the time
      }
    }        

    i = String(server.argName(j)).indexOf("dontp");
    if (i != -1){  // have a request to request a time update
      bDoTimeUpdate = true ;
    }


    i = String(server.argName(j)).indexOf("testb");
    if (i != -1){                                    // test board
     iTestBoard = String(server.arg(j)).toInt() ;   
     iTestBoard = constrain(iTestBoard,0,15);
    }
    i = String(server.argName(j)).indexOf("testt");
    if (i != -1){                                    // test time
     iTestTime = String(server.arg(j)).toInt() ;   
     iTestTime = constrain(iTestTime,3,60);
    }
    i = String(server.argName(j)).indexOf("tstinc");
    if (i != -1){                                    // test and increment
     iTestInc = String(server.arg(j)).toInt() ;   
     iTestInc = constrain(iTestInc,0,1);
    }
    i = String(server.argName(j)).indexOf("testc");
    if (i != -1){                                    // test coil number
     iTestCoil = String(server.arg(j)).toInt() ;   
     iTestCoil = constrain(iTestCoil,0,15);
    }
    
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######

  if ( bSaveReq != 0 ){
     message = F("<blink>");      
  }   
  message += F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>\r\n") ;     
  if ( bSaveReq != 0 ){
    message += F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>\r\n") ;     
  }
    
  if (ghks.lProgMethod == 0 ){    
    message += ("<a href='/prog'>Programs</a><br>\r\n") ;         
  }else{
    message += F("<a href='/prognew'>Programs</a><br>\r\n") ;             
  }

  snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
  if (ghks.fTimeZone > 0 ) {
    message += "<b>"+ String(buff) + " UTC +" + String(ghks.fTimeZone,1) ;   
  }else{
    message += "<b>"+ String(buff) + " UTC " + String(ghks.fTimeZone,1)  ;       
  }
  if ( year() < 2000 ) {
    message += F("  --- CLOCK NOT SET ---") ;
  }
  message += F("</b><br>\r\n") ;  
  if ( ghks.AutoOff_t > now() )  {
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t), second(ghks.AutoOff_t));
    message += F("<b><font color=red>Automation OFFLINE Untill \r\n") ;  
    message += String(buff) ; 
    message += F("</font></b><br>\r\n") ; 
  }else{
    if ( year() > 2000 ) {
      message += F("<b><font color=green>Automation ONLINE</font></b><br>\r\n") ;  
    }else{
      message += F("<b><font color=green>Automation OFFLINE Invalid time</font></b><br>\r\n") ;        
    }
  }
  server.sendContent(message);
  message = "" ;
    
  if (String(server.uri()).indexOf("stime")>0) {  // ################   SETUP TIME    #######################################
    bDefault = false ;
    snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(), month(), day() , hour(), minute());
    message = "<br><br><form method=post action=" + server.uri() + "><br>Set Current Time: <input type='text' name='stime' value='"+ String(buff) + "' size=12>\r\n";
    message += F("<input type='submit' value='SET'><br><br></form>\r\n");
    server.sendContent(message);
    message = "" ;
  }

  if (String(server.uri()).indexOf("btest")>0) {  // ################   TEST BOARD    #######################################
    bDefault = false ;
    if ( iTestMode == -1 ){
      MyColor = F("TEST") ;
    }else{
      MyColor = F("ABORT") ;
    }
    message = F("<br><br><b>Test Relay Board</b><br><table border=1 title='Test Coils and Relay Board'>\r\n");
    message += "<tr><th>Parameter</th><th>Value</th><th><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='42'><input type='submit' value='"+MyColor+"'></form></th></tr>\r\n";
    message += "<tr><td><form method=post action=" + server.uri() + ">Test Boad Number</td><td><input type='text' name='testb' value='"+ String(iTestBoard) + "' maxlength=2 size=4 align='center'>";
    message += F("</td><td><input type='submit' value='SET'></form></td></tr>\r\n");
    message += "<tr><td><form method=post action=" + server.uri() + ">Test Start/Coil Number</td><td><input type='text' name='testc' value='"+ String(iTestCoil) + "' maxlength=2 size=4 align='center'>";
    message += F("</td><td><input type='submit' value='SET'></form></td></tr>\r\n");   
    message += "<tr><td><form method=post action=" + server.uri() + ">Time between Coil Tests (s)</td><td><input type='text' name='testt' value='"+ String(iTestTime) + "' maxlength=2 size=4 align='center'>";
    message += F("</td><td><input type='submit' value='SET'></form></td></tr>\r\n");
    message += "</td><td><form method=post action=" + server.uri() + ">Test Mode</td><td><select name='tstinc'>" ;
    if (iTestInc == 1 ){
      message += F("<option value='0' SELECTED>0 - Increment After Coil Test"); 
      message += F("<option value='1'>1 - Test same coil"); 
    }else{
      message += F("<option value='0'>0 - Increment After Coil Test"); 
      message += F("<option value='1' SELECTED>1 - Test same coil"); 
    }
    message += F("</select></td><td><input type='submit' value='SET'></form></td></tr>\r\n");

    if ( iTestMode == -1 ){
      message += F("</td><td colspan=3 align='center' bgcolor='ltgreen'><b>Normal Operation</b></td></tr>\r\n");
    }else{
      message += "</td><td colspan=3 align='center' bgcolor='yellow'><b>Test In Progres - Coil "+String(iTestCoil)+"</b></td></tr>\r\n";  
    }
    message += F("</table>\r\n");
    server.sendContent(message);
    message = "" ;
  }
    
  
  if (String(server.uri()).indexOf("prog")>0) {  // #############################  program interface  #####################################
    bDefault = false ;
    message = "<br><form method=post action=" + server.uri() + ">\r\n" ;
    message += "Program: <select name='prgm'>" ;
    for ( j = 0 ; j < MAX_PROGRAM ; j++ ){
      if ( j == iProgNum ){
        MyColor = F("SELECTED");
      }else{
        MyColor = "" ;        
      }
      message += "<option value='"+String(j)+"' " + MyColor + " >" + String(j+1) ;
    }
    message += F("</select>\r\n<input type='submit' value='SET'></form><br>\r\n");
    message += F("<b>Valve Programs</b><br><table border=1 title='Valve Program'>\r\n");
    message += F("<tr><th rowspan=2>Valve</th>");
    j = iProgNum ;
//    for ( j = 0 ; j < MAX_PROGRAM ; j++ ){
      message += "<th colspan=8>Program " + String(j+1) + " - Days</th>";
      message +=F("<th rowspan=2>Start<br>HHMM</th><th rowspan=2>Run<br>(min)</th>");
//    }
    message += F("</tr><tr>");
//    for ( j = 0 ; j < MAX_PROGRAM ; j++ ){
      for ( i = 0 ; i < 8 ; i++ ){
        message += "<th>" + String(dayarray[i]) + "</th>" ;
      }
//    }
    message +=F("</tr>\r\n");   
    server.sendContent(message) ; // End of Table Header
    
    for ( i = 0 ; i < ghks.lMaxDisplayValve ; i++ ){   // MAX_VALVE
      MyNum = String(i) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }  
      switch (ProgramStatus(i)){
        case 0:
          MyColor = ""   ;  
        break;  
        case 1:
          MyColor = F("bgcolor=green")   ;  
        break;
        default:
          MyColor = F("bgcolor=red")   ;  
        break;
      }
      message =  "<tr><th "+String(MyColor)+">"+String(i+1)+" - "+evalve[i].description+"</th>" ;
      message += "<form method=post action=" + server.uri() + "><input type='hidden' name='prgm' value='" + String(iProgNum) + "'><input type='hidden' name='vx" + MyNum + "' value='" + MyNum + "'>" ;    
      j = iProgNum ;
//      for (j = 0 ; j < MAX_PROGRAM ; j++ ) {                   // lay down the days 
        for (k = 0 ; k < 8 ; k++){      
          MyColor =  ""   ;  
          if ( ( vp[i].p[j].wdays & (0x01 << k)) != 0 ){
            MyCheck = F("CHECKED")  ;    
            if ((k != 7 ) && ((vp[i].p[j].wdays & 0x80 ) == 0  ) && ((vp[i].p[j].runtime & 0xffff) > 0 )){ // selected with runtime but not enabled
              MyColor =  F("bgcolor=red")   ;  
            }          
            if ((k == 7 )  && ((vp[i].p[j].runtime & 0xffff) == 0 )){ // enabled but no runitem
              MyColor =  F("bgcolor=red")   ;  
            }          
          }else{
            MyCheck = ""    ;      
          }
          message += "<td "+String(MyColor)+"><input type='checkbox' name='v" + MyNum + "dw"+String(j)+"day"+String(k)+"' "+String(MyCheck)+ "></td>";    
        }
        message += "<td><input type='text' name='v" + MyNum + "st"+String(j)+"' value='" + String((vp[i].p[j].starttime)) + "' maxlength=4 size=2></td>" ;
        message += "<td><input type='text' name='v" + MyNum + "rt"+String(j)+"' value='" + String((vp[i].p[j].runtime)) + "' maxlength=5 size=2></td>" ;
//      }
/*      message += F("</tr><tr><td>Start Time</td>");
      for (j = 0 ; j < MAX_PROGRAM ; j++ ) {                   // lay down the start time
        message += "<td colspan=7><form method=post action=" + server.uri() + "><input type='text' name='v"+String(i)+"st"+String(j)+"' value='" + String((vp[i].p[j].starttime)) + "' maxlength=4 size=2></td><td align=center>"+String(vp[i].p[j].wdays,HEX ) +"</td><td><input type='submit' value='SET'></td></form>";    
      }
      message += F("</tr><tr><td>Run Time</td>");
      for (j = 0 ; j < MAX_PROGRAM ; j++ ) {                   // lay down the run time
        message += "<td colspan=8><form method=post action=" + server.uri() + "><input type='text' name='v"+String(i)+"rt"+String(j)+"' value='" + String((vp[i].p[j].runtime)) + "' maxlength=5 size=2></td><td><input type='submit' value='SET'></td></form>";          
      }*/
      message += F("<td><input type='submit' value='SET'></td></form></tr>\r\n"); 
      server.sendContent(message) ;
    }
    server.sendContent(F("</table><br>\r\n"));    
  }

  if (String(server.uri()).indexOf("filt")>0) {  // ########################   filter interface   ##################################################
    bDefault = false ;
    server.sendContent(F("<br><b>Filter Setup</b>\r\n"));
    message = F("<table border=1 title='Filter Setup'>\r\n") ;
    message += F("<tr><th rowspan=2>Bank</th><th rowspan=2>Enable</th><th rowspan=2>Wash Delay</th><th rowspan=2>Wash Time</th><th rowspan=2>Canisters</th>") ; 
    message += F("<th rowspan=2>Polarity</th><th colspan=17>Canister Control Relays</th></tr>\r\n") ;
    message += F("<tr>");
    for ( j = 0 ; ( j < MAX_CANISTERS )  ; j++){
      message += F("<th>Board</th><th>Bit</th>");
    }
    message += F("</tr>") ; 
    server.sendContent(message) ;
    message = "" ;
    for ( i = 0 ; i < MAX_FILTER ; i++){  // to the number of filter prgram in the chip
      if ( efilter[i].Enable == 0 ) {
        MyColor = F("bgcolor=red") ;
        MyCheck = F("")  ;    
      }else{
        MyColor = F("bgcolor=green") ;
        MyCheck = F("CHECKED")  ;    
      }
      message += "<tr><form method=post action=" + server.uri() + "><input type='hidden' name='fx"+String(i)+"' value='"+String(i)+"'><td align=center " + String(MyColor) + ">" + String(i+1) + "</td>" ;
      message += "<td align=center><input type='checkbox' name='fe"+String(i)+"' "+String(MyCheck)+"></td>" ;
//      Serial.println(String(efilter[i].WashDelay));
      message += "<td><input type='text' name='f"+String(i)+"d' value='" + String(efilter[i].WashDelay) + "' maxlength=6 size=6></td>" ; 
      message += "<td><input type='text' name='f"+String(i)+"t' value='" + String(efilter[i].WashTime) + "' maxlength=4 size=4></td>";         
      message += "<td><input type='text' name='f"+String(i)+"n' value='" + String(efilter[i].Canisters) + "' maxlength=1 size=4></td>";
      message += "<td><input type='text' name='f"+String(i)+"p' value='" + String(efilter[i].bPol) + "' maxlength=3 size=3></td>";
      server.sendContent(message) ;
      message = "" ;
      for ( j = 0 ; ( j < MAX_CANISTERS )  ; j++){
        message += "<td><input type='text' name='f" + String(i) + "a" + String(j) + "' value='" + String((efilter[i].BoardBit[j] & 0xf0)>>4) + "' maxlength=2 size=2></td>" ;
        message += "<td><input type='text' name='f" + String(i) + "b" + String(j) + "' value='" + String((efilter[i].BoardBit[j] & 0x0f)) + "' maxlength=2 size=2></td>";         
      }  
      message += F("<td><input type='submit' value='SET'></td></form></tr>\r\n") ;
      server.sendContent(message) ;
      message = "" ;
    }
    message = F("</table>\r\n") ;
    server.sendContent(message) ;
  }

  if (String(server.uri()).indexOf("fert")>0) {  // ##############################   fertigation interface  ########################################
    bDefault = false ;
    server.sendContent(F("<br><b>Fertigation Setup</b>\r\n"));
    message = F("<table border=1 title='Fertigation Setup'>\r\n") ;
    message += F("<tr><th rowspan=2>Tank</th>") ;
    if (iPage==0) {
      message += F("<th colspan=8>Enable</th>");
      message += F("<th>Current Qty</th><th>Max Qty</th><th rowspan=2>Description</th><th rowspan=2>Type</th><th rowspan=2>Master</th><th colspan=2 rowspan=2>.</th></tr>\r\n");
    }  
    if (iPage==2) {
      message += F("<th>On Time</th><th>Off time</th><th>Base Flow</th><th>Pump Rate</th><th colspan=2>Connection</th><th rowspan=2>.</th></tr>\r\n") ; 
    }
    message += F("<tr>") ;                   // second header row 
    if (iPage==0) {
      for ( j = 0 ; j < 8 ; j++ ){
        message += "<th>" + String(dayarray[j]) + "</th>" ;
      }
      message += F("<th>(l)</th><th>(l)</th></tr>\r\n");
    }
    if (iPage==2) {
      message += F("<th>(s)</th><th>(s)</th><th>(l/s)</th><th>(l/s)</th><th>Board</th><th>Bit</th></tr>\r\n") ; 
    }
    server.sendContent(message) ;
    message = "" ;
    
    for ( i = 0 ; i < MAX_FERT ; i++){
      if ((efert[i].DaysEnable & 0x80 ) == 0  ) {
        MyColor = F("bgcolor=red") ;
      }else{
        MyColor = F("bgcolor=green") ;
      }
      if (iPage==0) {
        message += "<tr><form method=post action=" + server.uri() + "><td align=center "+String(MyColor)+">"+String(i+1)+"</td>";           
      }
      if (iPage==2) {
        message += "<tr><form method=post action=" + server.uri() + "><td align=center "+String(MyColor)+">"+String(i+1)+" - "+String(efert[i].description)+"</td>";   
        message += F("<input type='hidden' name='command' value='5'>");
      }  
      message += "<input type='hidden' name='xnum"+String(i)+"' value='"+String(i)+"'>";
      if (iPage==0) {
        for (k = 0 ; k < 8 ; k++){      
          MyColor =  ""   ;  
          if ( ( efert[i].DaysEnable & (0x01 << k)) != 0 ){
            MyCheck = F("CHECKED")  ;    
            if ((k != 7 ) && ((efert[i].DaysEnable & 0x80 ) == 0  ) ){ // 
              MyColor =  F("bgcolor=red")   ;  
            }          
          }else{
            MyCheck = ""    ;      
          }
          message += "<td "+String(MyColor)+"><input type='checkbox' name='x"+String(i)+"dw"+ String(k)+"' "+String(MyCheck)+ "></td>";    
        }
        message += "<td><input type='text' name='xcqt"+String(i)+"' value='" + String(efert[i].CurrentQty) + "' maxlength=5 size=2></td><td><input type='text' name='xmqt"+String(i)+"' value='" + String(efert[i].MaxQty) + "' maxlength=5 size=2>";
        message += "<td><input type='text' name='xdes"+String(i)+"' value='" + String(efert[i].description) + "' maxlength=7 size=8></td>";
        message += "<td><select name='xaty"+String(i)+"'>" ;
        switch (efert[i].AType >> 4 ){
          case 2:    // master pump 
            message += F("<option value='1'>Proportioning Pump") ;
            message += F("<option value='2' SELECTED>Master Pump") ;
            message += F("<option value='4'>Chemical Valve") ;
            message += F("<option value='8'>Wash Valve") ;
          break;
          case 4:    // supply valve
            message += F("<option value='1'>Proportioning Pump") ;
            message += F("<option value='2'>Master Pump") ;
            message += F("<option value='4' SELECTED>Chemical Valve") ;
            message += F("<option value='8'>Wash Valve") ;
          break;
          case 8:    // wash valve
            message += F("<option value='1'>Proportioning Pump") ;
            message += F("<option value='2'>Master Pump") ;
            message += F("<option value='4'>Chemical Valve")  ;
            message += F("<option value='8' SELECTED>Wash Valve") ;
          break;
          default:    // garden variety proportioning pump
            message += F("<option value='1' SELECTED>Proportioning Pump") ;
            message += F("<option value='2'>Master Pump") ;
            message += F("<option value='4'>Chemical Valve") ;
            message += F("<option value='8'>Wash Valve") ;
          break;
        }
        message += "</select></td>" ;
        message += "<td><select name='xatm"+String(i)+"'>" ;
        for ( ii = 0 ; ii < MAX_FERT ; ii++){                             // handle all the fertigation control arguments
            if ((efert[i].AType & 0x0f) == ii) {
              message += "<option value='"+String(ii)+"' SELECTED>" + String(ii+1) ;          
            }else{
              message += "<option value='"+String(ii)+"'>" + String(ii+1) ;                        
            }
        }
        message += "</select></td>" ;
//        message += "<td><input type='text' name='xatm"+String(i)+"' value='" + String((efert[i].AType & 0x0f) + 1) + "' maxlength=1 size=3></td>";
      }
      if (iPage==2) {
        message += "<td><input type='text' name='xont"+String(i)+"' value='" + String(efert[i].OnTime) + "' maxlength=5 size=2></td><td><input type='text' name='xoft"+String(i)+"' value='" + String(efert[i].OffTime) + "' maxlength=5 size=2>";
        message += "<td><input type='text' name='xbas"+String(i)+"' value='" + String(efert[i].BaseFlow) + "' maxlength=5 size=2></td><td><input type='text' name='xpmr"+String(i)+"' value='" + String(efert[i].PumpRate) + "' maxlength=5 size=2>";
        message += "<td><input type='text' name='xbba"+String(i)+"' value='" + String((efert[i].BoardBit & 0xf0 )>>4 ) + "' maxlength=5 size=2></td><td><input type='text' name='xbbb"+String(i)+"' value='" + String(efert[i].BoardBit & 0x0F) + "' maxlength=5 size=2>";
      }
      message += F("<td><input type='submit' value='SET'></td></form>");
      if (iPage==0) {
        message += "<td><form method=post action=" + server.uri() + "><input type='hidden' name='xcqt"+String(i)+"' value='" + String(efert[i].MaxQty) + "'><input type='submit' value='REFILL'></form></td>" ;
      }
      message += F("</tr>\r\n") ;
      server.sendContent(message) ;
      message = "" ;
    }
    message = F("</table>\r\n") ;
    server.sendContent(message) ;
  }

  if (String(server.uri()).indexOf("vsss")>0) {  // volitile status - all structures status
    bDefault = false ;
    server.sendContent(F("<br><b>Fertigation Status</b>\r\n"));
    message = F("<table border=1 title='Fertigation Status'>\r\n") ;
    message += F("<tr><th>Tank</th><th>Run</th><th>Enable</th><th>On/Off</th><th>lTTG</th><th>Flowrate</th></tr>\r\n") ; 
    server.sendContent(message) ;
    message = "" ;
    
    for ( i = 0 ; i < MAX_FERT ; i++){
      if ((vfert[i].bRun ) == 0  ) {
        MyColor = F("bgcolor=red") ;
      }else{
        MyColor = F("bgcolor=green") ;
      }
      message += "<tr><td align=center "+String(MyColor)+">"+String(i)+"</td>";         
      message += "<td>" + String(vfert[i].bRun) + "</td>";
      message += "<td>" + String(vfert[i].bEnable) + "</td>";
      message += "<td>" + String(vfert[i].bOnOff) + "</td>";
      message += "<td>" + String(vfert[i].lTTG) + "</td>";
      message += "<td>" + String(vfert[i].Flowrate) + "</td></tr>\r\n";
      server.sendContent(message) ;
      message = "" ;
    }
    message = F("</table>\r\n") ;
    server.sendContent(message) ;

    message = F("<br><b>Filter Status</b><br><table border=1 title='Filter Status'>\r\n") ;
    message += F("<tr><th>Filter</th><th>Next Can</th><th>bOnOff</th><th>bFlush</th><th>lTTG</th></tr>\r\n") ; 
    server.sendContent(message) ;
    message = "" ;
    for ( i = 0 ; i < MAX_FILTER ; i++){
      if ((vfilter[i].bOnOff ) == 0  ) {
        MyColor = F("bgcolor=red") ;
      }else{
        MyColor = F("bgcolor=green") ;
      }
      message += "<tr><td align=center "+String(MyColor)+">"+String(i)+"</td>";         
      message += "<td>" + String(vfilter[i].NextCan) + "</td>";
      message += "<td>" + String(vfilter[i].bOnOff) + "</td>";
      message += "<td>" + String(vfilter[i].bFlush) + "</td>";
      message += "<td>" + String(vfilter[i].lTTG) + "</td></tr>\r\n";
      server.sendContent(message) ;
      message = "" ;
    }
    message = F("</table>\r\n") ;
    server.sendContent(message) ;

    message = F("<br><b>Valve Status</b><br><table border=1 title='Valve Status'>\r\n") ;
    message += F("<tr><th>Valve</th><th>bOnOff</th><th>lATTG</th><th>lTTG</th><th>iFB</th></tr>\r\n") ; 
    server.sendContent(message) ;
    message = "" ;
    for ( i = 0 ; i < MAX_VALVE ; i++){
      if ((vvalve[i].bOnOff ) == 0  ) {
        MyColor = F("bgcolor=red") ;
      }else{
        MyColor = F("bgcolor=green") ;
      }
      message += "<tr><td align=center "+String(MyColor)+">"+String(i)+"</td>";         
      message += "<td>" + String(vvalve[i].bOnOff,BIN) + "</td>";
      message += "<td>" + String(vvalve[i].lATTG) + "</td>";
      message += "<td>" + String(vvalve[i].lTTG) + "</td>";
      message += "<td>" + String(vvalve[i].iFB,HEX) + "</td></tr>\r\n";
      server.sendContent(message) ;
      message = "" ;
    }
    message = F("</table>\r\n") ;
    server.sendContent(message) ;
    message = F("<br><b>ADC Input Status</b><br><table border=1 title='ADC Status'>\r\n") ;
    message += "<tr><td>Raw </td><td>" + String(ADC_Raw) + "</td><td>(Counts)</td></tr>\r\n"  ;
    message += "<tr><td>Cooked </td><td>" + String(ADC_Value) + "</td><td>("+ String(ghks.ADC_Unit) + ")</td></tr>\r\n"  ;
    message += F("</table>\r\n") ;
    server.sendContent(message) ;
    message = "" ;    
  }
  
  if (bDefault) {     // #####################################   default valve control and setup  ##############################################
    server.sendContent(F("<br><b>Logical Valve Control</b>\r\n"));
    message = F("<table border=1 title='Logical Valve Control'>\r\n") ;
    message += F("<tr><th rowspan=2>Valve</th>") ;               // first heading row of table ( there are 2 )
    if (bExtraValve) {
      if (iPage == 1 ){
        message += F("<th rowspan=2>Description</th>") ; 
        message += F("<th colspan=5>Control Options</th><th colspan=3>Remote</th><th>Flow</th><th colspan=4>On</th><th colspan=4>Off</th></tr>\r\n");      
        message += F("<tr><th title='Cascade to Master Valve Number'>Cascade</th><th title='Always On - ignore global off until dates'>AO</th><th title='Domestic Water - Only activate if clear of fertiliser'>DW</th><th title='This is a master valve'>MV</th><th title='Feeback Circuit Present'>FB</th><th>Valve</th><th>RX</th><th>Node</th><th>(l/s)</th><th>Rly</th><th>Brd</th><th title='Active Polarity'>Pol</th><th title='Pulse Length'>Pulse</th><th>Rly</th><th>Brd</th><th title='Active Polarity'>Pol</th><th title='Pulse Length'>Pulse</th>") ; 
      }else{
        message += F("<th colspan=2>Feed Back</th><th colspan=");              
        message += String(MAX_FERT)+">Fertigate</th><th colspan="+String(MAX_FILTER)+">Filter</th></tr>\r\n" ;
        message += F("<tr><th>Rly</th><th>Brd</th>") ;
        for ( i = 0 ; ( i < 8 ) && ( i < MAX_FERT ) ; i++ ){
          message += "<th>" + String(i+1) + "</th>" ;
        }
        for ( i = 0 ; ( i < 8 ) && ( i < MAX_FILTER ) ; i++ ){
          message += "<th>" + String(i+1) + "</th>" ;
        }
      }
      message += F("</tr>\r\n") ;
    }else{                           // second heading row
      message += F("<th rowspan=2 colspan=2>State</th><th rowspan=2>Enable</th><th rowspan=2>ATTG</th><th rowspan=2>MTTG</th><th rowspan=2>DTTG</th></tr><tr></tr>\r\n") ;
    }
    for ( i = 0 ; i < ghks.lMaxDisplayValve ; i++){       // MAX_VALVE
      MyNum = String(i) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }        
      if ( vvalve[i].bOnOff ) {
        if (( vvalve[i].lTTG!=0 ) || ( vvalve[i].lATTG!=0 )) {
          MyColor = F("bgcolor=green") ;
        }else{
          MyColor = F("bgcolor=yellow") ;                  
        }
      }else{
        if (( vvalve[i].lTTG==0 ) && ( vvalve[i].lATTG==0 )) {
          MyColor = F("bgcolor=red") ;
        }else{
          MyColor = F("bgcolor=orange") ;          
        }
      }
      message += "<tr><td align=center "+String(MyColor)+">"+String(i+1) ;        // actual rows of table
      if (( bExtraValve) && (iPage == 1 )) {
        message += "</td>" ;        
      }else{
        message += " - "+String(evalve[i].description)+"</td>" ;                
      }
      if (!bExtraValve) {
        message += "<td align=center><form method=post action=" + server.uri() + "><input type='hidden' name='vsea"+MyNum+"' value='1'><input type='submit' value='ON'></form></td><td><form method=post action=" + server.uri() + "><input type='hidden' name='vsea"+MyNum+"' value='0'><input type='submit' value='OFF'></form></td>";         
        if ( evalve[i].bEnable ){
          MyCheck = "CHECKED" ;
          MyColor = F("bgcolor=green") ;
        }else{
          MyCheck = "" ;       
          MyColor = F("bgcolor=red") ;
        }
        message += "<form method=post action=" + server.uri() + "><input type='hidden' name='vnum"+MyNum+"' value='"+String(i)+"'><td align=center "+String(MyColor)+"><input type='checkbox' name='vben"+MyNum+"' "+String(MyCheck)+ "></td>";
        message += "<td>" + String(vvalve[i].lATTG) + "</td><td><input type='text' name='vttg"+MyNum+"' value='" + String(vvalve[i].lTTG) + "' maxlength=5 size=2></td><td><input type='text' name='dttg"+MyNum+"' value='" + String(evalve[i].lDTTG) + "' maxlength=5 size=2></td>";
      }
      if (bExtraValve) {
        if (iPage == 1 ){
          if (( evalve[i].TypeMaster & 0x200 ) != 0 ){   // always on program valve
            MyCheck3 = F(" CHECKED") ;
          }else{
            MyCheck3 = "" ;       
          }
          if (( evalve[i].TypeMaster & 0x100 ) != 0 ){  // domestic water
            MyCheck2 = F(" CHECKED") ;
          }else{
            MyCheck2 = "" ;       
          }
          if (( evalve[i].TypeMaster & 0x80 ) != 0 ){  // master valve
            MyCheck = F(" CHECKED") ;
          }else{
            MyCheck = "" ;       
          }
          if (( evalve[i].TypeMaster & 0x40 ) != 0 ){   // feedback
            MyColor = F(" CHECKED") ;
          }else{
            MyColor = "" ;       
          }
          message += "<form method=post action=" + server.uri() + "><input type='hidden' name='command' value='3'><input type='hidden' name='vnum"+MyNum+"' value='"+String(i)+"'><td><input type='text' name='vdes"+MyNum+"' value='" + String(evalve[i].description) + "' maxlength=7 size=8></td>" ; 
          message += "<td><input type='text' name='vmas"+MyNum+"' value='" + String((evalve[i].TypeMaster & 0x3f )) + "' maxlength=2 size=2></td><td><input type='checkbox' name='vtya"+MyNum+"'"+ String(MyCheck3) + "></td><td><input type='checkbox' name='vtyd"+MyNum+"'"+ String(MyCheck2) + "></td><td><input type='checkbox' name='vtym"+MyNum+"'"+ String(MyCheck) + "></td><td><input type='checkbox' name='vtyf"+MyNum+"'"+ String(MyColor) + "></td>" ;
          message += "<td><input type='text' name='ncom"+MyNum+"' value='" + String(evalve[i].Valve & 0x7f) + "' maxlength=3 size=2></td>";
          if (( evalve[i].Valve & 0xf0 ) != 0 ){
            MyColor = F(" CHECKED") ;
          }else{
            MyColor = "" ;       
          }
          message += "<td><input type='checkbox' name='ncul"+MyNum+"'"+ String(MyColor) + "></td>";
          message += "<td><input type='text' name='ncon"+MyNum+"' value='" + String(evalve[i].Node) + "' maxlength=3 size=2></td>";
          message += "<td><input type='text' name='nflo"+MyNum+"' value='" + String(evalve[i].Flowrate) + "' maxlength=5 size=2></td>";
          if ((evalve[i].OnOffPolPulse & 0x80) != 0 ){
            MyCheck = "<select name='vppon"+MyNum+"'><option value='0'>Pos<option value='1' SELECTED>Neg</select>" ;
          }else{
            MyCheck = "<select name='vppon"+MyNum+"'><option value='0' SELECTED>Pos<option value='1'>Neg</select>" ;            
          }
          message += "<td><input type='text' name='vcon"+MyNum+"' value='" + String(evalve[i].OnCoilBoardBit & 0x0f ) + "' maxlength=3 size=2></td><td><input type='text' name='vaon"+MyNum+"' value='" + String((evalve[i].OnCoilBoardBit & 0xf0 ) >> 4 ) + "' maxlength=2 size=2></td><td>"+ MyCheck+"</td><td><input type='text' name='vpon"+MyNum+"' value='" + String((evalve[i].OnOffPolPulse & 0x70 ) >> 4 ) + "' maxlength=3 size=2></td>" ;
/*          if ((evalve[i].OnOffPolPulse & 0x08) != 0 ){
            MyCheck = "<select name='vppof"+MyNum+"'><option value='0'>Pos<option value='1' SELECTED>Neg</select>" ;
          }else{
            MyCheck = "<select name='vppof"+MyNum+"'><option value='0' SELECTED>Pos<option value='1'>Neg</select>" ;            
          }*/
          if (evalve[i].OnCoilBoardBit == evalve[i].OffCoilBoardBit ) { // if these are the same just show polarity as reversed
            if ((evalve[i].OnOffPolPulse & 0x80) != 0 ){
              MyCheck = "Pos" ;
            }else{
              MyCheck = "Neg" ;
            }            
          }else{
            if ((evalve[i].OnOffPolPulse & 0x80) != 0 ){
              MyCheck = "Neg" ;
            }else{
              MyCheck = "Pos" ;
            }            
          }
          message += "<td><input type='text' name='vcof"+MyNum+"' value='" + String(evalve[i].OffCoilBoardBit & 0x0f) + "' maxlength=3 size=2></td><td><input type='text' name='vaof"+MyNum+"' value='" + String((evalve[i].OffCoilBoardBit & 0xf0 ) >> 4) + "' maxlength=3 size=2></td><td>"+MyCheck+"</td><td><input type='text' name='vpof"+MyNum+"' value='" + String((evalve[i].OnOffPolPulse & 0x07 )) + "' maxlength=3 size=2></td>" ; 
        }else{
          message += "<form method=post action=" + server.uri() + "><input type='hidden' name='command' value='4'><td><input type='text' name='vfbt"+MyNum+"' value='" + String(evalve[i].FeedbackBoardBit & 0x0f ) + "' maxlength=3 size=2></td><td><input type='text' name='vfad"+MyNum+"' value='" + String((evalve[i].FeedbackBoardBit & 0xf0 ) >> 4 ) + "' maxlength=3 size=2></td>" ;
          for (k = 0 ; ( k < 8 ) && ( k < MAX_FERT ) ; k++){      
            MyColor =  ""   ;  
            if ( ( evalve[i].Fertigate & (0x01 << k)) != 0 ){
              MyCheck = F("CHECKED")  ;    
              MyColor =  F("bgcolor=green") ;  
            }else{
              MyCheck = "" ;      
            }
            message += "<td align=center"+String(MyColor)+"><input type='checkbox' name='v"+MyNum+"fg"+ String(k)+"' "+String(MyCheck)+ "></td>";    
          }
  
          for (k = 0 ; ( k < 8 ) && ( k < MAX_FILTER ) ; k++){      
            MyColor =  ""   ;  
            if ( ( evalve[i].Filter & (0x01 << k)) != 0 ){
              MyCheck = F("CHECKED")  ;    
              MyColor =  F("bgcolor=green")   ;  
            }else{
              MyCheck = "" ;      
            }
            message += "<td align=center"+String(MyColor)+"><input type='checkbox' name='v"+MyNum+"fs"+ String(k)+"' "+String(MyCheck)+ "></td>";    
          }          
        }
      }
      message += F("<td><input type='submit' value='SET'></td></form></tr>\r\n") ;
      server.sendContent(message) ;
      message = "" ;
    }
    if ( !bExtraValve ){
      message += "<tr><form method=post action=" + server.uri() + "><td colspan=8 align=center><input type='hidden' name='command' value='0'><input type='submit' value='### MANUAL ALL OFF ###'></td></form></tr>\r\n" ;
    }
    message += F("</table>\r\n");
    server.sendContent(message) ;
    message = "" ;
  }
  SendHTTPPageFooter();
}


