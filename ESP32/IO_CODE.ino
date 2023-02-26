bool CheckIfMCPInput(int Board, int Bit ){
bool bRet = false ;
int k ;
  for ( k = 0 ; k < MAX_VALVE ; k ++ ) {
    if (( evalve[k].TypeMaster & 0x40 ) != 0 ){
      if (((( evalve[k].FeedbackBoardBit & 0x0f ) >> 4 )  == Board ) && (( mcp_relay_tranlate[evalve[k].FeedbackBoardBit] & 0x0f ) == Bit )){
        bRet = true ;    
      }
    }
  }  
  return(bRet);  
}

int SolPulseWidth(int PulseIndex){
  switch (PulseIndex){
    case 1:
      return(20);
    break;
    case 2:
      return(40);
    break;
    case 3:
      return(80);
    break;
    case 4:
      return(160);
    break;
    case 5:
      return(320);
    break;
    case 6:
      return(640);
    break;
    case 7:
      return(1280);
    break;
    default:
      return(10);
    break;
  }
}

void ActivateOutput(int Board, int Bit, bool State, int PulseTime){
int ii , k = -1 ;  
int j ;
  switch ( eboard[Board].Type ){
    case 0:  // PCF8574
      for ( ii = 0 ; ii < MAX_BOARDS ; ii ++ ) {
        if (eboard[Board].Address == IOEXP[ii].getaddress()){
          k = ii ;
        }
      }
      if ( k != -1 ) { // make sure the address is valid !!!
        if ( PulseTime > 0 ){
          IOEXP[k].pulsepin(Bit,PulseTime,State) ;              
        }else{
          IOEXP[k].write(Bit,State) ;      
        }
      }
    break;
    case 1:  // MCP23017
      Serial.println("Activate Output Bit " + String(Bit) + " Board " + String(Board) + " Pulse Time " + String(PulseTime) + " State " + String(State));
      for ( ii = 0 ; ii < MAX_MCP23017 ; ii ++ ) { // scan all the possible port expanders
//        Serial.println("eboard.address " + String(eboard[Board].Address) + " Physical " + String(mcp[ii].getaddress())  ) ;
        if (eboard[Board].Address == ( mcp[ii].getaddress() + 0x20) ){
          k = ii ;
        }
      }
//      Serial.println("Board Array Address " + String(k));
      if ( k != -1 ){  // make sure the address is valid
        switch (eboard[Board].Translate ){
          case 0:  // no translation
            j = Bit ;
          break;
          case 1:  // table one for MCP
            j = mcp_relay_tranlate[Bit] ;
          break;
        }
        Serial.println("MCP23017 Output " + String(j) + " Board " + String(k) + " time " + String(PulseTime) + " state " + String(State));
        if ( PulseTime > 0 ){
          mcp[k].pulsepin(j,PulseTime,State) ;        
        }else{
          mcp[k].digitalWrite( j , State) ;              
        }        
      }
    break;
    case 2:  // LOCAL I/O
      Serial.println("Activate Output Bit " + String(Bit) + " Board " + String(Board) + " Pulse Time " + String(PulseTime) + " State " + String(State));
      if (( Bit < MAX_LOCAL_IO ) && ( LocalPINOK(elocal.IOPin[Bit]))){
        if ((( elocal.IOPin[Bit] >= 0 ) && ( elocal.IOPin[Bit] < MaxPinPort)) && ( elocal.IOPin[Bit] != 255 ))   {
          if ( PulseTime > 0 ){
            digitalWrite(elocal.IOPin[Bit],!State) ;
            delay(PulseTime);          
            digitalWrite(elocal.IOPin[Bit],State) ;          
          }else{
            digitalWrite(elocal.IOPin[Bit],State) ;          
          }
        }
      }
    break;
    case 3: // None
    break;
  }
}

int MaxBoardOutputs(int Board){
  switch ( eboard[Board].Type ){
    case 0:  // PCF8574
      return(8);
    break;
    case 1:  // MCP23017
      return(16);
    break;
    case 2:  // LOCAL I/O
#if defined(ESP32)    
      return(16);
#elif defined(ESP8266)
      return(4);
#endif           
    break;
    default: // 3 None
      return(0);
    break;
  }  
}

bool GetIntput(int Board, int Bit){
int ii , k = -1 ;  
int j ; 
bool bRet ;
  switch ( eboard[Board].Type ){
    case 0:  // PCF8574
      for ( ii = 0 ; ii < MAX_BOARDS ; ii ++ ) {
        if (eboard[Board].Address == IOEXP[ii].getaddress()){
          k = ii ;
        }
      }
      if ( k != -1 ) { // make sure the address is valid !!!
        bRet = !IOEXP[( k & 0x0f )].readButton( Bit & 0x07 ); 
      }
    break;
    case 1:  // MCP23017
      for ( ii = 0 ; ii < MAX_BOARDS ; ii ++ ) {
        if (eboard[Board].Address == mcp[ii].getaddress()){
          k = ii ;
        }
      }
      if ( k != -1 ){  // make sure the address is valid
        switch (eboard[Board].Translate ){
          case 0:  // no translation
            j = Bit ;
          break;
          case 1:  // table one for MCP
            j = mcp_relay_tranlate[Bit] ;
          break;
        }
        bRet = !mcp[k].digitalRead(j) ;              
      }
    break;
    case 2:  // LOCAL I/O
      if (( Bit < MAX_LOCAL_IO ) && ( LocalPINOK(elocal.IOPin[Bit]))) {
        if (( elocal.IOPin[Bit] >= 0 ) && ( elocal.IOPin[Bit] < MaxPinPort) && ( elocal.IOPin[Bit] != 255 )){
          bRet = !digitalRead(elocal.IOPin[Bit]) ;          
        }
      }
    break;
    case 3: // None
    break;
  }
  return(bRet);
}


void ioLocalMap() {                                //  ##############  LOCAL IO MAP  ##########################
  int i , ii , x , k ;
  uint8_t j , kk ;
  uint8_t BoardBit ; 
  String message ;  
  String bgcolor ;  
  String pinname ;

  SerialOutParams();  
  for (uint8_t j=0; j<server.args(); j++){
    for ( ii = 0 ; ii < MAX_LOCAL_IO ; ii++){                              // handle all the filter control arguments
      i = String(server.argName(j)).indexOf("i" + String(ii));
      if (i != -1){                                     
        k = String(server.arg(j)).toInt() ;                                // IO pin address
        if ((( k >= 0 ) && ( k <= MaxPinPort )) || ( k == 255 )){
          elocal.IOPin[ii] = k ;
        }else{
          elocal.IOPin[ii] = MaxPinPort ;
        }
      }      
    }
  }

  SendHTTPHeader();

  server.sendContent(F("<br><center><b>Local Processor Pin I/O Map</b><br><table border=1 title='Local I/O Map'><tr><th>Bit</th><th>PIN</th><th>.</th></tr>"));
  
  for (i = 0; i < MAX_LOCAL_IO ; i++) {     // for each Bit allocate a local I/O pin against it
    message = F("<tr>");
    message += "<tr><form method=get action=" + server.uri() + "><td>"+String(i)+"</td><td><select name='i"+String(i)+"'>";
    for (ii = 0 ; ii < MaxPinPort; ii++) {
      if (elocal.IOPin[i] == ii ){
        bgcolor = F(" SELECTED ");
      }else{
        bgcolor = "";            
      }
      if (LocalPINOK(ii)){
        pinname = strPINName(ii,&x,1) ;  // look for digital pins
        if ( x == 0 ){
          message += "<option value="+String(ii)+ bgcolor +">" + pinname + String(ii) ;          
        }
      }
    }
    if ( elocal.IOPin[i] == 255){
      bgcolor = F(" SELECTED ");
    }else{
      bgcolor = "";            
    }
    message += "<option value=255 "+ bgcolor +">Not Connected " ;
    message += F("</select></td><td><input type='submit' value='SET'></form></td></tr>\n\r");
    server.sendContent(message);
    message = "" ;
  }
  server.sendContent(F("</table>"));
  SendHTTPPageFooter();   
}



//                         I/O Database scan
void ioScan() {
  int i , ii ;
  uint8_t j , k , kk ;
  uint8_t BoardBit ;
  String message ;  
  String bgcolor ;  
/*
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
  */
  for (uint8_t j=0; j<server.args(); j++){
    for ( ii = 0 ; ii < MAX_BOARDS ; ii++){                              // handle all the filter control arguments
      i = String(server.argName(j)).indexOf("a" + String(ii));
      if (i != -1){                                     
        k = String(server.arg(j)).toInt() ;  // board address
//        Serial.println("i "+String(i)+" Board Address " + String(k));
        if (( k > 8 ) && ( k < 127 )){
          eboard[ii].Address = k ;
        }else{
          eboard[ii].Address = 0x20 ;
        }
      }      
      i = String(server.argName(j)).indexOf("t" + String(ii));
      if (i != -1){                                                     // 
        eboard[ii].Type = String(server.arg(j)).toInt();        
      }      
      i = String(server.argName(j)).indexOf("m" + String(ii));
      if (i != -1){                                                     // 
        eboard[ii].Translate = String(server.arg(j)).toInt();        
      }      
    }
  }
  SendHTTPHeader();

  server.sendContent(F("<br><center><b>I/O DB Check</b><br><table border=1 title='I/O DB Check'><tr><th rowspan=2>Board</th><th rowspan=2>I2C<br>Address</th><th rowspan=2>Board<br>Type</th><th rowspan=2>Bit<br>Mapping</th><th rowspan=2>.</th><th colspan=16>Output / Input</th></tr><tr>"));
  for (i = 0; i < 16; i++) {
    server.sendContent("<th>"+String(i,HEX)+"</th>");
  }
  server.sendContent(F("</tr>"));
  
  for (i = 0; i < MAX_BOARDS ; i++) {                             // for each board
    server.sendContent(F("<tr>"));
    server.sendContent("<tr><form method=get action=" + server.uri() + "><td>"+String(i)+"</td><td><select name='a"+String(i)+"'>");
    message = "" ;
    switch (eboard[i].Type){
      case 0:
        for (ii = 0; ii < 8; ii++) {
          if ((ii+0x20)== eboard[i].Address ){
            bgcolor = F(" SELECTED ");
          }else{
            bgcolor = "";            
          }
          message += ("<option value="+String(ii+0x20)+ bgcolor +">0x"+String(ii+0x20,HEX));          
        }
        for (ii = 0; ii < 8; ii++) {
          if ((ii+0x38)== eboard[i].Address ){
            bgcolor = F(" SELECTED ");
          }else{
            bgcolor = "";            
          }
          message += ("<option value="+String(ii+0x38)+ bgcolor +">0x"+String(ii+0x38,HEX));          
        }
      break;
      case 1:
        for (ii = 0; ii < 8; ii++) {
          if ((ii+0x20)== eboard[i].Address ){
            bgcolor = F(" SELECTED ");
          }else{
            bgcolor = "";            
          }
          message += ("<option value="+String(ii+0x20)+ bgcolor +">0x"+String(ii+0x20,HEX));          
        }
      break;
      case 2:    // local
      case 3:    // none
      default:
        for (ii = 0; ii < MAX_BOARDS ; ii++) {
          bgcolor = "";            
          if ( ii < 8 ) {
            kk = 0x20 ;
          }else{
            kk = 0x30 ;
          }
          if ((ii+kk)== eboard[i].Address ){
            bgcolor = F(" SELECTED ");
          }
          message += ("<option value="+String(ii+kk)+ bgcolor +">0x"+String(ii+kk,HEX));                                
        }
      break;
    }
    server.sendContent(message);
    server.sendContent("</select></td><td><select name='t"+String(i)+"'>");
    switch (eboard[i].Type){
      case 0:
        server.sendContent( F("<option value='0' SELECTED >PCF8574 <option value='1' >MCP23017 <option value='2' >Local I/O<option value='3' >-- None --")) ;    
      break;
      case 1:
        server.sendContent( F("<option value='0' >PCF8574 <option value='1' SELECTED >MCP23017 <option value='2' >Local I/O<option value='3' >-- None --")) ;    
      break;
      case 2:
        server.sendContent( F("<option value='0' >PCF8574 <option value='1' >MCP23017 <option SELECTED value='2' >Local I/O<option value='3' >-- None --")) ;    
      break;
      default:
        server.sendContent( F("<option value='0' >PCF8574 <option value='1' >MCP23017 <option value='2' >Local I/O<option SELECTED value='3' >-- None --")) ;    
      break;
    }
    server.sendContent(F("</select></td><td>"));
    server.sendContent("<select name='m"+String(i)+"'>");
    switch (eboard[i].Translate){
      case 0:
        server.sendContent( F("<option value='0' SELECTED>None <option value='1' >DIL AB")) ;    
      break;
      default:
        server.sendContent( F("<option value='0'>None <option value='1' SELECTED>DIL AB")) ;    
      break;
    }    
    server.sendContent(F("</select></td><td><input type='submit' value='SET'></form></td>"));
    for (j = 0 ; j < 16 ; j ++ ){     //   each bit on each board
      BoardBit = ( i << 4 ) | j ;
      message = "" ;
      bgcolor = "" ;
      for ( k = 0 ; k < MAX_VALVE ; k ++ ) {
        if (( evalve[k].OnCoilBoardBit == BoardBit ) && ( evalve[k].OnCoilBoardBit == evalve[k].OffCoilBoardBit )){
          message += "V"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Lime'") ;          
        }else{
          if ( evalve[k].OnCoilBoardBit == BoardBit ){
            message += "VE"+String(k+1) ;  
            bgcolor = F("bgcolor = 'SpringGreen'") ;         
          }
          if ( evalve[k].OffCoilBoardBit == BoardBit ){
            message += "VA"+String(k+1) ;  
            bgcolor = F("bgcolor = 'Crimson'") ;          
          }
        }
        if ( evalve[k].FeedbackBoardBit == BoardBit ){
          message += "VF"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Orange'") ;          
        }
      }
      for ( k = 0 ; k < MAX_FERT ; k ++ ) {
        if ( efert[k].BoardBit == BoardBit ) {
          message += "X"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Blue'") ;          
        }
      }
      for ( k = 0 ; k < MAX_FILTER ; k ++ ) {
        for ( kk = 0 ; kk < MAX_CANISTERS ; kk ++ ) {
          if ( efilter[k].BoardBit[kk] == BoardBit ) {
            message += "F"+String(k+1)+"C"+String(kk+1) ;  
            bgcolor = F("bgcolor = 'Magenta'") ;
          }
        }
      }
      server.sendContent("<td " + bgcolor + ">" + message + "</td>");
    }
    
    server.sendContent(F("</tr>"));
  }
  server.sendContent(F("</table><br><table title='Legend'><tr><td>V</td><td bgcolor='Lime'>Single Acting Valve Energise</td></tr>"));
  server.sendContent(F("<tr><td>VE</td><td bgcolor='SpringGreen'>Double Acting Valve On</td></tr>"));
  server.sendContent(F("<tr><td>VA</td><td bgcolor='Crimson'>Double Acting Valve Off</td></tr>"));
  server.sendContent(F("<tr><td>X</td><td bgcolor='Blue'>Fertigation Pump</td></tr>"));
  server.sendContent(F("<tr><td>F</td><td bgcolor='Magenta'>Filter</td></tr>"));
  server.sendContent(F("<tr><td>VF</td><td bgcolor='Orange'>Valve FeedBack - Input</td></tr>"));
  server.sendContent(F("</table>"));
  SendHTTPPageFooter();  
//  server.sendContent(F("<br><a href='/scan'>I2C Scan</a><br><a href='/'>Home</a></body></html>\r\n"));
}


String strPINName(int iPin,int *iTmp,int iPinType)
{
  *iTmp = 0 ;
  String pinname = "" ;
  switch(iPinType){
    case 1:   // digital pins
#if defined(ESP32)
      switch(iPin){
        case 0: pinname = F("GPIO 0 DIO") ; break;
        case 5: pinname = F("GPIO 5 DIO") ; break;
        case 12: pinname = F("GPIO 12 DIO") ; break;
        case 13: pinname = F("GPIO 13 DIO") ; break;
        case 14: pinname = F("GPIO 14 DIO") ; break;
        case 16: pinname = F("GPIO 16 DIO") ; break;
        case 17: pinname = F("GPIO 17 DIO") ; break;
        case 18: pinname = F("GPIO 18 DIO") ; break;
        case 19: pinname = F("GPIO 19 DIO") ; break;
        case 23: pinname = F("GPIO 23 DIO") ; break;
        case 25: pinname = F("GPIO 25 DIO") ; break;
        case 26: pinname = F("GPIO 26 DIO") ; break;
        case 27: pinname = F("GPIO 27 DIO") ; break;
        case 2: pinname = F("GPIO 2 ADC") ; break;
        case 4: pinname = F("GPIO 4 ADC") ; break;
        case 34: pinname = F("GPIO 34 ADC") ; break;
        case 35: pinname = F("GPIO 35 ADC") ; break;
        case 36: pinname = F("GPIO 36 ADC") ; break;
        case 39: pinname = F("GPIO 39 ADC") ; break;
        default: pinname = F("- UNKNOWN-") ; *iTmp = 1 ; break;
      }
#elif defined(ESP8266)
      switch(iPin){
        case 0: pinname = F("GPIO 0 - D3") ; break;
        case 1: pinname = F("GPIO 1 - D1 TXD0") ; break;
        case 2: pinname = F("GPIO 2 - D9") ; break;
        case 3: pinname = F("GPIO 3 - D0 - RXD0") ; break;
        case 4: pinname = F("GPIO 4 - I2C SDA - Dont Use") ; break;
        case 5: pinname = F("GPIO 5 - I2C SCL - Dont Use") ; break;
        case 6: pinname = F("GPIO 6 - SDCLK - NA Dont Use") ; break;
        case 7: pinname = F("GPIO 7 - SDD0 - NA Dont Use") ; break;
        case 8: pinname = F("GPIO 8 - SDD1 - NA Dont Use") ; break;
        case 9: pinname = F("GPIO 9 - SDD2 - NA ? ") ; break;
        case 10: pinname = F("GPIO 10 - SDD3 - NA ? ") ; break;
        case 11: pinname = F("GPIO 11 - SDCMD - NA Dont Use") ; break;
        case 12: pinname = F("GPIO 12 - D12 - ") ; break;
        case 13: pinname = F("GPIO 13 - D11 - RXD2") ; break;
        case 14: pinname = F("GPIO 14 - D13") ; break;
        case 15: pinname = F("GPIO 15 - D10 -  TXD2") ; break;
        case 16: pinname = F("GPIO 16 - D2 -  Wake") ; break;
        default: pinname = F("- UNUSED -") ; *iTmp = 1  ; break;
      }
#endif      
    break;
    default:  // analog pins
#if defined(ESP32)
      switch(iPin){
        case 2: pinname = F("GPIO 2 ADC") ; break;
        case 4: pinname = F("GPIO 4 ADC") ; break;
        case 34: pinname = F("GPIO 34 ADC") ; break;
        case 35: pinname = F("GPIO 35 ADC") ; break;
        case 36: pinname = F("GPIO 36 ADC") ; break;
        case 39: pinname = F("GPIO 39 ADC") ; break;
        default: pinname = F("- UNKNOWN-") ; *iTmp = 1 ; break;
      }
#elif defined(ESP8266)
      switch(iPin){
        case 0: pinname = F("ADC") ; break;
        default: pinname = F("- UNKNOWN-") ; *iTmp = 1 ; break;
      }  
#endif      
    break;
  }
  return(pinname);
}

void ResetADCCalInfo(){
  int i ,j ; 
  sprintf(ghks.ADC_Unit,"kPa\0") ;
  ghks.ADC_Cal_Ofs = -170.0 ;
  ghks.ADC_Cal_Mul = 640.0 ;
  ghks.ADC_Alarm_Mode = 0 ;               // high low etc
  ghks.ADC_Alarm1 = 0 ;
  ghks.ADC_Alarm2 = 0 ;                   // 
  ghks.ADC_Input_PIN1 = 25 ;
  ghks.ADC_Input_PIN2 = 26 ;  
  ghks.ADC_Alarm_Delay = 60 ; 

  for ( i = 0 ; i < ADC_MAX_CHAN ; i++ ) {
    adcs.chan[i].ADC_RAW = 0 ;
    adcs.chan[i].ADC_Value = 0 ;
    adcs.chan[i].ADC_Cal_Mul = 1.0 ;
    adcs.chan[i].ADC_Cal_Ofs = 0.0 ;
    sprintf(adcs.chan[i].ADC_Unit , "kPa\0") ;      // units for display
//    adcs.chan[i].ADC_Alarm_Mode[j] = 0 ;
//    adcs.chan[i].ADC_Alarm[j] = 0 ;
//    adcs.chan[i].ADC_Alarm_Delay[j] = 60 ;
    switch(i){
      case 0:
        adcs.chan[i].ADC_Input_PIN = 2 ;
      break;
      case 1:
        adcs.chan[i].ADC_Input_PIN = 4 ;
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

bool LocalPINOK ( int iTest){
bool bRet = false ;  
  switch (iTest){
    case 12:
    case 13:
    case 17:
    case 23:
    case 25:
    case 34:
    case 35:
    case 36:
    case 39:
    case 2:
    case -1:
      bRet = true ;
    break;
  }
  return(bRet);
}


