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
void SendHTTPHeader(void){
  SendHTTPHeader(0);
}  

void SendHTTPHeader(int iRefresh){
String message ;
String strTmp = "" ;
//  setCpuFrequencyMhz(240);

  server.sendHeader(F("Server"),F("ESP32-on-beetle-juice"),false);
  server.sendHeader(F("X-Powered-by"),F("Dougal-filament-7"),false);
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  message = F("<!DOCTYPE HTML>");
  message += "<head><title>Team Trouble - Irrigation Controler " + String(Toleo) + "</title>";
  message += F("<meta name=viewport content='width=320, auto inital-scale=1'>"); 
  if ( iRefresh > 0 ){
    message += "<meta http-equiv='Refresh' content='"+String(iRefresh)+"'>" ;
  }
  message += F("</head><body><html><center><h3>");   
  if ( ( ghks.ADC_Alarm_Mode & 0x80 ) != 0 ){
    strTmp = String(ADC_Value) + " " + String(ghks.ADC_Unit) ;
  }else{
    strTmp = "" ;
  }  
  message += String(WiFi.RSSI()) + " (dBm) <a title='click for home / refresh' href='/'>"+String(ghks.NodeName)+"</a> " + strTmp + "</h3>\r\n";
  server.sendContent(message) ;  
  message = "" ;       
}



void SendHTTPPageFooter(){
  String message = F("<br><a href='/?command=3'>Valve Setup Page 1</a>.. <a href='/?command=4'>Valve Setup Page 2</a><br>\r\n") ;         
  message += F("<a href='/fert'>Fertigation Control Page 1</a>.. <a href='/fert?command=5'>Fertigation Setup Page 2</a> <br> <a href='/filt'>Filter Setup</a><br>\r\n") ;   
  message += F("<br><a href='/?command=1'>Load Parameters from EEPROM</a><br><br><a href='/?command=667'>Reset Memory to Factory Default</a><br><a href='/?command=665'>Sync UTP Time</a><br><a href='/stime'>Manual Time Set</a><br><a href='/scan'>I2C Scan</a><br><a href='/iosc'>Database I/O Scan</a><br><a href='/iolocal'>Local I/O Mapping</a><br>\r\n") ;     
  message += "<a href='/?reboot=" + String(lRebootCode) + "'>Reboot</a><br>\r\n" ;
  message += F("<a href='/?command=668'>Save Fert Current QTY</a><br>\r\n") ;
  message += F("<a href='/eeprom'>EEPROM Memory Contents</a><br>\r\n");
  message += F("<a href='/setup'>Node Setup</a><br>\r\n");
  message += F("<a href='/email'>Email Setup</a><br>\r\n");  
  message += F("<a href='/adc'>ADC Setup</a><br>\r\n");  
  message += F("<a href='/info'>Node Infomation</a><br>\r\n");
  server.sendContent(message) ;  
  message = "" ;       
  message += F("<a href='/btest'>Relay Board Test</a><br>\r\n");
  message += F("<a href='/vsss'>view volatile memory structures</a><br>\r\n");
  if ((MyIP[0]==0)&&(MyIP[1]==0)&&(MyIP[2]==0)&&(MyIP[3]==0)) {
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);
  }else{
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
  }
  message += "<a href='http://" + String(buff) + "/update'>OTA Firmware Update</a><br>\r\n" ;  
  message += F("<a href='https://github.com/Dougal121/Essence_of_Thor'>Source at GitHub</a><br>\r\n") ;  
  message += "<a href='http://" + String(buff) + "/backup'>Backup / Restore Settings</a><br><br>\r\n" ;  
  snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
  message += "Computer Uptime <b>"+String(buff)+"</b> (day:hr:min) <br>\r\n" ;  
  message += F("</body></html>\r\n\r\n") ;
  server.sendContent(message) ;  
  message = "" ;       
//  SetSelectedSpeed();
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
  tmElements_t tm;

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
          bSendSaveConfirm = true ;

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
//          ESP.reset() ;
        break;
        case 9: //  Warm Reboot
          ESP.restart() ;
        break;
        case 42:
          if ( iTestMode == -1 ) {
            iTestMode = 0 ;            
          }else{
            iTestMode = -1 ;            
          }
        break;
        case 11:  // load values
          bSendTestEmail = true ;
        break;
        case 121:
          ResetSMTPInfo();
        break;        
        case 122:
          ResetADCCalInfo();
        break;   
        
        case 667: // wipe the memory to factory default
          BackInTheBoxMemory();
        break;
        case 665:
          sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour  
        break;
        case 668:
          ii = SaveCurrentQty(false);
//          Serial.println(String(ii) + " Fert records updated");
        break;
      }  
    }
    i = String(server.argName(j)).indexOf("reboot");
    if (i != -1){  // 
      if (( lRebootCode == String(server.arg(j)).toInt() ) && (lRebootCode>0 )){  // stop the phone browser being a dick and retry resetting !!!!
        ESP.restart() ;        
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

    
    i = String(server.argName(j)).indexOf("atoff");
    if (i != -1){  // have a request to request a time update
      tm.Year = (String(server.arg(j)).substring(0,4).toInt()-1970) ;
      tm.Month =(String(server.arg(j)).substring(5,7).toInt()) ;
      tm.Day = (String(server.arg(j)).substring(8,10).toInt()) ;
      tm.Hour =(String(server.arg(j)).substring(11,13).toInt()) ;
      tm.Minute = (String(server.arg(j)).substring(14,16).toInt()) ;
      tm.Second = 0 ;
      ghks.AutoOff_t = makeTime(tm);
    }  

    
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######

  if ( bSaveReq != 0 ){
    server.sendContent(F("<blink>"));      
  }   
  server.sendContent(F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>")) ;     
  if ( bSaveReq != 0 ){
    server.sendContent(F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>")) ;     
  }
    
  if (ghks.lProgMethod == 0 ){    
    server.sendContent(F("<a href='/prog'>Programs</a><br>")) ;         
  }else{
    server.sendContent(F("<a href='/prognew'>Programs</a><br>")) ;             
  }
  
  SendCurrentTime();

  if ( ghks.AutoOff_t > now() )  {
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t), second(ghks.AutoOff_t));
    server.sendContent(F("<b><font color=red>Automation OFFLINE Untill ")) ;  
    server.sendContent(String(buff)) ; 
    server.sendContent(F("</font></b><br>")) ; 
  }else{
    if ( year() > 2000 ) {
      server.sendContent(F("<b><font color=green>Automation ONLINE</font></b><br>")) ;  
    }else{
      server.sendContent(F("<b><font color=green>Automation OFFLINE Invalid time</font></b><br>")) ;        
    }
  }



    
  
  if (String(server.uri()).indexOf("prog")>0) {  // #############################  program interface  #####################################
    bDefault = false ;
    message = "<br><form method=post action=" + server.uri() + ">" ;
    message += "Program: <select name='prgm'>" ;
    for ( j = 0 ; j < MAX_PROGRAM ; j++ ){
      if ( j == iProgNum ){
        MyColor = F("SELECTED");
      }else{
        MyColor = "" ;        
      }
      message += "<option value='"+String(j)+"' " + MyColor + " >" + String(j+1) ;
    }
    message += F("</select><input type='submit' value='SET'></form><br>");
    message += F("<b>Valve Programs</b><br><table border=1 title='Valve Program'>");
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
    message +=F("</tr>");   
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
      message += F("<td><input type='submit' value='SET'></td></form></tr>"); 
      server.sendContent(message) ;
    }
    server.sendContent(F("</table><br>"));    
  }


  
  if (bDefault) {     // #####################################   default valve control and setup  ##############################################
    server.sendContent(F("<br><b>Logical Valve Control</b>"));
    message = F("<table border=1 title='Logical Valve Control'>") ;
    message += F("<tr><th rowspan=2>Valve</th>") ;               // first heading row of table ( there are 2 )
    if (bExtraValve) {
      if (iPage == 1 ){
        message += F("<th rowspan=2>Description</th>") ; 
        message += F("<th colspan=5>Control Options</th><th colspan=3>Remote</th><th>Flow</th><th colspan=4>On</th><th colspan=4>Off</th></tr>");      
        message += F("<tr><th>Cascade</th><th title='Always On'>AO</th><th title='Domestic Water'>DW</th><th title='Master Valve'>MV</th><th title='Feed Back'>FB</th><th title='Send to Valve'>Valve</th><th title='Accept Uplink'>Rx</th><th title='Send to Node ID'>Node</th><th>(l/s)</th><th>Rly</th><th>Brd</th><th>Pol</th><th>Pulse</th><th>Rly</th><th>Brd</th><th>Pol</th><th>Pulse</th></tr>") ; 
      }else{
        message += F("<th colspan=2>Feed Back</th><th colspan=");              
        message += String(MAX_FERT)+">Fertigate</th><th colspan="+String(MAX_FILTER)+">Filter</th></tr>" ;
        message += F("<tr><th>Rly</th><th>Brd</th>") ;
        for ( i = 0 ; ( i < 8 ) && ( i < MAX_FERT ) ; i++ ){
          message += "<th>" + String(i+1) + "</th>" ;
        }
        for ( i = 0 ; ( i < 8 ) && ( i < MAX_FILTER ) ; i++ ){
          message += "<th>" + String(i+1) + "</th>" ;
        }
      }
      message += F("</tr>") ;
    }else{                           // second heading row
      message += F("<th rowspan=2 colspan=2>State</th><th rowspan=2>Enable</th><th rowspan=2>ATTG</th><th rowspan=2>MTTG</th><th rowspan=2>DTTG</th></tr><tr></tr>") ;
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
          if (( evalve[i].TypeMaster & 0x100 ) != 0 ){
            MyCheck2 = F(" CHECKED") ;
          }else{
            MyCheck2 = "" ;       
          }
          if (( evalve[i].TypeMaster & 0x80 ) != 0 ){
            MyCheck = F(" CHECKED") ;
          }else{
            MyCheck = "" ;       
          }
          if (( evalve[i].TypeMaster & 0x40 ) != 0 ){
            MyColor = F(" CHECKED") ;
          }else{
            MyColor = "" ;       
          }
          message += "<form method=post action=" + server.uri() + "><input type='hidden' name='command' value='3'><input type='hidden' name='vnum"+MyNum+"' value='"+String(i)+"'><td><input type='text' name='vdes"+MyNum+"' value='" + String(evalve[i].description) + "' maxlength=7 size=8></td>" ; 
          message += "<td><input type='text' name='vmas"+MyNum+"' value='" + String((evalve[i].TypeMaster & 0x3f )) + "' maxlength=2 size=2></td><td><input type='checkbox' name='vtya"+MyNum+"'"+ String(MyCheck3) + "></td><td><input type='checkbox' name='vtyd"+MyNum+"'"+ String(MyCheck2) + "></td><td><input type='checkbox' name='vtym"+MyNum+"'"+ String(MyCheck) + "></td><td><input type='checkbox' name='vtyf"+MyNum+"'"+ String(MyColor) + "></td>" ;
          message += "<td><input type='text' name='ncom"+MyNum+"' value='" + String(evalve[i].Valve & 0x7f ) + "' maxlength=3 size=2></td>";
          if (( evalve[i].Valve & 0x80 ) != 0 ){  // f0 ????
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
      message += F("<td><input type='submit' value='SET'></td></form></tr>") ;
      server.sendContent(message) ;
      message = "" ;
    }
    message += F("</table>");
    server.sendContent(message) ;
  }

  SendHTTPPageFooter();
}

void SendCurrentTime(void){
  String message ;

  snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
  if (ghks.fTimeZone > 0 ) {
    message = "<b>"+ String(buff) + " UTC +" + String(ghks.fTimeZone,1)  ;   
  }else{
    message = "<b>"+ String(buff) + " UTC " + String(ghks.fTimeZone,1) ;       
  }
  if ( year() < 2000 ) {
    message += F("  --- CLOCK NOT SET ---") ;
  }
  message += F("</b><br>") ;
  server.sendContent(message) ;  
}


