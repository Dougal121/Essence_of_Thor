


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
//        Serial.println("PCF8574 Output " + String(j) + " Board " + String(k) + " time " + String(PulseTime) + " state " + String(State));
      if ( k != -1 ) { // make sure the address is valid !!!
        if ( PulseTime > 0 ){
          IOEXP[k].pulsepin(Bit,PulseTime,State) ;              
        }else{
          IOEXP[k].write(Bit,State) ;      
        }
      }
    break;
    case 1:  // MCP23017
//      Serial.println("Activate Output Bit " + String(Bit) + " Board " + String(Board) + " Pulse Time " + String(PulseTime) + " State " + String(State));
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
//        Serial.println("MCP23017 Output " + String(j) + " Board " + String(k) + " time " + String(PulseTime) + " state " + String(State));
        if ( PulseTime > 0 ){
          mcp[k].pulsepin(j,PulseTime,State) ;        
        }else{
          mcp[k].digitalWrite( j , State) ;              
        }        
      }
    break;
    case 2:  // LOCAL I/O
//      Serial.println("Activate Local Output Bit " + String(Bit) + " Board " + String(Board) + " Pulse Time " + String(PulseTime) + " State " + String(State));
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
    case 4: // HC595 
//        Serial.println("Activate HC595 Output Bit " + String(Bit) + " Board " + String(Board) + " Pulse Time " + String(PulseTime) + " State " + String(State));
        if (( Bit >= 0 ) && ( Bit <= 16 ))  {
          if ( PulseTime > 0 ){
            HCIO.setBit(Bit,State) ;
            HCIO.update();
            delay(PulseTime);          
            HCIO.setBit(Bit,!State) ;
            HCIO.update();
          }else{
            HCIO.setBit(Bit,!State) ;  // state is reversed ??? it just is
            HCIO.update();
          }
        }
    break;
    case 5: // RS485 Modbus
        if (( Bit >= 0 ) && ( Bit <= 16 ))  {
          modbus.writeCoil( eboard[Board].Address, Bit, State ); 
        }   
    break;
    case 6: // TCP Modbus
        if (( Bit >= 0 ) && ( Bit <= 16 ))  {
//          modbusTCP.writeCoil( eboard[Board].Address, Bit, State ); 
        }   
    break;
  }
}



int MaxBoardOutputs(int Board){
  switch ( eboard[Board].Type ){
    case 0:  // PCF8574
      return(8);
    break;
    case 4:  // HCIO   HC595 on board IO 16 realys
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
    case 5:
      return(16);
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

    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 2: // Save values
          LoadParamsFromEEPROM(false);
          bSendSaveConfirm = true ;
//          Serial.println("Save to EEPROM");
        break;
        case 142:  
          ResetLocalIO();
        break;
      }
    }

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

//  bool z = digitalPinCanOutput(34);

  server.sendContent(F("<br><center><b>Local Processor Pin I/O Map</b><br><table border=1 title='Local I/O Map'><tr><th>Bit</th><th>PIN</th><th>.</th></tr>"));
  
  for (i = 0; i < MAX_LOCAL_IO ; i++) {     // for each Bit allocate a local I/O pin against it
    message = F("<tr>");
    message += "<tr><form method=get action=" + server.uri() + "><td>"+String(i)+"</td><td><select name='i"+String(i)+"'>";
    if ( elocal.IOPin[i] == 255){
      bgcolor = F(" SELECTED ");
    }else{
      bgcolor = "";            
    }
    message += "<option value=255 "+ bgcolor +">Not Connected " ;
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
    message += F("</select></td><td><input type='submit' value='SET'></form></td></tr>\n\r");
    server.sendContent(message);
    message = "" ;
  }
  server.sendContent("<tr><td align=center colspan=3><a href='" + server.uri() + "?command=142'>Reset Local IO Defaults</a></td></tr></table>");


  server.sendContent(F("<br><center><b>CPU GPIO Pin Caps</b><br><table border=1 title='Local I/O Map'><tr><th>Pin</th><th>IO MUX<br>Function</th><th>Output<br>Index</th><th>Drive<br>caps</th><th>Pullups</th><th>Pulldown</th><th>Input<br>Enable</th><th>Output<br>Enable</th><th>Perif<br>Output</th><th>Output<br>Inverse</th><th>Open<br>Drain</th><th>Sleep<br>Mode</th></tr>"));
  for ( i = 0 ; i < GPIO_PIN_COUNT ; i++){
    gpio_io_config_t myiopin;
    gpio_get_io_config((gpio_num_t)i, &myiopin);
    bool bOut = digitalPinCanOutput(i);
    String strOut = bOut ? "bgcolor='lime'" : "" ; 
    bool bVP = digitalPinIsValid(i);
    String strVP = bVP ? "bgcolor='lime'" : "" ; 
    message = "<tr align=center><td "+strVP+">GPIO "+String(i)+"</td><td>"+String(myiopin.fun_sel,HEX)+"</td><td>"+String(myiopin.sig_out,HEX)+"</td><td>"+capName(myiopin.drv)+"</td><td>"+bool2CB(myiopin.pu,"pu"+String(i))+"</td><td>"+bool2CB(myiopin.pd,"pd"+String(i))+"</td><td>"+bool2CB(myiopin.ie,"ie"+String(i))+"</td><td "+strOut+">"+bool2CB(myiopin.oe,"oe"+String(i))+"</td><td>"+bool2CB(myiopin.oe_ctrl_by_periph,"oe_ctrl_by_periph"+String(i))+"</td><td>"+bool2CB(myiopin.oe_inv,"oe_inv"+String(i))+"</td><td>"+bool2CB(myiopin.od,"od"+String(i))+"</td><td>"+bool2CB(myiopin.slp_sel,"slp_sel"+String(i))+"</td></tr>\r\n" ;
    server.sendContent(message);
  }
  server.sendContent(F("</table>"));



  SendHTTPPageFooter();   
}

String bool2CB(bool myVal , String name){
  String MyCheck = myVal ? "CHECKED" : "" ;
  String strRet = "<input type='checkbox' name='" + name + "' "+String(MyCheck)+ ">";
  return( strRet );
}
const char* capName(gpio_drive_cap_t cap)
{
    switch (cap)
    {
        case GPIO_DRIVE_CAP_0: return "Weak";
        case GPIO_DRIVE_CAP_1: return "Low";
        case GPIO_DRIVE_CAP_2: return "Medium";
        case GPIO_DRIVE_CAP_3: return "Strong";
        default: return "Unknown";
    }
}


//                         I/O Database scan
void ioScan() {
  int i , ii ;
  uint8_t j , k , kk ;
  uint8_t BoardBit ;
  String message ;  
  String message2 ;  
  String bgcolor ;  
  uint8_t  error ;      
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
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 0: // Reset Doubble
          ResetValveInfo(true);
        break;
        case 1: // Reset Single
          ResetValveInfo(false);
        break;
        case 10:  // Factory reset
          ResetBoardInfo();
        break;
        case 2: // Save values
          LoadParamsFromEEPROM(false);
        break;        
      }
    }    
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
      i = String(server.argName(j)).indexOf("b" + String(ii));
      if (i != -1){                                     
        k = String(server.arg(j)).toInt() ;  // board address free form entry
        eboard[ii].Address = k ;
      }    
      i = String(server.argName(j)).indexOf("t" + String(ii));
      if (i != -1){            
        int iTmp = String(server.arg(j)).toInt();                                         //                 
        if ((iTmp == 4) && (eboard[ii].Type != iTmp) && (eboard[ii].Extra[0]==0)){      
          eboard[ii].Extra[0] = 14; // setup if not set
          eboard[ii].Extra[1] = 13;
          eboard[ii].Extra[2] = 12;
          eboard[ii].Extra[3] = 5;
        }
        eboard[ii].Type = iTmp ;        
      }      
      i = String(server.argName(j)).indexOf("m" + String(ii));
      if (i != -1){                                                     // 
        eboard[ii].Translate = String(server.arg(j)).toInt();        
      }      
      for ( int z = 0 ; z < 5 ; z++ ){  // scan for the extra paramater bytes
        i = String(server.argName(j)).indexOf("e" + String(z) + String(ii));
        if (i != -1){                                                     // 
          eboard[ii].Extra[z] = String(server.arg(j)).toInt();        
        }      
      }
    }
  }
  SendHTTPHeader();
  message = "<a href='" + server.uri() + "?command=2'>Save Parameters to EEPROM</a><br>\r\n" ;  
  message += F("<br><center><b>I/O DB Check</b><br><table border=1 title='I/O DB Check'><tr><th rowspan=2>Board</th><th rowspan=2>IO<br>Address</th><th rowspan=2>Board<br>Type</th><th rowspan=2>Bit<br>Mapping</th><th rowspan=2>Extra Data<br>Bytes</th><th rowspan=2>.</th><th colspan=16>Output / Input</th></tr><tr>\r\n");
  for (i = 0; i < 16; i++) {
    message += "<th>"+String(i,HEX)+"</th>";
  }
  message += F("</tr>\r\n");
  server.sendContent(message);
  message = "" ;
  
  for (i = 0; i < MAX_BOARDS ; i++) {                             // for each board do I2C test and show if online
    server.sendContent(F("<tr>"));
    if ((eboard[i].Type >=0 ) && (eboard[i].Type <= 3 ) && (eboard[i].Address < 127 )){ // ie the I2C ones 
      bgcolor = "title='Device not found'" ;
      Wire.beginTransmission(eboard[i].Address);
      error = Wire.endTransmission();
      if ( error == 0 ){
        bgcolor = F("bgcolor = 'Lime' title='Device On Line'") ;
      }else{
        if (error == 4){
          bgcolor = F("bgcolor = 'Red' title='Device Faulted'") ;      
        }
      }
    }
    else
    {
      bgcolor = "";
    }
    server.sendContent("<tr><form method=get action=" + server.uri() + "><td "+bgcolor+">"+String(i)+"</td><td>");
    if ((eboard[i].Type==0) || (eboard[i].Type == 1)){
      message += "<select name='a"+String(i)+"'>";
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
      }
      message += "</select>";
    }else{
      message += "<input type='text' name='b"+String(i)+"' value='"+String(eboard[i].Address)+"' maxlength=3 size=6>";
    }
    message += "</td><td><select name='t"+String(i)+"'>";
    for ( int k = 0  ; k < MAX_BOARD_TYPES ; k++){
      message +=  "<option value='" + String(k) + "'" ; 
      if (eboard[i].Type == k ){
        message +=  " SELECTED";
      }
      message += " >" + strDeviceType(k) + " " ;
    }
    message += F("</select></td><td>");
    message += "<select name='m"+String(i)+"'>" ;
    switch (eboard[i].Translate){
      case 0:
        message +=  F("<option value='0' SELECTED>None <option value='1' >DIL AB") ;    
      break;
      default:
        message +=  F("<option value='0'>None <option value='1' SELECTED>DIL AB") ;    
      break;
    }    
    message += F("</select></td><td>");
    for (j = 0 ; j < MAX_EXTRA_BOARD_BYTES ; j ++ ){
         message += "<input type='text' name='e"+String(j)+String(i)+"' value='"+String(eboard[i].Extra[j])+"' maxlength=3 size=3>";
    }
    message += F("</td><td><input type='submit' value='SET'></form></td>");
    for (j = 0 ; j < 16 ; j ++ ){     //   each bit on each board
      BoardBit = ( i << 4 ) | j ;
      message2 = "" ;
      bgcolor = "" ;
      for ( k = 0 ; k < MAX_VALVE ; k ++ ) {
        if (( evalve[k].OnCoilBoardBit == BoardBit ) && ( evalve[k].OnCoilBoardBit == evalve[k].OffCoilBoardBit )){
          message2 += "V"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Lime'") ;          
        }else{
          if ( evalve[k].OnCoilBoardBit == BoardBit ){
            message2 += "VE"+String(k+1) ;  
            bgcolor = F("bgcolor = 'SpringGreen'") ;         
          }
          if ( evalve[k].OffCoilBoardBit == BoardBit ){
            message2 += "VA"+String(k+1) ;  
            bgcolor = F("bgcolor = 'Crimson'") ;          
          }
        }
        if ( evalve[k].FeedbackBoardBit == BoardBit ){
          message2 += "VF"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Orange'") ;          
        }
      }
      for ( k = 0 ; k < MAX_FERT ; k ++ ) {
        if ( efert[k].BoardBit == BoardBit ) {
          message2 += "X"+String(k+1) ;  
          bgcolor = F("bgcolor = 'Blue'") ;          
        }
      }
      for ( k = 0 ; k < MAX_FILTER ; k ++ ) {
        for ( kk = 0 ; kk < MAX_CANISTERS ; kk ++ ) {
          if ( efilter[k].BoardBit[kk] == BoardBit ) {
            message2 += "F"+String(k+1)+"C"+String(kk+1) ;  
            bgcolor = F("bgcolor = 'Magenta'") ;
          }
        }
      }
      message += "<td " + bgcolor + ">" + message2 + "</td>";
    }
    
    message += F("</tr>\r\n");
    server.sendContent(message);
    message = "" ;
  }
  message = "<tr><form  method=get action=" + server.uri() + "><td colspan=3><input type='hidden' name='command' value='1'><input type='submit' value='Factory Reset Signle Acting'>   </td></form><form  method=get action=" + server.uri() + "><td colspan=4>   <input type='hidden' name='command' value='0'><input type='submit' value='Factory Reset Double Acting'></td></form><form  method=get action=" + server.uri() + "><td colspan=4>   <input type='hidden' name='command' value='10'><input type='submit' value='Factory Reset Board Data'></td></form></tr>\r\n";
  server.sendContent(message);
  message = F("</table><br><table title='Legend'><tr><td>V</td><td bgcolor='Lime'>Single Acting Valve Energise</td></tr>");
  message += F("<tr><td>VE</td><td bgcolor='SpringGreen'>Double Acting Valve On</td></tr>");
  message += F("<tr><td>VA</td><td bgcolor='Crimson'>Double Acting Valve Off</td></tr>");
  message += F("<tr><td>X</td><td bgcolor='Blue'>Fertigation Pump</td></tr>");
  message += F("<tr><td>F</td><td bgcolor='Magenta'>Filter</td></tr>");
  message += F("<tr><td>VF</td><td bgcolor='Orange'>Valve FeedBack - Input</td></tr>");
  message += F("</table>");
  server.sendContent(message);
  message = "" ;
  SendHTTPPageFooter();  
//  server.sendContent(F("<br><a href='/scan'>I2C Scan</a><br><a href='/'>Home</a></body></html>\r\n"));
}

String strDeviceType(uint8_t type)
{
  String strRet = "" ;
  switch(type){
    case 0:   // 
      strRet = F("PCF8574"); 
      break;
    case 1:   // 
      strRet = F("MCP23017"); 
      break;
    case 2:   // 
      strRet = F("Local CPU");
      break;
    case 3:   // 
      strRet = F("-- None --"); 
      break;
    case 4:   // 
      strRet = F("Local HC595"); 
      break;
    case 5:   // 
      strRet = F("RTU Modbus"); 
      break;
    case 6:   // 
      strRet = F("TCP Modbus");
      break;
  }
  return(strRet);
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
        case 12: pinname = F("GPIO 12 ADC*") ; break;
        case 13: pinname = F("GPIO 13 ADC*") ; break;
        case 14: pinname = F("GPIO 14 ADC*") ; break;
        case 25: pinname = F("GPIO 25 ADC*") ; break;
        case 26: pinname = F("GPIO 26 ADC*") ; break;
        case 27: pinname = F("GPIO 27 ADC*") ; break;
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



bool LocalPINOK ( int iTest){
bool bRet = false ;  
  switch (iTest){
    case 12:
    case 13:
    case 16:
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


