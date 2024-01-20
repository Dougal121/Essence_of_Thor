void handleInfo(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];


  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
  }
  
  SendHTTPHeader();

  server.sendContent(F("<br><center><b>Node Info</b><br>"));
  server.sendContent(F("<table border=1 title='Device Info'>"));

  server.sendContent("<tr><td>ESP ID</td><td align=center>0x" + String((uint32_t)chipid, HEX) + "</td><td align=center>"+String((uint32_t)chipid)+"</td></tr>" ) ; 
  MyIP =  WiFi.localIP() ;
  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
  server.sendContent("<tr><td>Network Node IP Address</td><td align=center>" + String(buff) + "</td><td>.</td></tr>" ) ; 
  MyIPC = WiFi.softAPIP();  // get back the address to verify what happened  
  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);
  server.sendContent("<tr><td>Config Node IP Address</td><td align=center>" + String(buff) + "</td><td>.</td></tr>" ) ; 
  server.sendContent("<tr><td>Time Server</td><td align=center>" + String(ghks.timeServer) + "</td><td>.</td></tr>" ) ; 
  server.sendContent("<tr><td>Network SSID</td><td align=center>" + String(ghks.nssid) + "</td><td>.</td></tr>" ) ; 
  server.sendContent("<tr><td>Network Password</td><td align=center>" + String(ghks.npassword) + "</td><td>.</td></tr>" ) ; 
  server.sendContent("<tr><td>Configure SSID</td><td align=center>" + String(cssid) + "</td><td>.</td></tr>" ) ; 
  server.sendContent("<tr><td>Configure Password</td><td align=center>" + String(ghks.cpassword) + "</td><td>.</td></tr>" ) ; 
  
  server.sendContent("<tr><td>WiFi RSSI</td><td align=center>" + String(WiFi.RSSI()) + "</td><td>(dBm)</td></tr>" ) ; 
  WiFi.macAddress(mac);      
  snprintf(buff, BUFF_MAX, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  server.sendContent("<tr><td>MAC Address</td><td align=center>" + String(buff) + "</td><td align=center>.</td></tr>" ) ; 
 
  server.sendContent("<tr><td>Last Scan Speed</td><td align=center>" + String(lScanLast) + "</td><td>(per second)</td></tr>" ) ;    

//  server.sendContent("<tr><td>ESP Core Version</td><td align=center>" + String(ESP.getCoreVersion()) + "</td><td>.</td></tr>" ) ;    
  server.sendContent("<tr><td>ESP Chp Revision</td><td align=center>" + String(ESP.getChipRevision()) + "</td><td>.</td></tr>" ) ;    
  server.sendContent("<tr><td>SDK Version</td><td align=center>" + String(ESP.getSdkVersion()) + "</td><td>.</td></tr>" ) ;    
//  server.sendContent("<tr><td>CPU Volts</td><td align=center>" + String(ESP.getVcc()) + "</td><td>(V)</td></tr>" ) ;    
  server.sendContent("<tr><td>CPU Frequecy</td><td align=center>" + String(ESP.getCpuFreqMHz()) + "</td><td>(MHz)</td></tr>" ) ;    
  server.sendContent("<tr><td>XTAL Frequecy</td><td align=center>" + String(getXtalFrequencyMhz()) + "</td><td>(MHz)</td></tr>" ) ;    
  server.sendContent("<tr><td>APB Frequecy</td><td align=center>" + String(getApbFrequency()/1000000) + "</td><td>(MHz)</td></tr>" ) ;    
//  server.sendContent("<tr><td>Get Rest Reason</td><td align=center>" + String(ESP.getResetReason()) + "</td><td></td></tr>" ) ;    
//  server.sendContent("<tr><td>Get Reset Into</td><td align=center>" + String(ESP.getResetInfo()) + "</td><td></td></tr>" ) ;    
//  server.sendContent("<tr><td>Get Sketch Size</td><td align=center>" + String(ESP.getSketchSize()) + "</td><td>(Bytes)</td></tr>" ) ;    
//  server.sendContent("<tr><td>Free Sketch Space</td><td align=center>" + String(ESP.getFreeSketchSpace()) + "</td><td>(Bytes)</td></tr>" ) ;    
  server.sendContent("<tr><td>Magnetic Sensor Value</td><td align=center>" + String(magval) + "</td><td>(?)</td></tr>" ) ;    
  server.sendContent("<tr><td>LoRa Bandwidth</td><td align=center>" + String(LoRa.getSignalBandwidth()) + "</td><td>(kBps)</td></tr>" ) ;    
  server.sendContent("<tr><td>LoRa Spreading Factor</td><td align=center>" + String(LoRa.getSpreadingFactor()) + "</td><td>(?)</td></tr>" ) ;    
  

  snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
  server.sendContent("<tr><td>Computer Uptime</td><td align=center>"+String(buff)+"</td><td>(day:hr:min)</td></tr>" ) ;

  server.sendContent(F("</table><br>"));    
  SendHTTPPageFooter();
}


void handleSetup(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message = "" ;  
  long lTmp ;
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  String MyCheck ;
  byte mac[6];
  String strSelected = "";
  String strOption = "" ; 
   tmElements_t tm;

//  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 2: // Save values
          LoadParamsFromEEPROM(false);
          bSendSaveConfirm = true ;
//          Serial.println("Save to EEPROM");
        break;
      }
    }

    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 369:  
          ResetLoRaParams();
        break;
      }  
    }

    
    i = String(server.argName(j)).indexOf("lrspr");
    if (i != -1){  // 
      ghks.iSpread = String(server.arg(j)).toInt() ;
      ghks.iSpread = constrain(ghks.iSpread,5,12);
    }        
    i = String(server.argName(j)).indexOf("lrpwr");
    if (i != -1){  // 
      ghks.iTXPower = String(server.arg(j)).toInt() ;
      ghks.iTXPower = constrain(ghks.iTXPower,2,20);
    }        
    i = String(server.argName(j)).indexOf("lrfre");
    if (i != -1){  // 
      ghks.iFreq = String(server.arg(j)).toInt() ;
      ghks.iFreq = constrain(ghks.iFreq,9150,9285);
    }        
    i = String(server.argName(j)).indexOf("lrban");
    if (i != -1){  // 
      ghks.iBandWidth = String(server.arg(j)).toInt() ;
      ghks.iBandWidth = constrain(ghks.iBandWidth,0,9);
    }        
    i = String(server.argName(j)).indexOf("ptime");
    if (i != -1){  // 
      ghks.lPulseTime = String(server.arg(j)).toInt() ;
      ghks.lPulseTime = constrain(ghks.lPulseTime,10,255);
    }        
    i = String(server.argName(j)).indexOf("ndadd");
    if (i != -1){  // 
      ghks.lNodeAddress = String(server.arg(j)).toInt() ;
      ghks.lNodeAddress = constrain(ghks.lNodeAddress,0,32768);
    }        
    i = String(server.argName(j)).indexOf("tzone");
    if (i != -1){  // 
      ghks.fTimeZone = String(server.arg(j)).toFloat() ;
      ghks.fTimeZone = constrain(ghks.fTimeZone,-12,12);
      bDoTimeUpdate = true ; // trigger and update to fix the time
    }        
    i = String(server.argName(j)).indexOf("disop");
    if (i != -1){  // 
      ghks.lDisplayOptions = String(server.arg(j)).toInt() ;
      ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,255);
    }  
    i = String(server.argName(j)).indexOf("disro");
    if (i != -1){  // 
      ii =(( 0x01 & String(server.arg(j)).toInt()) << 1 ) ;
      ghks.lDisplayOptions = ( ghks.lDisplayOptions & 0xFD ) | ii ;
      ghks.lDisplayOptions = constrain(ghks.lDisplayOptions,0,255);
    }  
    i = String(server.argName(j)).indexOf("cpufr");
    if (i != -1){  // 
      ghks.cpufreq = String(server.arg(j)).toInt() ;
    }  
    i = String(server.argName(j)).indexOf("magsn");
    if (i != -1){  // 
      ghks.magsens = String(server.arg(j)).toInt() ;
    }  

    i = String(server.argName(j)).indexOf("netop");
    if (i != -1){  // 
      ghks.lNetworkOptions = String(server.arg(j)).toInt() ;
      ghks.lNetworkOptions = constrain(ghks.lNetworkOptions,0,255);
    }
    i = String(server.argName(j)).indexOf("maxvn");
    if (i != -1){  // 
      ghks.lMaxDisplayValve = String(server.arg(j)).toInt() ;
      ghks.lMaxDisplayValve = constrain(ghks.lMaxDisplayValve,2,MAX_VALVE);
    }  
    i = String(server.argName(j)).indexOf("dotmr");
    if (i != -1){  // 
      ghks.displaytimer = String(server.arg(j)).toInt() ;
      ghks.displaytimer = constrain(ghks.displaytimer,-1,255);
    }  
    
    i = String(server.argName(j)).indexOf("prgop");
    if (i != -1){  // 
      ghks.lProgMethod = String(server.arg(j)).toInt() ;
      ghks.lProgMethod = constrain(ghks.lProgMethod,0,1);
    }          
    i = String(server.argName(j)).indexOf("lpntp");
    if (i != -1){  // 
      ghks.localPort = String(server.arg(j)).toInt() ;
      ghks.localPort = constrain(ghks.localPort,1,65535);
    }        
    i = String(server.argName(j)).indexOf("lpctr");
    if (i != -1){  // 
      ghks.localPortCtrl = String(server.arg(j)).toInt() ;
      ghks.localPortCtrl = constrain(ghks.localPortCtrl,1,65535);
    }        
    i = String(server.argName(j)).indexOf("rpctr");
    if (i != -1){  // 
      ghks.RemotePortCtrl = String(server.arg(j)).toInt() ;
      ghks.RemotePortCtrl = constrain(ghks.RemotePortCtrl,1,65535);
    }        
    i = String(server.argName(j)).indexOf("dontp");
    if (i != -1){  // have a request to request a time update
      bDoTimeUpdate = true ;
    }
    i = String(server.argName(j)).indexOf("cname");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( ghks.NodeName , sizeof(ghks.NodeName)) ;
    }
    i = String(server.argName(j)).indexOf("rpcip");
    if (i != -1){  // have a request to request an IP address
      ghks.RCIP[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.RCIP[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.RCIP[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.RCIP[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("staip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPStatic[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPStatic[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPStatic[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPStatic[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("gatip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPGateway[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPGateway[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPGateway[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPGateway[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("mskip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPMask[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPMask[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPMask[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPMask[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("dnsip");
    if (i != -1){  // have a request to request an IP address
      ghks.IPDNS[0] = String(server.arg(j)).substring(0,3).toInt() ;
      ghks.IPDNS[1] =String(server.arg(j)).substring(4,7).toInt() ;
      ghks.IPDNS[2] = String(server.arg(j)).substring(8,11).toInt() ;
      ghks.IPDNS[3] =String(server.arg(j)).substring(12,15).toInt() ;
    }
    i = String(server.argName(j)).indexOf("nssid");
    if (i != -1){                                    // SSID
 //    Serial.println("SookyLala 1 ") ;
     String(server.arg(j)).toCharArray( ghks.nssid , sizeof(ghks.nssid)) ;
    }
    
    i = String(server.argName(j)).indexOf("npass");
    if (i != -1){                                    // Password
     String(server.arg(j)).toCharArray( ghks.npassword , sizeof(ghks.npassword)) ;
    }
    
    i = String(server.argName(j)).indexOf("cpass");
    if (i != -1){                                    // Password
     String(server.arg(j)).toCharArray( ghks.cpassword , sizeof(ghks.cpassword)) ;
    }
    
    i = String(server.argName(j)).indexOf("timsv");
    if (i != -1){                                    // timesvr
     String(server.arg(j)).toCharArray( ghks.timeServer , sizeof(ghks.timeServer)) ;
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
    i = String(server.argName(j)).indexOf("srbt");
    if (i != -1){  // 
      ghks.SelfReBoot =  String(server.arg(j)).toInt()  ;
      if ((ghks.SelfReBoot < MIN_REBOOT )){
        if (ghks.SelfReBoot <= 0 ) {
          ghks.SelfReBoot = 0 ;                  
        }else{
          ghks.SelfReBoot = MIN_REBOOT ;
        }
      }
    }  
    i = String(server.argName(j)).indexOf("lrtd");
    if (i != -1){  // 
      lTmp =  String(server.arg(j)).toInt()  ;
      ghks.lRebootTimeDay = lTmp & 0xfff ;
    }        
    for ( k = 0 ; k < 8 ; k++){  // handle all the valve control commands for any and all valves
      i = String(server.argName(j)).indexOf( "dw" + String(k) );
      if (i != -1){  // 
        ghks.lRebootTimeDay |= ( 0x1000 << k ) ;
      }              
    }    



    i = String(server.argName(j)).indexOf("pprb");
    if (i != -1){  // off valve address
       ghks.SolPulsePower = (  ghks.SolPulsePower & 0x0F ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
       ghks.SolPulsePower &= 0x3FFF ;
    }     
    i = String(server.argName(j)).indexOf("pprz");
    if (i != -1){  // off valve control number
       ghks.SolPulsePower = (  ghks.SolPulsePower & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
    } 
    i = String(server.argName(j)).indexOf("ppre");
    if (i != -1){  // 
      ghks.SolPulsePower |= 0x8000 ;
    }              
    i = String(server.argName(j)).indexOf("pprp");
    if (i != -1){  
       ghks.SolPulsePower |= (( 0x01 & String(server.arg(j)).toInt())<<14 )  ;
    }

    i = String(server.argName(j)).indexOf("hprb");
    if (i != -1){  // off valve address
       ghks.SolContPower = (  ghks.SolContPower & 0x0F ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
       ghks.SolContPower &= 0x3FFF ;
    }     
    i = String(server.argName(j)).indexOf("hprz");
    if (i != -1){  // off valve control number
       ghks.SolContPower = ( ghks.SolContPower & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
    } 
    i = String(server.argName(j)).indexOf("hpre");
    if (i != -1){  // 
      ghks.SolContPower |= 0x8000 ;
    }              
    i = String(server.argName(j)).indexOf("hprp");
    if (i != -1){  
       ghks.SolContPower |= (( 0x01 & String(server.arg(j)).toInt())<<14 )  ;
    }

    i = String(server.argName(j)).indexOf("mprb");
    if (i != -1){  // 
       ghks.SolMastPower = (  ghks.SolMastPower & 0x0F ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
       ghks.SolMastPower &= 0x3FFF ;
    }     
    i = String(server.argName(j)).indexOf("mprz");
    if (i != -1){  // 
       ghks.SolMastPower = ( ghks.SolMastPower & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
    } 
    i = String(server.argName(j)).indexOf("mpre");
    if (i != -1){  // 
      ghks.SolMastPower |= 0x8000 ;
    }              
    i = String(server.argName(j)).indexOf("mprp");
    if (i != -1){  
       ghks.SolMastPower |= (( 0x01 & String(server.arg(j)).toInt())<<14 )  ;
    }
            
  }
  
  SendHTTPHeader();
  server.sendContent(F("<a href='/setup?command=2'>Save Parameters to EEPROM</a><br>")) ;     

  message += "<form method=post action=" + server.uri() + "><table border=1 title='Node Settings'>";
  message += F("<tr><th>Parameter</th><th>Value</th><th><input type='submit' value='SET'></th></tr>\r\n");

  message += F("<tr><td>Controler Name</td><td align=center>") ; 
  message += "<input type='text' name='cname' value='"+String(ghks.NodeName)+"' maxlength=15 size=12></td><td></td></tr>\r\n";

  snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t));
  if (ghks.AutoOff_t > now()){
    MyColor =  F("bgcolor=red") ;
  }else{
    MyColor =  "" ;
  }
  message += "<tr><td "+String(MyColor)+">Auto Off Until</td><td align=center>" ; 
  message += "<input type='text' name='atoff' value='"+ String(buff) + "' size=12></td><td>(yyyy/mm/dd)</td></tr>\r\n";

  message += F("<tr><td>Activation Pulse</td><td align=center>") ; 
  message += "<input type='text' name='ptime' value='" + String(ghks.lPulseTime) + "' size=12></td><td>(ms)</td></tr>\r\n";

  message += F("<tr><td>Node Address</td><td align=center>") ; 
  message += "<input type='text' name='ndadd' value='" + String(ghks.lNodeAddress) + "' size=12></td><td>"+String(ghks.lNodeAddress & 0xff)+"</td></tr>\r\n";

  message += F("<tr><td>Time Zone</td><td align=center>") ; 
  message += "<input type='text' name='tzone' value='" + String(ghks.fTimeZone,1) + "' size=12></td><td>(Hours)</td></tr>\r\n";

  message += F("<tr><td>Self Reboot Timer</td><td align=center>") ; 
  message += "<input type='text' name='srbt' value='" + String( ghks.SelfReBoot) + "' size=8 maxlength=8></td><td>(min)</td></tr>";

  message += F("<tr><td>Reboot Time of Day</td><td align=center>") ; 
  message += "<input type='text' name='lrtd' value='" + String( ghks.lRebootTimeDay & 0xfff ) + "' size=8 maxlength=8></td><td>(HHMM)</td></tr>";
  message += "<tr><td>Reboot Days</td><td colspan=2 align=center>";
  for (k = 0 ; k < 8 ; k++){      
    MyColor =  ""   ;  
    if ( ( ghks.lRebootTimeDay & (0x1000 << k)) != 0 ){
      MyCheck = F("CHECKED")  ;  
    }else{
      MyCheck = "" ;      
    }
    message += String(dayarray[k])+ "<input type='checkbox' name='dw" + String(k)+"' "+String(MyCheck)+ ">";    
  }
  message += F("</td></tr></form>\r\n<tr><td colspan=3></td></tr>");
  message += "<form method=post action=" + server.uri() + ">" ;
  message += F("<tr><td><b>Active Power Management</b></td><td><b><center>Board / Bit / Active / Pol<b></td><td><input type='submit' value='SET'></td></tr>\r\n") ;

  message += F("<tr><td>Pulse Power</td><td align=center>") ; 
  if (( ghks.SolPulsePower & 0x8000 ) != 0 ){
    MyCheck = F("CHECKED")  ;  
  }else{
    MyCheck = "" ;      
  }
  message += "<input type='text' name='pprb' value='" + String(( ghks.SolPulsePower >> 4) & 0x0f ) + "' size=4 maxlength=2>" ;
  message += "<input type='text' name='pprz' value='" + String(( ghks.SolPulsePower & 0x0f )) + "' size=4 maxlength=2>" ;
  message += " Enable <input type='checkbox' name='ppre' "+String(MyCheck)+ ">";    
  if ((ghks.SolPulsePower & 0x4000  ) != 0 ){       // polarity of the pulse
    message +=  "<select name='pprp'><option value='0'>Pos<option value='1' SELECTED>Neg</select>" ;    
  }else{
    message +=  "<select name='pprp'><option value='0' SELECTED>Pos<option value='1'>Neg</select>" ;            
  }
  message += F("</td><td></td></tr>\r\n");

  message += F("<tr><td>Hold Power</td><td align=center>") ; 
  if (( ghks.SolContPower & 0x8000 ) != 0 ){
    MyCheck = F("CHECKED")  ;  
  }else{
    MyCheck = "" ;      
  }
  message += "<input type='text' name='hprb' value='" + String(( ghks.SolContPower >> 4) & 0x0f ) + "' size=4 maxlength=2>" ;
  message += "<input type='text' name='hprz' value='" + String(( ghks.SolContPower & 0x0f )) + "' size=4 maxlength=2>" ;
  message += " Enable <input type='checkbox' name='hpre' "+String(MyCheck)+ ">";    
  if (( ghks.SolContPower & 0x4000  )!= 0 ){       // polarity of the pulse
    message +=  "<select name='hprp'><option value='0'>Pos<option value='1' SELECTED>Neg</select>" ;    
  }else{
    message +=  "<select name='hprp'><option value='0' SELECTED>Pos<option value='1'>Neg</select>" ;            
  }
  message += F("</td><td></td></tr>\r\n");

   message += F("<tr><td>Master Power</td><td align=center>") ; 
  if (( ghks.SolMastPower & 0x8000 ) != 0 ){
    MyCheck = F("CHECKED")  ;  
  }else{
    MyCheck = "" ;      
  }
  message += "<input type='text' name='mprb' value='" + String(( ghks.SolMastPower >> 4) & 0x0f ) + "' size=4 maxlength=2>" ;
  message += "<input type='text' name='mprz' value='" + String(( ghks.SolMastPower & 0x0f )) + "' size=4 maxlength=2>" ;
  message += " Enable <input type='checkbox' name='mpre' "+String(MyCheck)+ ">";    
  if (( ghks.SolMastPower & 0x4000  )!= 0 ){       // polarity of the pulse
    message +=  "<select name='mprp'><option value='0'>Pos<option value='1' SELECTED>Neg</select>" ;    
  }else{
    message +=  "<select name='mprp'><option value='0' SELECTED>Pos<option value='1'>Neg</select>" ;            
  }
  message += F("</td><td></td></tr>\r\n");
 
  message += F("</form><tr><td colspan=3></td></tr>");


  server.sendContent(message) ;
  message = "<form method=post action=" + server.uri() + ">" ;
      
  message += F("<tr><td>CPU Frequency</td><td align=center>") ; 
  message += F("<select name='cpufr'>") ;
  for ( j = 0 ; j <= 4 ; j++ ) {
    strOption = CPUSpeedTextLUT(j,&ii);
    if ( ghks.cpufreq == ii ){
      strSelected = "SELECTED" ;
    }else{
      strSelected = "" ;
    }
    message += "<option value='"+String(ii)+"' " + strSelected + ">" + strOption + "\r\n"; 
  }
  message += F("</select></td><td>(MHz)<input type='submit' value='SET'></td></tr>\r\n");

  message += F("<tr><td>Display Option</td><td align=center>") ; 
  message += F("<select name='disop'>") ;
  if ((ghks.lDisplayOptions & 0x01 ) == 0 ){
    message += F("<option value='0' SELECTED>0 - Normal"); 
    message += F("<option value='1'>1 - Invert"); 
  }else{
    message += F("<option value='0'>0 - Normal"); 
    message += F("<option value='1' SELECTED>1 - Invert"); 
  }
  message += F("</select></td><td></td></tr>\r\n");

  message += F("<tr><td>Display Power Option</td><td align=center>") ; 
  message += F("<select name='disro'>") ;
  if (((ghks.lDisplayOptions & 0x02 ) >> 1 ) == 0 ){
    message += F("<option value='0' SELECTED>0 - Always On"); 
    message += F("<option value='1'>1 - Go off with WiFi"); 
  }else{
    message += F("<option value='0'>0 - Always On"); 
    message += F("<option value='1' SELECTED>1 - Go off with WiFi"); 
  }
  message += F("</select></td><td></td></tr>\r\n");

  message += F("<tr><td>Turn Wifi and Display off after</td><td align=center>") ; 
  message += "<input type='text' name='dotmr' value='" + String(ghks.displaytimer) + "' size=4 maxlength=3></td><td>(min) (-1) - disables</td></tr>\r\n";

  message += F("<tr><td>Mag Switch Level</td><td align=center>") ; 
  message += "<input type='text' name='magsn' value='" + String(ghks.magsens) + "' size=4 maxlength=5></td><td>(?)</td></tr>\r\n";

  message += F("<tr><td>Display Number of Valves</td><td align=center>") ; 
  message += "<input type='text' name='maxvn' value='" + String(ghks.lMaxDisplayValve) + "' size=4 maxlength=2></td><td>2 - "+String(MAX_VALVE)+"</td></tr>\r\n";

  message += F("<tr><td>Program Method</td><td align=center>") ; 
  message += F("<select name='prgop'>") ;
  if (ghks.lProgMethod == 0 ){
    message += F("<option value='0' SELECTED>0 By Valve"); 
    message += F("<option value='1'>1 By Progam / Shift"); 
  }else{
    message += F("<option value='0'>0 By Valve"); 
    message += F("<option value='1' SELECTED>1 By Progam / Shift"); 
  }
  message += F("</select></td><td></td></tr></form>\r\n");
  server.sendContent(message) ;
  message = "" ;
  
  message += "<form method=post action=" + server.uri() + "><tr><td colspan=3></td></tr>\r\n" ; 

  message += F("<tr><td>Local UDP Port NTP</td><td align=center>") ; 
  message += "<input type='text' name='lpntp' value='" + String(ghks.localPort) + "' size=12></td><td><input type='submit' value='SET'></td></tr>\r\n";

  message += F("<tr><td>Local UDP Port Control</td><td align=center>") ; 
  message += "<input type='text' name='lpctr' value='" + String(ghks.localPortCtrl) + "' size=12></td><td></td></tr>\r\n";

  message += F("<tr><td>Remote UDP Port Control</td><td align=center>") ; 
  message += "<input type='text' name='rpctr' value='" + String(ghks.RemotePortCtrl) + "' size=12></td><td></td></tr>\r\n";

  message += F("<tr><td>Network SSID</td><td align=center>") ; 
  message += "<input type='text' name='nssid' value='" + String(ghks.nssid) + "' maxlength=15 size=12></td><td></td></tr>\r\n";

  message += F("<tr><td>Network Password</td><td align=center>") ; 
  message += "<input type='text' name='npass' value='" + String(ghks.npassword) + "' maxlength=15 size=12></td><td></td></tr>\r\n";

  message += F("<tr><td>Configure Password</td><td align=center>") ; 
  message += "<input type='text' name='cpass' value='" + String(ghks.cpassword) + "' maxlength=15 size=12></td><td></td></tr>\r\n";

  message += F("<tr><td>Time Server</td><td align=center>") ; 
  message += "<input type='text' name='timsv' value='" + String(ghks.timeServer) + "' maxlength=23 size=12></td><td></td></tr>\r\n";

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.RCIP[0],ghks.RCIP[1],ghks.RCIP[2],ghks.RCIP[3]);
  message += F("<tr><td>Remote IP Address Control</td><td align=center title='0.0.0.1 client wifi  0.0.0.2 SoftaAP  0.0.0.0 disable  x.x.x.x actual IP  x.x.x.255 broadcast'> ") ; 
  message += "<input type='text' name='rpcip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr></form>";
  server.sendContent(message) ;
  message = "" ;
  message += F("<tr><td colspan=3></td></tr>\r\n") ;

  message += "<tr><td><b>LoRa Stuff</b></td><td><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='369'><input type='submit' value='Reset LoRa Parameters'></form></td><td><form method=post action=" + server.uri() + "><input type='submit' value='SET'></td></tr>\r\n" ; 

  message += F("<tr><td>Frequency</td><td align=center>") ; 
  message += "<input type='text' name='lrfre' value='" + String(ghks.iFreq) + "' maxlength=4 size=12></td><td>(MHz) x10</td></tr>\r\n";

  message += F("<tr><td>Spreading Factor</td><td align=center>") ; 
  message += "<input type='text' name='lrspr' value='" + String(ghks.iSpread) + "' maxlength=2 size=12></td><td>(?)6-12</td></tr>\r\n";
  
  message += F("<tr><td>Power</td><td align=center>") ; 
  message += "<input type='text' name='lrpwr' value='" + String(ghks.iTXPower) + "' maxlength=2 size=12></td><td>(dB) 2-20</td></tr>\r\n";

  message += F("<tr><td>Bandwidth</td><td align=center>") ; 
  message += "<select name='lrban'>" ;
  for ( j = 0 ; j <= 9 ; j++ ) {
    if ( ghks.iBandWidth == j ){
      strSelected = "SELECTED" ;
    }else{
      strSelected = "" ;
    }
    message += "<option value='"+String(j)+"' " + strSelected + ">"+BandWidthText(j)+"\r\n"; 
  }
  message += "</select></td><td>(KBps)</td></tr></form>";
//  message += "<tr><td colspan=3 align='center'><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='369'><input type='submit' value='Reset LoRa Parameters'></form></td></tr>\r\n" ; 
  
  server.sendContent(message) ;
  message = "" ;

  message += "<form method=postaction=" + server.uri() + "><tr><td colspan=3></td></tr>\r\n" ; 

  message += F("<tr><td><b>Network Options</b></td><td align=center>") ; 
  message += F("<select name='netop'>") ;
  if ((ghks.lNetworkOptions & 0x01 ) == 0 ){
    message += F("<option value='0' SELECTED>0 - DHCP"); 
    message += F("<option value='1'>1 - Static"); 
  }else{
    message += F("<option value='0'>0 - DHCP"); 
    message += F("<option value='1' SELECTED>1 - Static IP"); 
  }
  message += F("</select></td><td><input type='submit' value='SET'></td></tr>");
  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPStatic[0],ghks.IPStatic[1],ghks.IPStatic[2],ghks.IPStatic[3]);
  message += F("<tr><td>Static IP Address</td><td align=center>") ; 
  message += "<input type='text' name='staip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>";

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPGateway[0],ghks.IPGateway[1],ghks.IPGateway[2],ghks.IPGateway[3]);
  message += F("<tr><td>Gateway IP Address</td><td align=center>") ; 
  message += "<input type='text' name='gatip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>";

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPMask[0],ghks.IPMask[1],ghks.IPMask[2],ghks.IPMask[3]);
  message += F("<tr><td>IP Mask</td><td align=center>") ; 
  message += "<input type='text' name='mskip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>";

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPDNS[0],ghks.IPDNS[1],ghks.IPDNS[2],ghks.IPDNS[3]);
  message += F("<tr><td>DNS IP Address</td><td align=center>") ; 
  message += "<input type='text' name='dnsip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>";

  message += "<tr><td>Last Scan Speed</td><td align=center>" + String(lScanLast) + "</td><td>(per second)</td></tr>"  ;    
  if( hasRTC ){
    rtc_status = DS3231_get_sreg();
    if (( rtc_status & 0x80 ) != 0 ){
      message += F("<tr><td>RTC Battery</td><td align=center bgcolor='red'>DEPLETED</td><td></td></tr>") ;            
    }else{
      message += F("<tr><td>RTC Battery</td><td align=center bgcolor='green'>-- OK --</td><td></td></tr>") ;                    
    }
    message += "<tr><td>RTC Temperature</td><td align=center>"+String(rtc_temp,1)+"</td><td>(C)</td></tr>" ;                    
  }
  message += F("</form></table>");
  server.sendContent(message) ;
  message = "" ;

  SendHTTPPageFooter();
}


void IndicateReboot(void){
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);  
    display.setFont(ArialMT_Plain_16);
    display.drawString(63, 20, "AUTO");
    display.drawString(63, 36, "REBOOT");
    display.setFont(ArialMT_Plain_10);
    display.setColor(INVERSE);
    display.fillRect(0, 0, 128, 11);
    display.fillRect(0, 53, 128, 11);
    display.display();    
    ESP.restart() ;
}


String CPUSpeedTextLUT(int Option , int *speedvalue){
String tmpStr = "" ;

  switch(Option){
    case 0:
      tmpStr = "13 Mhz - Glacial - Danger Will Robinson" ;
      *speedvalue = 13 ;
    break;    
    case 1:
      tmpStr = "26 Mhz - Sloooow - Danger Will Robinson" ;
      *speedvalue = 26 ;
    break;    
    case 2:
      tmpStr = "80 Mhz - Slow - Low Power" ;
      *speedvalue = 80 ;
    break;    
    case 3:
      tmpStr = "160MHz" ;
      *speedvalue = 160 ;     
    break;  
    case 4:  
    default:
      tmpStr = "240MHz - Fast - High Power" ;
      *speedvalue = 240 ;
    break;    
  }
  return(tmpStr);  
}

String BandWidthText(uint8_t bw_no){
String tmpStr = "" ;  
  switch(bw_no){
    case 0:
      tmpStr = "7.8" ;
    break;
    case 1:
      tmpStr = "10.4" ;
    break;
    case 2:
      tmpStr = "15.6" ;
    break;
    case 3:
      tmpStr = "20.8" ;
    break;
    case 4:
      tmpStr = "31.25" ;
    break;
    case 5: 
      tmpStr = "41.7" ;
    break;
    case 6:
      tmpStr = "62.5" ;
    break;
    case 7:
      tmpStr = "125" ;
    break;
    case 8:
      tmpStr = "250" ;
    break;
    default:
      tmpStr = "500" ;
    break;
  }
  return(tmpStr);
}


