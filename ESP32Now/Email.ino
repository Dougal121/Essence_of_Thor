#define MESSAGE_MAX 1280 


void SendEmailToClient(int iMessageID){
char csTemp[MESSAGE_MAX] ;
int i ;
String strMyTmp ;
IPAddress MyIP;
bool bRoach = false ;
  
//  Serial.println("Send Email Called");
  MyIP =  WiFi.localIP() ;                  
  if (WiFi.isConnected() && (MyIP[0] != 0 )&& (MyIP[3] != 0 ))  {
    if (( String(SMTP.FROM).length() >5 )&&( String(SMTP.TO).length() >5 ) && ( String(SMTP.server).indexOf('.') > 0 ) && ( String(SMTP.TO).indexOf('.')>0  )){
//      Serial.println( "Going into the send - MessageID " + String(iMessageID) );
      snprintf(csTemp,MESSAGE_MAX,"\0"  )  ;  
      for ( i = 0 ; i < MAX_FERT ; i++){
        switch(iMessageID){
            case -4:  // Settings saved to eeprom
                snprintf(csTemp,MESSAGE_MAX,"%s - Confirmation Program and settings saved to EEPROM \0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break;           
            case -3:  // About to /reboot
                snprintf(csTemp,MESSAGE_MAX,"%s - One minute to Self reboot \0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break;   
            case -2:  // bootup/reboot
                snprintf(csTemp,MESSAGE_MAX,"%s - Booted up or Rebooted 15 minutes ago\0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break;   
            case -1:  // test
                snprintf(csTemp,MESSAGE_MAX,"%s - Test Email sent %02d:%02d:%02d \0",ghks.NodeName ,  timeinfo.tm_hour,  timeinfo.tm_min,  timeinfo.tm_sec )  ;  
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
                snprintf(csTemp,MESSAGE_MAX,"%s - Irrigation Still Presurised after shutoff \0",ghks.NodeName  )  ;  
            break ; 
            case 3:  // irriagtion line failed to pressurise after startup
                snprintf(csTemp,MESSAGE_MAX,"%s - Irrigation Line Failed to presurise after startup \0",ghks.NodeName  )  ;  
            break ;
            case 4:  // main soleniod CB trip
                snprintf(csTemp,MESSAGE_MAX,"%s - Main Solenoid Circuit Breaker Trip \0",ghks.NodeName  )  ;  
                i = MAX_FERT ;
            break ;
            case 5:  // ADC value greater than Alarm 1
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) greater then Alarm 1 \0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit )  ;  
                i = MAX_FERT ;
            break ;
            case 6:  // ADC value greater than Alarm 2
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) greater then Alarm 2 \0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit )  ;  
                i = MAX_FERT ;
            break ;
            case 7:  // ADC value less than Alarm 1
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) less then Alarm 1 \0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 8:  // ADC value less than Alarm 2
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) less then Alarm 2 \0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 9:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Greater than Alarm 1 when Main Valve on\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 10:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Greater than Alarm 2 when Main Valve on\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit   )  ;  
                i = MAX_FERT ;
            break ;
            case 11:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Less than Alarm 1 when Main Valve on\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit   )  ;  
                i = MAX_FERT ;
            break ;
            case 12:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Less than Alarm 2 when Main Valve on\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit   )  ;  
                i = MAX_FERT ;
            break ;
            case 13:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Greater than Alarm 1 when Main Valve off\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 14:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Greater than Alarm 2 when Main Valve off\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 15:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Less than Alarm 1 when Main Valve off\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 16:
                snprintf(csTemp,MESSAGE_MAX,"%s - %f (%s) Less than Alarm 2 when Main Valve off\0",ghks.NodeName,adcs.chan[0].ADC_Value,adcs.chan[0].ADC_Unit  )  ;  
                i = MAX_FERT ;
            break ;
            case 666:
                snprintf(csTemp,MESSAGE_MAX,"I2C Bus device(s) have changed state\r\n\0"  )  ;  
                i = MAX_FERT ;
            break;
            case 668:
                snprintf(csTemp,MESSAGE_MAX,"LoRa Bus device(s) have not responded in a timely manner\r\n\0"  )  ;  
                i = MAX_FERT ;
            break;
            
            default: snprintf(csTemp,MESSAGE_MAX,"%s \0",SMTP.subject ) ; 
                i = MAX_FERT ;
            break ;
        }
      }
//      Serial.println("Setting Subject >" + String(csTemp)+"<");

      // Set the session config      
      session.server.host_name = String(SMTP.server) ;     //  "smtp.office365.com"; // for outlook.com
      session.server.port = SMTP.port ;
      if ( SMTP.user[0] == 0 ){
        session.login.email = "" ;                       // set to empty for no SMTP Authentication
        session.login.password = "" ;                    // set to empty for no SMTP Authentication
        session.login.user_domain = "" ;
      }else{
        session.login.email = SMTP.user ;                  // Get settings from memory
        session.login.password = SMTP.password ;           // 
        session.login.user_domain = "";                    // client domain or ip e.g. mydomain.com << currently no setting for this
      }
    
      // Set the NTP config time
      session.time.ntp_server = String(ghks.timeServer[24]) ; // "pool.ntp.org,time.nist.gov";
      session.time.gmt_offset = (int)( ghks.gmtOffset_sec / 3600 ) ;
      session.time.day_light_offset = 0;
//      session.time.ntp_server = "" ;
    
      // Declare the SMTP_Message class variable to handle to message being transport
      SMTP_Message message;
      
      // Set the message headers
      if ( String(SMTP.FROM).length() < 6 ){
        message.sender.name = "Doby The House Elf";
        message.sender.email = "doby@house.elf";
      }else{
        message.sender.name = ghks.NodeName ;
        message.sender.email = String(SMTP.FROM) ;
      }

//      if ( iMessageID == -1 ){
//        snprintf(buff, BUFF_MAX, "%02d:%02d:%02d\0", hour(), minute(), second());
//        message.subject = "Test Email Sent "+ String(buff);
//      }else{
        message.subject = String(csTemp);      
        strcat(csTemp,"\r\n");
//      }
      message.addRecipient("", String(SMTP.TO));
    
      if ( String(SMTP.CC).length() > 5 ){
        message.addCc(String(SMTP.CC));
      }
      if ( String(SMTP.BCC).length() > 5 ){
        message.addBcc(String(SMTP.BCC ));
      }     

      if (iMessageID == 666 ) {
        strcat(csTemp,strBusResults.c_str() ) ;  // if a buss alarm add this in
        strBusResults = "" ; // clear after sending
      }
      if (iMessageID == 668 ){
        strcat(csTemp,strLoRaResults.c_str() ) ;  // if a buss alarm add this in
        strLoRaResults = "" ; // clear after sending
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
      
      snprintf(buff, BUFF_MAX, "\r\n %d/%02d/%02d %02d:%02d:%02d \r\n\0", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday , timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      strcat(csTemp,buff) ;
      snprintf(buff, BUFF_MAX,"Node ID %08X\r\n\0",chipid );
      strcat(csTemp,buff) ;
      snprintf(buff, BUFF_MAX,"IP %03u.%03u.%03u.%03u \r\n\0",MyIP[0],MyIP[1],MyIP[2],MyIP[3]);
      strcat(csTemp,buff) ;
      snprintf(buff, BUFF_MAX, "CPU Uptime %d:%02d:%02d (Days:Hrs:Min)  \r\n\r\n\0",(lMinUpTime/1440),((lMinUpTime/60)%24),(lMinUpTime%60));
      strcat(csTemp,buff) ;
      
//      Serial.println("Setting Body >" + String(csTemp) + "<");
      message.text.content = csTemp;

      smtp.connect(&session /* session credentials */);    
      if (!MailClient.sendMail(&smtp, &message)){  // Start sending Email and close the session
        Serial.println("Error sending Email, " + smtp.errorReason());
      }else    {
        Serial.println("Email Sent OK");        
      }
  
  //    Serial.println("Server: " + String(SMTP.server));
  //    Serial.println("Port: " + String(SMTP.port));
  //    Serial.println("User: " + String(SMTP.user));
  //    Serial.println("Password: " + String(SMTP.password));
  
      
    }else{
      Serial.println("Mail not set up proper like dude...");
    }
  }
}

//*********************************************  WEB GUI EMAIL *************************************************************************
void DisplayEmailSetup() {
  long  i = 0 ;
  int ii  ;
  int j ,iTmp ;
  int k , kk  ;
  String message ;
  String MyCheck ;
  String MyColor ;
  String pinname ;
//  SerialOutParams();
  
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
        case 122:
          ResetADCCalInfo();
        break;   
      }
    }
/*    i = String(server.argName(j)).indexOf("adp1");
    if (i != -1) { // ADC input pin 1
      ghks.ADC_Input_PIN1 = String(server.arg(j)).toInt() ;
    }
    i = String(server.argName(j)).indexOf("adp2");
    if (i != -1) { // ADC input pin 2
      ghks.ADC_Input_PIN2 = String(server.arg(j)).toInt() ;
    }
*/    
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
    i = String(server.argName(j)).indexOf("bsci");
    if (i != -1){ 
      SMTP.iBusScanInterval = String(server.arg(j)).toInt() ;
      if (( SMTP.iBusScanInterval < MINBUSSCANINTERVAL ) && ( SMTP.iBusScanInterval > 0 )){
        SMTP.iBusScanInterval = MINBUSSCANINTERVAL ;
      }
    }
    i = String(server.argName(j)).indexOf("lrsi");  // lora scan interval
    if (i != -1){ 
      SMTP.iLoRaScanInterval = String(server.arg(j)).toInt() ;
      if (( SMTP.iLoRaScanInterval < MINLORASCANINTERVAL ) && ( SMTP.iLoRaScanInterval > 0 )){
        SMTP.iLoRaScanInterval = MINLORASCANINTERVAL ;
      }
    }
    i = String(server.argName(j)).indexOf("lrto");   // lora time out
    if (i != -1){ 
      SMTP.iLoRaTimeOut = String(server.arg(j)).toInt() ;
      if (( SMTP.iLoRaTimeOut < 120 ) && ( SMTP.iLoRaTimeOut > 0 )){
        SMTP.iLoRaTimeOut = 120 ;
      }
    }
    i = String(server.argName(j)).indexOf("lotk");
    if (i != -1){ 
      SMTP.LowTankQty= String(server.arg(j)).toFloat() ;
    }       

    i = String(server.argName(j)).indexOf("smuz");
    if (i != -1){  //  
        SMTP.bUseEmail = false ;        
    }
    i = String(server.argName(j)).indexOf("smuy");
    if (i != -1){  //  
        SMTP.bSpare = false ;
    }
    i = String(server.argName(j)).indexOf("smue");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        SMTP.bUseEmail = true ;        
      }
    }     
    i = String(server.argName(j)).indexOf("smup");
    if (i != -1){  //  
      if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
        SMTP.bSpare = true ;        
      }
    }     
/*
    i = String(server.argName(j)).indexOf("allu");
    if (i != -1){  
     String(server.arg(j)).toCharArray(ghks.ADC_Unit , sizeof(ghks.ADC_Unit)) ;
    }    
    i = String(server.argName(j)).indexOf("all1");
    if (i != -1){ 
      ghks.ADC_Alarm1 = String(server.arg(j)).toFloat() ;
    }          
    i = String(server.argName(j)).indexOf("all2");
    if (i != -1){ 
      ghks.ADC_Alarm2 = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("adcm");
    if (i != -1){ 
      ghks.ADC_Cal_Mul = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("adco");
    if (i != -1){ 
      ghks.ADC_Cal_Ofs = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("aldl");
    if (i != -1){ 
      ghks.ADC_Alarm_Delay = String(server.arg(j)).toInt() ;
    }      
    
    i = String(server.argName(j)).indexOf("almmc");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0x7f ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x80 ;              
      }
    }  

    i = String(server.argName(j)).indexOf("alme2");
    if (i != -1){ 
      switch ( String(server.arg(j)).toInt()  ){
        case 0:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
        break;
        case 1:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
          ghks.ADC_Alarm_Mode |=  0x10 ;      
        break;
        case 2:
          ghks.ADC_Alarm_Mode &=  0xcf ;      
          ghks.ADC_Alarm_Mode |=  0x20 ;      
        break;
        case 3:
          ghks.ADC_Alarm_Mode |=  0x30 ;      
        break;
      }  
    }

    i = String(server.argName(j)).indexOf("alme1");
    if (i != -1){ 
      switch ( String(server.arg(j)).toInt()  ){
        case 0:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
        break;
        case 1:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
          ghks.ADC_Alarm_Mode |=  0x02 ;      
        break;
        case 2:
          ghks.ADC_Alarm_Mode &=  0xf9 ;      
          ghks.ADC_Alarm_Mode |=  0x04 ;      
        break;
        case 3:
          ghks.ADC_Alarm_Mode |=  0x06 ;      
        break;
      }  
    }

    i = String(server.argName(j)).indexOf("alma2");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0xf7 ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x08 ;              
      }
    }  
    i = String(server.argName(j)).indexOf("alma1");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        ghks.ADC_Alarm_Mode &=  0xfe ;      
      }else{
        ghks.ADC_Alarm_Mode |=  0x01 ;              
      }
    }  
    */
    i = String(server.argName(j)).indexOf("frtadc");
    if (i != -1){ 
      ghks.lFertActiveOptions = constrain( String(server.arg(j)).toInt(),0,3) ;                      
    }    
  }

  SendHTTPHeader();   //  ################### START OF THE RESPONSE  ######
  String strLineStart = "<tr><form method=post action=" + server.uri() + "><td ";
  String strLineStop = "</td><td><input type='submit' value='SET'></td></form></tr>\r\n" ;
  message = F("<a href='/?command=2'>Save Parameters to EEPROM</a><br>\r\n");
  if ( bSaveReq != 0 ){
    message += F("</blink><font color='red'><b>Changes Have been made to settings.<br>Make sure you save if you want to keep them</b><br></font><br>\r\n") ;     
  }
  message += F("<table border=1 title='Email setup for SCADA'>\r\n") ;
  message += F("<tr><th>Email Parameter</th><th>Value</th><th>Units</th><th>.</th></tr>\r\n") ;              
  message += strLineStart + ">SMTP Port</td><td align=center title='Popular Values 25 , 465 , 2525 , 587'><input type='text' name='smpo' value='"+String(SMTP.port)+"' size=30></td><td>"+strLineStop ;
  message += strLineStart + ">SMTP Server</td><td align=center><input type='text' name='smse' value='"+String(SMTP.server)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP User</td><td align=center><input type='text' name='smus' value='"+String(SMTP.user)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP Password</td><td align=center><input type='text' name='smpa' value='"+String(SMTP.password)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP FROM</td><td align=center><input type='text' name='smfr' value='"+String(SMTP.FROM)+"' size=30></td><td>."+strLineStop ;
  server.sendContent(message) ;
//  Serial.println("1 Sent " + String(message.length(),DEC) + " bytes" );
  message = "" ;

  message += strLineStart + ">SMTP TO</td><td align=center><input type='text' name='smto' value='"+String(SMTP.TO)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP CC</td><td align=center><input type='text' name='smcc' value='"+String(SMTP.CC)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP BCC</td><td align=center><input type='text' name='smbc' value='"+String(SMTP.BCC)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP Subject</td><td align=center><input type='text' name='smsj' value='"+String(SMTP.subject)+"' size=30></td><td>."+strLineStop ;
  message += strLineStart + ">SMTP Message</td><td align=center><input type='text' name='smmb' value='"+String(SMTP.message)+"' size=30></td><td>."+strLineStop ;
  server.sendContent(message) ;
//  Serial.println("2 Sent " + String(message.length(),DEC) + " bytes" );
  message = "" ;
  
  if ( ( SMTP.bSecure ) != 0 ){
    MyCheck = F("CHECKED")  ;    
  }else{
    MyCheck = F("")  ;    
  }
  message += strLineStart + ">SMTP Secure<input type='hidden' name='smbz' value='0'></td><td align=center><input type='checkbox' name='smbs' " + String(MyCheck)+ "></td><td>."+strLineStop ;
  message += "<tr><td>Last Email Status</td><td>"+String(lRet_Email)+"</td><td colspan=2>.</td></tr>\r\n" ;
  message += "<tr></td><td colspan=4>.</td></tr>\r\n" ;
  server.sendContent(message) ;
  message = "" ;    

  message += strLineStart + ">ADC Alarms to disable fert</td>" ;
  message += "<td><select name='frtadc'>" ;
  for ( j = 0 ; j < 4 ; j++ ){ 
    if (( ghks.lFertActiveOptions & 0x03 ) == j ){
      MyCheck = "SELECTED" ;
    }else{
      MyCheck = "" ;
    }
    message += "<option value='"+String(j)+"' " + MyCheck +">" ;
    switch ( j ){
      case 0: message += F("None") ;  break ;
      case 1: message += F("Greater Than Only") ; break ;
      case 2: message += F("Less Than Only") ; break ;
      case 3: message += F("Any ADC Alarm") ; break ;
    }
  }
  message += "</select></td><td>."+strLineStop ;
  message += strLineStart + "title='-1 to disable alarm emails'>Tank Low Alarm Volume</td><td align=center><input type='text' name='lotk' value='"+String(SMTP.LowTankQty)+"' size=30></td><td>(L)"+strLineStop ;
  server.sendContent(message) ;
  message = "" ;    

  message += strLineStart + "title='Bus Monitor -1 disables'>Bus Monitor Interval</td><td align=center><input type='text' name='bsci' value='"+String(SMTP.iBusScanInterval)+"' size=30></td></td><td>(min)"+strLineStop ;
  if ( ( SMTP.bUseEmail ) != 0 ){
    MyCheck = F("CHECKED")  ;    
  }else{
    MyCheck = ""  ;    
  }
  message += strLineStart + "><input type='hidden' name='smuz' value='0'>Use Email Alarm Messaging</td><td align=center><input type='checkbox' name='smue' " + String(MyCheck)+ "></td><td>."+strLineStop ;
  message += strLineStart + "colspan=4 align='center'><input type='hidden' name='command' value='11'><input type='submit' value='### SEND TEST EMAIL ###'></td></form></tr>\r\n" ;
  message += strLineStart + "colspan=4 align='center'><input type='hidden' name='command' value='121'><input type='submit' value='Clear / RESET EMAIL Settings'></form></td></tr>\r\n" ; 
  message += strLineStart + "colspan=4 align='center'><input type='hidden' name='command' value='122'><input type='submit' value='Clear / ADC Settings'></form></td></tr>\r\n" ; 
  message += "</table>\r\n" ;
  
  server.sendContent(message) ;
//  Serial.println("6 Sent " + String(message.length(),DEC) + " bytes" );
  message = "" ;    
  
  SendHTTPPageFooter();
}



void ResetSMTPInfo(){
  SMTP.port = 25 ;                // 25 , 465 , 2525 , 587
//  sprintf(SMTP.server,"203.36.137.241\0") ;   // smtp.telstrabusiness.com 
  sprintf(SMTP.server,"121.200.0.25\0") ;   // mail.aussiebroadband.com.au
  sprintf(SMTP.user,"\0") ;
  sprintf(SMTP.password,"\0") ;
  sprintf(SMTP.FROM,"dougal@plummer.com.au\0") ;
//  sprintf(SMTP.FROM,"\0") ;
  sprintf(SMTP.TO,"dougal@plummersoftware.onmicrosoft.com\0") ;
//  sprintf(SMTP.TO,"\0") ;
  sprintf(SMTP.CC,"\0") ;
  sprintf(SMTP.BCC,"\0") ;
  sprintf(SMTP.message,"Test Email\0") ;
//  String(((uint32_t)chipid),HEX).toCharArray(SMTP.subject,sizeof(SMTP.subject)) ;
//  sprintf(SMTP.subject,"Node %s\0", String( EPS.getEfuseMac(),HEX).c_str()   ) ;
  sprintf(SMTP.subject,"Node %08X\0",chipid);
  SMTP.bSecure = false ;
  SMTP.bUseEmail = false ;
  SMTP.LowTankQty = 50.0 ;   
  Serial.println(F("*** ResetSMTPInfo Called ***"));  
  SMTP.iLoRaScanInterval = -1 ;
  SMTP.iBusScanInterval = -1 ;
  SMTP.iLoRaTimeOut = 180 ;
}



