
void adcLocalMap() {
  int i , ii , x , iTmp ;
  uint8_t j , k , kk ;
  uint8_t BoardBit ;
  int chan = 0 ;
  String message ;  
  String bgcolor ;  
  String pinname ;
  String MyColor ;
  String MyCheck ;

//  SerialOutParams();
  
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

    i = String(server.argName(j)).indexOf("chan");
    if (i != -1){  
      chan = String(server.arg(j)).toInt() ;
      chan = constrain(chan,0,ADC_MAX_CHAN - 1) ;
    }    
    i = String(server.argName(j)).indexOf("adpin");
    if (i != -1){  
     adcs.chan[chan].ADC_Input_PIN = String(server.arg(j)).toInt() ;
    }    
    i = String(server.argName(j)).indexOf("admul");
    if (i != -1){ 
      adcs.chan[chan].ADC_Cal_Mul = String(server.arg(j)).toFloat() ;
    }      
    i = String(server.argName(j)).indexOf("adcof");
    if (i != -1){ 
      adcs.chan[chan].ADC_Cal_Ofs = String(server.arg(j)).toFloat() ;
    }          
    i = String(server.argName(j)).indexOf("aduni");
    if (i != -1){  
     String(server.arg(j)).toCharArray(adcs.chan[chan].ADC_Unit , sizeof(adcs.chan[chan].ADC_Unit)) ;
    }     

    i = String(server.argName(j)).indexOf("almno");  //alarm channel
    if (i != -1){ 
      chan = String(server.arg(j)).toInt()  ;  
      chan = constrain(chan,0,ADC_MAX_ALARM - 1) ;
    }
    i = String(server.argName(j)).indexOf("almch");
    if (i != -1){ 
      adcs.alarm[chan].ADC_Channel = String(server.arg(j)).toInt() ;   
    }
    i = String(server.argName(j)).indexOf("alme1");
    if (i != -1){ 
      switch ( String(server.arg(j)).toInt()  ){
        case 0:
          adcs.alarm[chan].ADC_Mode &=  0xf9 ;      
        break;
        case 1:
          adcs.alarm[chan].ADC_Mode &=  0xf9 ;      
          adcs.alarm[chan].ADC_Mode |=  0x02 ;      
        break;
        case 2:
          adcs.alarm[chan].ADC_Mode &=  0xf9 ;      
          adcs.alarm[chan].ADC_Mode |=  0x04 ;      
        break;
        case 3:
          adcs.alarm[chan].ADC_Mode |=  0x06 ;      
        break;
      }  
    }
  
    i = String(server.argName(j)).indexOf("alma1");
    if (i != -1){ 
      if ( String(server.arg(j)).toInt() == 0x00 ){
        adcs.alarm[chan].ADC_Mode &=  0xfe ;      
      }else{
        adcs.alarm[chan].ADC_Mode |=  0x01 ;              
      }
    } 
    i = String(server.argName(j)).indexOf("almdl");
    if (i != -1){ 
      adcs.alarm[chan].ADC_Delay = String(server.arg(j)).toInt() ;
    }      
    i = String(server.argName(j)).indexOf("almll");
    if (i != -1){ 
      adcs.alarm[chan].ADC_Value = String(server.arg(j)).toInt() ;
    }      
     
    
  }
  
  //  ######################  START RESPONCE  ########################
  SendHTTPHeader();
  message = F("<br><center><b>ADC Setup</b><br><table border=1 title='ADC Setup'>\r\n");
  message += F("<tr><th>Chanel</th><th>PIN</th><th>Mult</th><th>Ofs</th><th>Units</th><th>Raw</th><th>Values</th><th colspan=2></th></tr>\r\n");  
  for ( j=0; j<ADC_MAX_CHAN; j++){
    message += "<form method=get action=" + server.uri() + "><input type='hidden' name='chan' value='"+String(j)+"'><tr><td align=center>"+String(j)+"</td><td align=center><select name='adpin'>" ; 
    message +=  "<option value="+String(MaxPinPort)+">None" ;          
    for (ii = MinPinPort; ii < MaxPinPort; ii++) {
      if ( LocalPINOK(ii) ){
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
    }
    message += "</select></td><td><input title='The number of S.I. units per input Volt x 3.2' type='text' name='admul' value='"+String(adcs.chan[j].ADC_Cal_Mul)+"' size=5></td><td><input title='The easy way to do this is use the zero cal button below' type='text' name='adcof' value='"+String(adcs.chan[j].ADC_Cal_Ofs)+"' size=5></td><td><input type='text' name='aduni' value='"+String(adcs.chan[j].ADC_Unit)+"' size=5 maxlength=5></td><td>"+String(adcs.chan[j].ADC_RAW)+"</td><td><b> "+String(adcs.chan[j].ADC_Value,1) +"</b></td>" ;
    message += "<td><input type='submit' value='SET'></td></form><form method=get action=" + server.uri() + "><td><input type='hidden' name='chan' value='"+String(j)+"'><input type='hidden' name='adcof' value='"+String(-1.0*adcs.chan[j].ADC_RAW)+"'><input title='Setup the transducer for Zero physical input quantity then press this button to calibrate the zero point on computer' type='submit' value='DO ZERO CAL'></td></form></tr>\r\n" ;
    server.sendContent(message) ;
    message = "" ;
  }
  message += F("<tr><td colspan=9 align='center'><a href='/adc?command=122'>Reset ADC setting to default</a></td></tr>\r\n</table>\r\n<br><br>");
  server.sendContent(message);

  message = F("<br><center><b>ADC Alarms</b><br><table border=1 title='ADC Alarms'>\r\n");
  message += F("<tr><th>Alarm No</th><th>Chanel</th><th>Alarm</th><th>State</th><th>Level</th><th>Delay(s)</th><th></th></tr>\r\n");  
  for ( j=0; j<ADC_MAX_ALARM; j++){
    message += "<form method=get action=" + server.uri() + "><input type='hidden' name='almno' value='"+String(j)+"'><tr><td align=center>"+String(j)+"</td>" ;
    message += "<td><select name='almch'"+String(j)+"'>" ;
    for ( kk=0; kk<ADC_MAX_CHAN; kk++){
      if ( adcs.alarm[j].ADC_Channel == kk ){
        MyCheck = "SELECTED" ;
      }else{
        MyCheck = "" ;
      }
      message += "<option value='"+String(kk)+"' " + MyCheck +">"+String(kk) ;
    }
    message += "</select></td><td><select name='alme1'>" ;
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
    message += "</select></td><td><input type='text' name='almll' value='"+String(adcs.alarm[j].ADC_Value)+"' size=6></td><td><input type='text' name='almdl' value='"+String(adcs.alarm[j].ADC_Delay)+"' size=6></td>" ;
    message += "<td><input type='submit' value='SET'></td></form></tr>\r\n" ;
    server.sendContent(message) ;
    message = "" ;
  }
  message += F("</table>\r\n");
  server.sendContent(message);
  
  SendHTTPPageFooter();   
  
}

void ResetADCCalInfo(){
  int i ,j ; 
/*  sprintf(ghks.ADC_Unit,"kPa\0") ;
  ghks.ADC_Cal_Ofs = -170.0 ;
  ghks.ADC_Cal_Mul = 640.0 ;
  ghks.ADC_Alarm_Mode = 0 ;               // high low etc
  ghks.ADC_Alarm1 = 0 ;
  ghks.ADC_Alarm2 = 0 ;                   // 
  ghks.ADC_Input_PIN1 = 25 ;
  ghks.ADC_Input_PIN2 = 26 ;  
  ghks.ADC_Alarm_Delay = 60 ; */

  for ( j=0; j<ADC_MAX_ALARM; j++){
    adcs.alarm[j].ADC_Channel = j / (ADC_MAX_ALARM / ADC_MAX_CHAN) ;
    adcs.alarm[j].ADC_Mode = 0 ; 
    adcs.alarm[j].ADC_Value = 0 ;
    adcs.alarm[j].ADC_Delay = 10 ;
  }

  for ( i = 0 ; i < ADC_MAX_CHAN ; i++ ) {
    adcs.chan[i].ADC_RAW = 0 ;
    adcs.chan[i].ADC_Value = 0 ;
    adcs.chan[i].ADC_Cal_Mul = 640.0 ;
    adcs.chan[i].ADC_Cal_Ofs = -170.0 ;
    sprintf(adcs.chan[i].ADC_Unit , "kPa\0") ;      // units for display
    switch(i){
      case 0:
        adcs.chan[i].ADC_Input_PIN = 12 ;
      break;
      case 1:
        adcs.chan[i].ADC_Input_PIN = 13 ;
      break;
      case 2:
        adcs.chan[i].ADC_Input_PIN = 35 ;
      break;
      case 3:
        adcs.chan[i].ADC_Input_PIN = 34 ;
      break;
      case 4:
        adcs.chan[i].ADC_Input_PIN = 36 ;
      break;
      default:  // 39 etc
        adcs.chan[i].ADC_Input_PIN = 39 ;
      break;
    }
  }  
  Serial.println(F("*** ResetADCInfo Called ***"));  
}



void DoADC_sec(){
bool bTrigger = false ;
bool bTriggerLess = false ;
bool bTriggerMore = false ;
int i , k  ;  
      for ( i = 0 ; i < ADC_MAX_CHAN ; i++ ) {
      if (( adcs.chan[i].ADC_Input_PIN > MinPinPort) && (adcs.chan[i].ADC_Input_PIN < MaxPinPort )) {
        adcs.chan[i].ADC_RAW = analogRead(adcs.chan[i].ADC_Input_PIN) ;
        adcs.chan[i].ADC_Value = ((adcs.chan[i].ADC_Cal_Mul * (( 1.0 * adcs.chan[i].ADC_RAW ) + adcs.chan[i].ADC_Cal_Ofs ) / 1023 ) )  ;
      }else{
        adcs.chan[i].ADC_RAW = 0 ;
        adcs.chan[i].ADC_Value = 0 ; 
      }
    }
    bFertDisable = false ;
    for ( i = 0 ; i < ADC_MAX_ALARM ; i++ ) {
      k = adcs.alarm[i].ADC_Channel ;
      if (( k >= 0) && ( k < ADC_MAX_CHAN ))
        ADC_Value = adcs.chan[k].ADC_Value  ;
      else
        ADC_Value = 0 ;
      if (( adcs.alarm[i].ADC_Mode & 0x80 ) != 0 ) {   
        bTrigger = false ;
        bTriggerLess = false ;
        bTriggerMore = false ;   
        if ((( adcs.alarm[i].ADC_Mode & 0x06 ) == 0x06  ) || ( (( adcs.alarm[i].ADC_Mode & 0x02 ) == 0x02 ) && bValveActive ) || ( (( adcs.alarm[i].ADC_Mode & 0x04 ) == 0x04 ) && !bValveActive ))  {    // alarm 1 on 
          if (( adcs.alarm[i].ADC_Mode & 0x01 ) != 0 ){ // looking for a high alarm else jump down for a low on
            if ( ADC_Value > adcs.alarm[i].ADC_Value ) {     // high alarm test 
              bTrigger = true ;  
              bTriggerMore = true ;
              if (( adcs.alarm[i].ADC_Mode & 0x06 ) == 0x06  ){  // this is the always case
                  iMailMsg = 5 ;              
              }else{
                if ( bValveActive ){
                  iMailMsg = 9 ;                 
                }else{
                  iMailMsg = 13 ;
                }
              }
            }          
          }else{
            if ( ADC_Value < adcs.alarm[i].ADC_Value ) { // low alarm test
              bTrigger = true ;
              bTriggerLess = true ;
              if (( adcs.alarm[i].ADC_Mode & 0x06 ) == 0x06  ){   // this is the always case
                  iMailMsg = 7 ;                                
              }else{
                if ( bValveActive ){
                  iMailMsg = 11 ;                  
                }else{
                  iMailMsg = 15 ;                  
                }
              }
            }          
          }
        }
       
        if ( bTrigger ) {
          switch ( ghks.lFertActiveOptions ){           
            case 1: if ( bTriggerMore ) bFertDisable = true ;  break ;      // greater than
            case 2: if ( bTriggerLess ) bFertDisable = true ;  break ;      // less than
            case 3: bFertDisable = true ;  break ;                          // Any  
            default: break ;                                                // none  ie zero 0      bFertDisable = false ; 
          }
          if (!bSentADCAlarmEmail) {
            ADC_Trigger[i]++ ;           
          }               
        }else{
          ADC_Trigger[i] = 0 ;      
  //          iMailMsg = 0 ;
  //        bFertDisable = false ;
        }
        if ( ADC_Trigger[i] > adcs.alarm[i].ADC_Delay ) {
          if ( !bSentADCAlarmEmail ){
            if ( iMailMsg != 0 ){
              SendEmailToClient(iMailMsg) ;
            }
            bSentADCAlarmEmail = true ;
          }  
        }
       
      }else{
        ADC_Trigger[i] = 0 ;
//        iMailMsg = 0 ;
//        bSentADCAlarmEmail = false ;
      }
    }

}

