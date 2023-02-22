
void adcLocalMap() {
  int i , ii , x , iTmp ;
  uint8_t j , k , kk ;
  uint8_t BoardBit ;
  String message ;  
  String bgcolor ;  
  String pinname ;
  String MyColor ;
  String MyCheck ;
  
  for ( j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 1:  // load values
          LoadParamsFromEEPROM(true);
          Serial.println("Load from EEPROM");
        break;
        case 2: // Save values
          LoadParamsFromEEPROM(false);
          Serial.println("Save to EEPROM");
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
          Serial.println(String(ii) + " Fert records updated");
        break;
      }  
    }

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

  }
  
  //  ######################  START RESPONCE  ########################
  SendHTTPHeader();
  message = F("<br><center><b>ADC Setup</b><br><table border=1 title='ADC Setup'>\r\n");
  message += F("<tr><th>Chanel</th><th>PIN</th><th>Mult</th><th>Ofs</th><th>Units</th><th>Raw</th><th>Values</th><th colspan=2></th></tr>\r\n");  
  for ( j=0; j<ADC_MAX_CHAN; j++){
    message += "<form method=get action=" + server.uri() + "><input type='hidden' name='chan' value='"+String(j)+"'><tr><td align=center>"+String(j)+"</td><td align=center><select name='adp"+String(x)+"'>" ; 
    for (ii = MinPinPort; ii < MaxPinPort; ii++) {
      if (adcs.chan[j].ADC_Input_PIN == ii ){
        MyColor = F(" SELECTED ");
      }else{
        MyColor = "";            
      }
  //    iTmp = 0 ;
      pinname = strPINName(ii,&iTmp,0);
      if ( iTmp == 0 ) {
        message +=  "<option value="+String(ii)+ MyColor +">" + pinname  ;          
      }
    }
    message += "</select></td><td><input title='The number of S.I. units per input Volt x 3.2' type='text' name='adm"+String(j)+"' value='"+String(adcs.chan[j].ADC_Cal_Mul)+"' size=5></td><td><input title='The easy way to do this is use the zero cal button below' type='text' name='adco' value='"+String(adcs.chan[j].ADC_Cal_Ofs)+"' size=5></td><td><input type='text' name='allu' value='"+String(adcs.chan[j].ADC_Unit)+"' size=5 maxlength=5></td><td>"+String(adcs.chan[j].ADC_RAW)+"</td><td><b> "+String(adcs.chan[j].ADC_Value,1) +"</b></td>" ;
    message += "<td><input type='submit' value='SET'></td></form><form method=get action=" + server.uri() + "><td><input type='hidden' name='adco' value='"+String(-1.0*adcs.chan[j].ADC_RAW)+"'><input title='Setup the transducer for Zero physical input quantity then press this button to calibrate the zero point on computer' type='submit' value='DO ZERO CAL'></td><input type='hidden' name='chan' value='"+String(j)+"'></form></tr>\r\n" ;
    server.sendContent(message) ;
    message = "" ;
  }
  message += F("</table>\r\n<br><br>");
  server.sendContent(message);

  message = F("<br><center><b>ADC Alarms</b><br><table border=1 title='ADC Alarms'>\r\n");
  message += F("<tr><th>Alarm No</th><th>Chanel</th><th>Alarm</th><th>State</th><th>Level</th><th>Delay(s)</th><th></th></tr>\r\n");  
  for ( j=0; j<ADC_MAX_ALARM; j++){
    message += "<form method=get action=" + server.uri() + "><input type='hidden' name='almno' value='"+String(j)+"'><tr><td align=center>"+String(j)+"</td>" ;
    message += "<input type='text' name='almch' value='"+String(adcs.alarm[j].ADC_Channel)+"'><tr>" ;
    message += "<td><select name='alme1'>" ;
    for ( k = 0 ; k < 4 ; k++ ){ 
      if ((( adcs.alarm[j].ADC_Mode & 0x06 )>>1 ) == k ){
        MyCheck = "SELECTED" ;
      }else{
        MyCheck = "" ;
      }
      message += "<option value='"+String(k)+"' " + MyCheck +">" ;
      switch ( k ){
        case 0: message += F("Disabled") ;  break ;
        case 1: message += F("On Master Enabled") ; break ;
        case 2: message += F("Off Master Enabled") ; break ;
        case 3: message += F("Always Enabled") ; break ;
      }
    }
    message += "</select></td>" ;
    message += "<td><select name='alma1'>" ;
    switch ( adcs.alarm[j].ADC_Mode & 0x01 ){
      case 0:   
        message += F("<option value='0' SELECTED>Less Than") ;
        message += F("<option value='1' >Greater Than") ;
      break;
      case 1:   
        message += F("<option value='0'>Less Than") ;
        message += F("<option value='1' SELECTED>Greater Than") ;
      break;
    }
    message += "</select></td><td><input type='text' name='all"+String(j)+"' value='"+String(adcs.alarm[j].ADC_Value)+"' size=6></td><td><input type='text' name='all1' value='"+String(adcs.alarm[j].ADC_Delay)+"' size=6></td>" ;
    message += "<td><input type='submit' value='SET'></td></form></tr>\r\n" ;
    server.sendContent(message) ;
    message = "" ;
  }
  message += F("</table>\r\n");
  server.sendContent(message);
  
  SendHTTPPageFooter();   
  
}

