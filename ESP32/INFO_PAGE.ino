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
//  server.sendContent("<tr><td>Get Rest Reason</td><td align=center>" + String(ESP.getResetReason()) + "</td><td></td></tr>" ) ;    
//  server.sendContent("<tr><td>Get Reset Into</td><td align=center>" + String(ESP.getResetInfo()) + "</td><td></td></tr>" ) ;    
//  server.sendContent("<tr><td>Get Sketch Size</td><td align=center>" + String(ESP.getSketchSize()) + "</td><td>(Bytes)</td></tr>" ) ;    
//  server.sendContent("<tr><td>Free Sketch Space</td><td align=center>" + String(ESP.getFreeSketchSpace()) + "</td><td>(Bytes)</td></tr>" ) ;    
  snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
  server.sendContent("<tr><td>Computer Uptime</td><td align=center>"+String(buff)+"</td><td>(day:hr:min)</td></tr>" ) ;

  server.sendContent(F("</table><br>"));    
  SendHTTPPageFooter();
}


void handleSetup(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
 

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("ptime");
    if (i != -1){  // 
      ghks.lPulseTime = String(server.arg(j)).toInt() ;
      ghks.lPulseTime = constrain(ghks.lPulseTime,10,1000);
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

    
  }
  
  SendHTTPHeader();

  server.sendContent("<form method=post action=" + server.uri() + "><table border=1 title='Node Settings'>");
  server.sendContent(F("<tr><th>Parameter</th><th>Value</th><th><input type='submit' value='SET'></th></tr>"));

  server.sendContent(F("<tr><td>Controler Name</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='cname' value='"+String(ghks.NodeName)+"' maxlength=15 size=12></td><td></td></tr>");

  snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(ghks.AutoOff_t), month(ghks.AutoOff_t), day(ghks.AutoOff_t) , hour(ghks.AutoOff_t), minute(ghks.AutoOff_t));
  if (ghks.AutoOff_t > now()){
    MyColor =  F("bgcolor=red") ;
  }else{
    MyColor =  "" ;
  }
  server.sendContent("<tr><td "+String(MyColor)+">Auto Off Until</td><td align=center>") ; 
  server.sendContent("<input type='text' name='atoff' value='"+ String(buff) + "' size=12></td><td>(yyyy/mm/dd)</td></tr>");

  server.sendContent(F("<tr><td>Activation Pulse</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='ptime' value='" + String(ghks.lPulseTime) + "' size=12></td><td>(ms)</td></tr>");

  server.sendContent(F("<tr><td>Node Address</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='ndadd' value='" + String(ghks.lNodeAddress) + "' size=12></td><td>"+String(ghks.lNodeAddress & 0xff)+"</td></tr>");

  server.sendContent(F("<tr><td>Time Zone</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='tzone' value='" + String(ghks.fTimeZone,1) + "' size=12></td><td>(Hours)</td></tr>");

  server.sendContent(F("<tr><td>Display Options</td><td align=center>")) ; 
  server.sendContent(F("<select name='disop'>")) ;
  if (ghks.lDisplayOptions == 0 ){
    server.sendContent(F("<option value='0' SELECTED>0 - Normal")); 
    server.sendContent(F("<option value='1'>1 - Invert")); 
  }else{
    server.sendContent(F("<option value='0'>0 - Normal")); 
    server.sendContent(F("<option value='1' SELECTED>1 - Invert")); 
  }
  server.sendContent(F("</select></td><td></td></tr>"));

  server.sendContent(F("<tr><td>Display Number of Valves</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='maxvn' value='" + String(ghks.lMaxDisplayValve) + "' size=4 maxlength=2></td><td>2 - "+String(MAX_VALVE)+"</td></tr>");

  server.sendContent(F("<tr><td>Program Method</td><td align=center>")) ; 
  server.sendContent(F("<select name='prgop'>")) ;
  if (ghks.lProgMethod == 0 ){
    server.sendContent(F("<option value='0' SELECTED>0 By Valve")); 
    server.sendContent(F("<option value='1'>1 By Progam / Shift")); 
  }else{
    server.sendContent(F("<option value='0'>0 By Valve")); 
    server.sendContent(F("<option value='1' SELECTED>1 By Progam / Shift")); 
  }
  server.sendContent(F("</select></td><td></td></tr></form>"));

  server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ; 

  server.sendContent(F("<tr><td>Local UDP Port NTP</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='lpntp' value='" + String(ghks.localPort) + "' size=12></td><td><input type='submit' value='SET'></td></tr>");

  server.sendContent(F("<tr><td>Local UDP Port Control</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='lpctr' value='" + String(ghks.localPortCtrl) + "' size=12></td><td></td></tr>");

  server.sendContent(F("<tr><td>Remote UDP Port Control</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='rpctr' value='" + String(ghks.RemotePortCtrl) + "' size=12></td><td></td></tr>");

  server.sendContent(F("<tr><td>Network SSID</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='nssid' value='" + String(ghks.nssid) + "' maxlength=15 size=12></td><td></td></tr>");

  server.sendContent(F("<tr><td>Network Password</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='npass' value='" + String(ghks.npassword) + "' maxlength=15 size=12></td><td></td></tr>");

  server.sendContent(F("<tr><td>Configure Password</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='cpass' value='" + String(ghks.cpassword) + "' maxlength=15 size=12></td><td></td></tr>");

  server.sendContent(F("<tr><td>Time Server</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='timsv' value='" + String(ghks.timeServer) + "' maxlength=23 size=12></td><td></td></tr>");

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.RCIP[0],ghks.RCIP[1],ghks.RCIP[2],ghks.RCIP[3]);
  server.sendContent(F("<tr><td>Remote IP Address Control</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='rpcip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr></form>");
  
  server.sendContent("<form method=post action=" + server.uri() + "><tr><td></td><td></td><td></td></tr>") ; 

  server.sendContent(F("<tr><td>Network Options</td><td align=center>")) ; 
  server.sendContent(F("<select name='netop'>")) ;
  if (ghks.lNetworkOptions == 0 ){
    server.sendContent(F("<option value='0' SELECTED>0 - DHCP")); 
    server.sendContent(F("<option value='1'>1 - Static")); 
  }else{
    server.sendContent(F("<option value='0'>0 - DHCP")); 
    server.sendContent(F("<option value='1' SELECTED>1 - Static IP")); 
  }
  server.sendContent(F("</select></td><td><input type='submit' value='SET'></td></tr>"));
  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPStatic[0],ghks.IPStatic[1],ghks.IPStatic[2],ghks.IPStatic[3]);
  server.sendContent(F("<tr><td>Static IP Address</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='staip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPGateway[0],ghks.IPGateway[1],ghks.IPGateway[2],ghks.IPGateway[3]);
  server.sendContent(F("<tr><td>Gateway IP Address</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='gatip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPMask[0],ghks.IPMask[1],ghks.IPMask[2],ghks.IPMask[3]);
  server.sendContent(F("<tr><td>IP Mask</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='mskip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

  snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", ghks.IPDNS[0],ghks.IPDNS[1],ghks.IPDNS[2],ghks.IPDNS[3]);
  server.sendContent(F("<tr><td>DNS IP Address</td><td align=center>")) ; 
  server.sendContent("<input type='text' name='dnsip' value='" + String(buff) + "' maxlength=16 size=12></td><td></td></tr>");

  server.sendContent("<tr><td>Last Scan Speed</td><td align=center>" + String(lScanLast) + "</td><td>(per second)</td></tr>" ) ;    
  if( hasRTC ){
    rtc_status = DS3231_get_sreg();
    if (( rtc_status & 0x80 ) != 0 ){
      server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='red'>DEPLETED</td><td></td></tr>")) ;            
    }else{
      server.sendContent(F("<tr><td>RTC Battery</td><td align=center bgcolor='green'>-- OK --</td><td></td></tr>")) ;                    
    }
    server.sendContent("<tr><td>RTC Temperature</td><td align=center>"+String(rtc_temp,1)+"</td><td>(C)</td></tr>") ;                    
  }
  server.sendContent(F("</form></table>"));

  SendHTTPPageFooter();
}

