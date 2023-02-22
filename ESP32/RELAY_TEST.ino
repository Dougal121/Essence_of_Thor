void handleBTest(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
 

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
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
  
  SendHTTPHeader();

  if ( iTestMode == -1 ){
    MyColor = F("TEST") ;
  }else{
    MyColor = F("ABORT") ;
  }
  server.sendContent(F("<br><br><b>Test Relay Board</b><br><table border=1 title='Test Coils and Relay Board'>"));
  server.sendContent("<tr><th>Parameter</th><th>Value</th><th><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='42'><input type='submit' value='"+MyColor+"'></form></th></tr>");
  server.sendContent("<tr><td><form method=post action=" + server.uri() + ">Test Boad Number</td><td><input type='text' name='testb' value='"+ String(iTestBoard) + "' maxlength=2 size=4 align='center'>");
  server.sendContent(F("</td><td><input type='submit' value='SET'></form></td></tr>"));
  server.sendContent("<tr><td><form method=post action=" + server.uri() + ">Test Start/Coil Number</td><td><input type='text' name='testc' value='"+ String(iTestCoil) + "' maxlength=2 size=4 align='center'>");
  server.sendContent(F("</td><td><input type='submit' value='SET'></form></td></tr>"));   
  server.sendContent("<tr><td><form method=post action=" + server.uri() + ">Time between Coil Tests (s)</td><td><input type='text' name='testt' value='"+ String(iTestTime) + "' maxlength=2 size=4 align='center'>");
  server.sendContent(F("</td><td><input type='submit' value='SET'></form></td></tr>"));
  server.sendContent("</td><td><form method=post action=" + server.uri() + ">Test Mode</td><td><select name='tstinc'>") ;
  if (iTestInc == 1 ){
    server.sendContent(F("<option value='0' SELECTED>0 - Increment After Coil Test")); 
    server.sendContent(F("<option value='1'>1 - Test same coil")); 
  }else{
    server.sendContent(F("<option value='0'>0 - Increment After Coil Test")); 
    server.sendContent(F("<option value='1' SELECTED>1 - Test same coil")); 
  }
  server.sendContent(F("</select></td><td><input type='submit' value='SET'></form></td></tr>"));

  if ( iTestMode == -1 ){
    server.sendContent(F("</td><td colspan=3 align='center' bgcolor='ltgreen'><b>Normal Operation</b></td></tr>"));
  }else{
    server.sendContent("</td><td colspan=3 align='center' bgcolor='yellow'><b>Test In Progres - Coil "+String(iTestCoil)+"</b></td></tr>");  
  }
  server.sendContent(F("</table>"));

  SendHTTPPageFooter();  
}



void handleVSSS(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
  long timediff ;

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){

  }
  
  SendHTTPHeader();

  server.sendContent(F("<br><b>Fertigation Status</b>"));
  message = F("<table border=1 title='Fertigation Status'>") ;
  message += F("<tr><th>Tank</th><th>Run</th><th>Enable</th><th>On/Off</th><th>lTTG</th><th>Flowrate</th></tr>") ; 
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
    message += "<td>" + String(vfert[i].Flowrate) + "</td><tr>";
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;

  message = F("<br><b>Filter Status</b><br><table border=1 title='Filter Status'>") ;
  message += F("<tr><th>Filter</th><th>Next Can</th><th>bOnOff</th><th>bFlush</th><th>lTTG</th></tr>") ; 
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
    message += "<td>" + String(vfilter[i].lTTG) + "</td><tr>";
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;

  message = F("<br><b>Valve Status</b><br><table border=1 title='Valve Status'>") ;
  message += F("<tr><th>Valve</th><th>bOnOff</th><th>lATTG</th><th>lTTG</th><th>iFB</th></tr>") ; 
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
    message += "<td>" + String(vvalve[i].iFB,HEX) + "</td><tr>";
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;


  message = F("<br><b>CNC Node Status</b><br><table border=1 title='CNC Node Status'>") ;
  message += F("<tr><th>Record</th><th>Node</th><th>RSSI</th><th>SNR</th><th>RX Time</th><th>Total</th><th>Uplinked</th></tr>") ; 
  server.sendContent(message) ;
  message = "" ;
  for (i = 0 ; i < MAX_REM_LIST ; i++ ) {   // enumerate the remote nodes list
    if ( remlist[i].node > -1 ){
      timediff = abs(remlist[i].rxt - now());
      MyColor = F("");
      if ( timediff > 600 ) {
        MyColor = F("bgcolor=red") ;
      }else{
        MyColor = F("bgcolor=green") ;
      }
      message += "<tr><td >"+String(i)+"</td>";         
      message += "<td>" + String(remlist[i].node) + "</td>";
      message += "<td>" + String(remlist[i].Rssi) + "</td>";
      message += "<td>" + String(remlist[i].Snr,1) + "</td>";
      snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(remlist[i].rxt), month(remlist[i].rxt), day(remlist[i].rxt) , hour(remlist[i].rxt), minute(remlist[i].rxt), second(remlist[i].rxt));              
      message += "<td align=center "+String(MyColor)+">" + String(buff) + "</td>";
      message += "<td>" + String(remlist[i].total) + "</td>";
      message += "<td>" + String(remlist[i].uplinked) + "</td><tr>";
    }
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;
  SendHTTPPageFooter();  
}



void handleSTime(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  byte mac[6];
  tmElements_t tm;
 

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
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

  }
  
  SendHTTPHeader();

  snprintf(buff, BUFF_MAX, "%04d/%02d/%02d %02d:%02d", year(), month(), day() , hour(), minute());
  server.sendContent("<br><br><form method=post action=" + server.uri() + "><br>Set Current Time: <input type='text' name='stime' value='"+ String(buff) + "' size=12>");
  server.sendContent(F("<input type='submit' value='SET'><br><br></form>"));
  
  SendHTTPPageFooter();  
}


