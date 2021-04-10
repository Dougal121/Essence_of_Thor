#define MESSAGE_MAX 1024 


void SendEmailToClient(int iMessageID){
char csTemp[MESSAGE_MAX] ;
int i ;
String strMyTmp ;
IPAddress MyIP;
bool bRoach = false ;
//  Serial.println("Send Email Called");

  MyIP =  WiFi.localIP() ;                  
  if (WiFi.isConnected() && (MyIP[0] != 0 )&& (MyIP[3] != 0 ))  {
    if (( String(SMTP.FROM).length() >5 )&&( String(SMTP.TO).length() >5 )){
//      Serial.println( "Going into the send - MessageID " + String(iMessageID) );
      snprintf(csTemp,MESSAGE_MAX,"\0"  )  ;  
      for ( i = 0 ; i < MAX_FERT ; i++){
        switch(iMessageID){
            case -2:  // bootup/reboot
                snprintf(csTemp,MESSAGE_MAX,"%s - Just Booted up or Rebooted \0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break;   
            case -1:  // test
                snprintf(csTemp,MESSAGE_MAX,"%s - Test Email \0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break;   
            case 0:  // tank empty
              if ( efert[i].CurrentQty <= 0 ){
                if ( !bRoach ){
                  snprintf(csTemp,MESSAGE_MAX,"%s - Out of Chemical in Tank \0",ghks.NodeName   )  ;  
                  bRoach = true ;
                }else{
                  snprintf(buff,BUFF_MAX," \& \0"  )  ;  
                  strcat(csTemp,buff) ;                        
                }
                snprintf(buff,BUFF_MAX," %d %s \0" , i+1 , efert[i].description )  ;  
                strcat(csTemp,buff) ;        
              }
            break;   
            case 1:  // tank low
              if (( efert[i].CurrentQty <= SMTP.LowTankQty )&& ( efert[i].CurrentQty > 0 )){
                if ( !bRoach ){
                  snprintf(csTemp,MESSAGE_MAX,"%s - Chemical Low in Tank \0" ,ghks.NodeName  )  ;  
                  bRoach = true ;
                }else{
                  snprintf(buff,BUFF_MAX," \& \0"  )  ;  
                  strcat(csTemp,buff) ;                                          
                }
                snprintf(buff,BUFF_MAX," %d %s \0", i+1 , efert[i].description )  ;
                strcat(csTemp,buff) ;        
              }
            break; 
            case 2:  // irrigation line still presurised after shut off  
            break ; 
            case 3:  // irriagtion line failed to pressurise after startup
            break ;
            case 4:  // main soleniod CB trip
            break ;
            default: snprintf(csTemp,MESSAGE_MAX,"%s \0",SMTP.subject ) ; 
                i = MAX_FERT ;
            break ;
        }
      }
//      Serial.println("Setting Subject >" + String(csTemp)+"<");
  
      if ( String(SMTP.FROM).length() < 6 ){
        WDmail.setSubject("doby@house.elf", csTemp);      
      }else{
        WDmail.setSubject( SMTP.FROM , csTemp );
      }
  //    Serial.println("Setting To");
      WDmail.addTo( SMTP.TO );
      if ( String(SMTP.CC).length() > 5 ){
        WDmail.addCC(SMTP.CC);
      }
      if ( String(SMTP.BCC).length() > 5 ){
        WDmail.addBCC(SMTP.BCC );
      }
      
      if (ghks.lProgMethod == 0 ){
        snprintf(buff,BUFF_MAX,"\r\nMode By Valve \r\n\r\n\0" ) ;
      }else{
        snprintf(buff,BUFF_MAX,"\r\nMode By Progam / Shift \r\n\r\n\0" ) ;
      }
      strcat(csTemp,buff ) ;
  //    Serial.println("Assembling 1 ");
      
      for ( i = 0 ; i < MAX_FERT ; i++){
        if (( efert[i].DaysEnable & ( 0x80 )) != 0  ){
          snprintf(buff,BUFF_MAX,"Tank %02d  %s  %3.1f \r\n\0",i+1,efert[i].description,efert[i].CurrentQty); 
          strcat(csTemp,buff) ;        
        }
      }
      strcat(csTemp,"\r\n\0") ;
  //    Serial.println("Assembling 2 ");
  
      for (i = 0 ; i < MAX_VALVE  ; i++ ) {   // setup the defaults in the 3 volitile structs that support data comin back from eeprom
        if ( vvalve[i].bOnOff ){   // valve on
          strMyTmp = " ON - " + String(vvalve[i].lTTG) + " - " +String(vvalve[i].lATTG) + " \0" ;
        }else{
          strMyTmp = " OFF \0" ;      
        }
        snprintf(buff,BUFF_MAX,"Valve %02d  %s - %s \r\n\0",i+1,evalve[i].description, strMyTmp.c_str()); 
        strcat(csTemp,buff) ;  
      }
  //    Serial.println("Assembling 3 ");
      
      snprintf(buff, BUFF_MAX, "\r\n %d/%02d/%02d %02d:%02d:%02d \r\n\0", year(), month(), day() , hour(), minute(), second());
      strcat(csTemp,buff) ;
      snprintf(buff, BUFF_MAX,"Node ID %08X\r\nIP %03u.%03u.%03u.%03u \r\n\0",ESP.getChipId(), MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
      strcat(csTemp,buff) ;
      snprintf(buff, BUFF_MAX, "CPU Uptime %d:%02d:%02d (Days:Hrs:Min)\r\n\r\n\0",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
      strcat(csTemp,buff) ;
      
//      Serial.println("Setting Body >" + String(csTemp) + "<");
      WDmail.setBody(csTemp);
  
  //    Serial.println("Server: " + String(SMTP.server));
  //    Serial.println("Port: " + String(SMTP.port));
  //    Serial.println("User: " + String(SMTP.user));
  //    Serial.println("Password: " + String(SMTP.password));
  
      WDmail.enableDebugMode();
      if ( SMTP.user[0] == 0 ){
        lRet_Email = WDmail.send(SMTP.server , SMTP.port , NULL , NULL ) ;
        if (lRet_Email == 0){
          Serial.println("Mail send OK");
        }
      }else{
        lRet_Email = WDmail.send(SMTP.server , SMTP.port , SMTP.user , SMTP.password );
        if (lRet_Email == 0){
          Serial.println("Mail send OK");
        }    
      }
      
    }else{
      Serial.println("Mail not set up proper like dude...");
    }
  }
}

void DisplayEmailSetup() {
  long  i = 0 ;
  int ii  ;
  int j ;
  int k , kk  ;
  String message ;
  String MyCheck ;

  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 11:  // load values
          bSendTestEmail = true ;
        break;
        case 121:
          ResetSMTPInfo();
        break;        
      }
    }
    i = String(server.argName(j)).indexOf("smse");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.server , sizeof(SMTP.server)) ;
    }
    i = String(server.argName(j)).indexOf("smus");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.user , sizeof(SMTP.user)) ;
    }
    i = String(server.argName(j)).indexOf("smpa");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.password , sizeof(SMTP.password)) ;
    }
    i = String(server.argName(j)).indexOf("smfr");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.FROM , sizeof(SMTP.FROM)) ;
    }
    i = String(server.argName(j)).indexOf("smto");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.TO , sizeof(SMTP.TO)) ;
    }
    i = String(server.argName(j)).indexOf("smcc");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.CC , sizeof(SMTP.CC)) ;
    }
    i = String(server.argName(j)).indexOf("smbc");
    if (i != -1){  // have a request to request a time update
     String(server.arg(j)).toCharArray( SMTP.BCC , sizeof(SMTP.BCC)) ;
    }
    i = String(server.argName(j)).indexOf("smmb");
    if (i != -1){  
     String(server.arg(j)).toCharArray( SMTP.message , sizeof(SMTP.message)) ;
    }
    i = String(server.argName(j)).indexOf("smsj");
    if (i != -1){  
     String(server.arg(j)).toCharArray( SMTP.subject , sizeof(SMTP.subject)) ;
    }
    i = String(server.argName(j)).indexOf("smbz");
    if (i != -1){  //  
        SMTP.bSecure = false ;        
    }
    i = String(server.argName(j)).indexOf("smbs");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        SMTP.bSecure = true ;        
      }
    }  
    i = String(server.argName(j)).indexOf("smpo");
    if (i != -1){ 
      SMTP.port = String(server.arg(j)).toInt() ;
    }       
    i = String(server.argName(j)).indexOf("lotk");
    if (i != -1){ 
      SMTP.LowTankQty= String(server.arg(j)).toInt() ;
    }       

    i = String(server.argName(j)).indexOf("smuz");
    if (i != -1){  //  
        SMTP.bUseEmail = false ;        
    }
    i = String(server.argName(j)).indexOf("smue");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        SMTP.bUseEmail = true ;        
      }
    }     
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######

  message = "";
  message = F("<table border=1 title='Email setup for SCADA'>") ;
  message += F("<tr><th>Email Parameter</th><th>Value</th><th>Units</th><th>.</th></tr>") ;              
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Port</td><td align=center title='Popular Values 25 , 465 , 2525 , 587'><input type='text' name='smpo' value='"+String(SMTP.port)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Server</td><td align=center><input type='text' name='smse' value='"+String(SMTP.server)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP User</td><td align=center><input type='text' name='smus' value='"+String(SMTP.user)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Password</td><td align=center><input type='text' name='smpa' value='"+String(SMTP.password)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP FROM</td><td align=center><input type='text' name='smfr' value='"+String(SMTP.FROM)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP TO</td><td align=center><input type='text' name='smto' value='"+String(SMTP.TO)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP CC</td><td align=center><input type='text' name='smcc' value='"+String(SMTP.CC)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP BCC</td><td align=center><input type='text' name='smbc' value='"+String(SMTP.BCC)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Subject</td><td align=center><input type='text' name='smsj' value='"+String(SMTP.subject)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Message</td><td align=center><input type='text' name='smmb' value='"+String(SMTP.message)+"' size=30></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;

  if ( ( SMTP.bSecure ) != 0 ){
    MyCheck = F("CHECKED")  ;    
  }else{
    MyCheck = F("")  ;    
  }
  message += "<tr><form method=post action=" + server.uri() + "><td>SMTP Secure<input type='hidden' name='smbz' value='0'></td><td align=center><input type='checkbox' name='smbs' " + String(MyCheck)+ "></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><td>Last Email Status</td><td>"+String(lRet_Email)+"</td><td colspan=2>.</td></tr>" ;
  message += "<tr></td><td colspan=4>.</td></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td title='-1 to disable alarm emails'>Tank Low Alarm Volume</td><td align=center><input type='text' name='lotk' value='"+String(SMTP.LowTankQty)+"' size=30></td><td>(L)</td><td><input type='submit' value='SET'></td></form></tr>" ;
  if ( ( SMTP.bUseEmail ) != 0 ){
    MyCheck = F("CHECKED")  ;    
  }else{
    MyCheck = F("")  ;    
  }
  message += "<tr><form method=post action=" + server.uri() + "><td>Use Email Alarm Messaging<input type='hidden' name='smuz' value='0'></td><td align=center><input type='checkbox' name='smue' " + String(MyCheck)+ "></td><td>.</td><td><input type='submit' value='SET'></td></form></tr>" ;
  message += "<tr><form method=post action=" + server.uri() + "><td colspan=4 align='center'><input type='hidden' name='command' value='11'><input type='submit' value='### SEND TEST EMAIL ###'></td></form></tr>" ;
  message += "<tr><td colspan=4 align='center'><form method=post action=" + server.uri() + "><input type='hidden' name='command' value='121'><input type='submit' value='Clear / RESET EMAIL Settings'></form></td></tr>" ; 
  message += "</table>" ;
  
  server.sendContent(message) ;

  SendHTTPPageFooter();

}


void ResetSMTPInfo(){
  SMTP.port = 25 ;                // 25 , 465 , 2525 , 587
  sprintf(SMTP.server,"203.36.137.241\0") ;   // smtp.telstrabusiness.com 
  sprintf(SMTP.user,"\0") ;
  sprintf(SMTP.password,"\0") ;
  sprintf(SMTP.FROM,"dougal@plummer.com.au\0") ;
  sprintf(SMTP.FROM,"\0") ;
  sprintf(SMTP.TO,"dougal@plummersoftware.onmicrosoft.com\0") ;
  sprintf(SMTP.TO,"\0") ;
  sprintf(SMTP.CC,"\0") ;
  sprintf(SMTP.BCC,"\0") ;
  sprintf(SMTP.message,"Test Email\0") ;
  sprintf(SMTP.subject,"Node %08X\0",ESP.getChipId()) ;
  SMTP.bSecure = false ;
  SMTP.bUseEmail = false ;
  SMTP.LowTankQty = 50.0 ;   
}


