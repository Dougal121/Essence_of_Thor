


void fertigation_min (void) {
int i , j , k , v , jj ;
uint8_t fbit , board ;

  for ( j = 0 ; j < MAX_FERT ; j++){
//    if ((efert[j].AType & 0x90 ) != 0 ) {
      vfert[j].bRun = false ;
      vfert[j].Flowrate = 0 ;
//    }
  }
  k = dayOfWeek(now()) ;
  for ( i = 0 ; i < MAX_VALVE ; i++ ) {                                                                  // for each valve work what fertigation units are on
    if (( vvalve[i].lATTG > 1 ) || ( vvalve[i].lTTG > 1 ) || ((( evalve[i].TypeMaster & 0x40 ) != 0 ) && ( vvalve[i].iFB != 0 ))  ) {  // cut out one minute short
       if ( evalve[i].Fertigate != 0 ){                                                                  // check if switched on first
          for ( j = 0 ; j < MAX_FERT ; j++){
            if (((evalve[i].Fertigate & ( 0x01 << j )) != 0 ) && ( efert[j].CurrentQty > 0 )) {          // check which fertigation channels are active
              if ((( efert[j].DaysEnable & ( 0x01 << (k-1) )) != 0  ) && (( efert[j].DaysEnable & ( 0x80 )) != 0  ) && !bFertDisable ) {   // check if enabled for these days
                vfert[j].Flowrate += evalve[i].Flowrate ;                                                // work out the total flow rate 
                vfert[j].bRun = true ;    
                if ((efert[j].AType & 0xC0 ) != 0 ) {                                                    // if a chemical valve see what channel has the pump and keep it enabled
                  if ( MAX_FERT == 16 ){
                    v = ( efert[j].AType & 0x0f ) ;                                                        // 16 outputs (god help us!!! - eh) 
                  }else{
                    v = ( efert[j].AType & 0x07 ) ;                                                        // 8 outputs                       
                  }
//                  if ( v != 0 ) {                                                                        // cant use 0 as master solenoid ?? why
                    vfert[v].bRun = true ;                    
                    vfert[v].Flowrate += evalve[i].Flowrate ;                                            // work out the total flow rate 
//                  }                  
                }
              }
            }
          }     
       }
    }          
  }
  for ( j = 0 ; j < MAX_FERT ; j++){
    if (( efert[j].AType & 0x30 ) != 0  ) {
      board = ( efert[j].BoardBit & 0xf0 ) >> 4 ;
      fbit = ( efert[j].BoardBit & 0x0f ) ;
      if ( vfert[j].bRun ) {
        if ((( efert[j].DaysEnable & 0x80 ) != 0 ) && !bFertDisable ){             // if it is enabled then start output
          if ( vfert[j].lTTG == 0 ) {                          // ie not in cycle
            if ( efert[j].BaseFlow > 0 ){
              vfert[j].lTTG = ( efert[j].OnTime * vfert[j].Flowrate / efert[j].BaseFlow ) + 1 ;
            }else{
              vfert[j].lTTG = efert[j].OnTime + 1 ;          
            }
            if (( efert[j].AType & 0x10 )== 0  ) {            
              efert[j].CurrentQty -= ( efert[j].PumpRate * vfert[j].lTTG ) ;  // only do if a normal pump
            }
            vfert[j].bOnOff = true ;  
            ActivateOutput(board , fbit , false , 0 ) ;
/*            for ( jj = 0 ; jj < MAX_FERT ; jj++){
              if (((efert[jj].AType & 0x40 ) != 0) && ( vfert[jj].bOnOff )&& (( efert[jj].AType & 0x0f ) == j )) {    
                efert[jj].CurrentQty -= ( efert[j].PumpRate * vfert[j].lTTG ) ;                  
                vfert[jj].bOnOff = true ;  
              }
            }*/  
          }
        }      
      }else{ // no run - It's sorta counter intuative but don't do anything to a master valve thats not running ... let it take it's natural course.
/*        vfert[j].lTTG = 0 ;
        if (vfert[j].bOnOff) { // if on turn off
          vfert[j].bOnOff = false ;
//          ActivateOutput(board , fbit , true , 0 ) ;  // active low so off is TRUE                                
        }  
        vfert[j].bRun = false ;*/
      } 
    }
  }
}


int fertigation_sec (void) {
int i , k , j ;
float ot  ;
uint8_t fbit , board ;
bool bFValve = false ;
bool bWValve = false ;
  k = 0 ;
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ((efert[i].AType & 0x40 ) != 0  ) {  //  chemical valve coil    
      if ( vfert[i].bRun ) {
        bFValve = true ;
      }
    }
  }
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ((efert[i].AType & 0x80 ) != 0 ) {  // wash valve coil   
      if ( bFValve ) {
        vfert[i].lTTG = efert[i].OffTime ;  // this is a arm sequence - ready to flush
      }else{
        if ( vfert[i].lTTG > 0 ){     // wait till there is a TTG and the other solinoids are all off
          bWValve = true ;            // we are in flush mode  
        }
      }
    }
  }

  for ( i = 0 ; i < MAX_FERT ; i++){
    board = ( efert[i].BoardBit & 0xf0 ) >> 4 ;
    fbit = ( efert[i].BoardBit & 0x0f ) ;
    switch ( ( efert[i].AType >> 4 ) ) {
      case 4: // Chemical Supply valve  
          if ( vfert[i].bRun ){  // if hasa flow etc turn on else off
            ActivateOutput(board , fbit , false , 0 ) ; // on active low  
            vfert[i].bOnOff = true ;
          }else{
            ActivateOutput(board , fbit , true , 0 ) ;  // off active high
            vfert[i].bOnOff = false ;
          }
      break;
      case 8: // wash water valve
        if ( bWValve ){     // wait till there is a TTG and the other solinoids are all off
          vfert[i].lTTG-- ;
          ActivateOutput(board , fbit , false , 0 ) ;   // ON active high (flipped)
          vfert[i].bOnOff = true ;
        }else{
          ActivateOutput(board , fbit , true , 0 ) ;     // off       
          vfert[i].bOnOff = false ;
        }
      break;
      case 2:                                     // 0x02   master pump 
        if ( bWValve ){                           // run on if wash sequence is activated
          ActivateOutput(board , fbit , false , 0 ) ;
          vfert[i].bOnOff = true ;
          vfert[i].lTTG = 2 ;                     // just hold it in a wee bit more than a tad
        }                                         // fall through don't break
      default:                                    // 0x01   garden variety proportioning pump
        if ( vfert[i].lTTG > 0 ){
          vfert[i].lTTG-- ;
        }
        if ( vfert[i].lTTG == 0 ){
          if ( vfert[i].bOnOff ){  // on so turn off
            ActivateOutput(board , fbit , true , 0 ) ;     // OFF        
            vfert[i].bOnOff = false ;
            if ( efert[i].BaseFlow > 0 ){
              vfert[i].lTTG = efert[i].OffTime ; // 
            }else{
              vfert[i].lTTG = efert[i].OffTime ;          
            }
          }else{   // off so turn on maybe
            if ( vfert[i].bRun ) {  // if hasa flow etc turn on else off
              if (( efert[i].BaseFlow > 0 ) && ( vfert[i].Flowrate >= 1  )){
                ot = efert[i].OnTime * vfert[i].Flowrate / efert[i].BaseFlow ;
              }else{
                ot = efert[i].OnTime ;          
              }
              if ( ot < 1 ){
                ot = 1 ;       
              }  
              if (( efert[i].AType & 0x10 ) != 0){   // normal pump
                efert[i].CurrentQty -= ( efert[i].PumpRate * ot ) ; // dont decrement the tank if a master pump only do if normal pump
              }else{
                ot = 0 ; // start again for master pump and add up all the on times
                for ( j = 0 ; j < MAX_FERT ; j++){
                  if (((efert[j].AType & 0xc0 ) != 0) && ( vfert[j].bOnOff )&& (( efert[j].AType & 0x0f ) == i ) && ( efert[j].CurrentQty > 0 )) {    
                    if (( efert[i].BaseFlow > 0 ) && ( vfert[i].Flowrate >= 1  )){
                      ot += efert[j].OnTime * vfert[i].Flowrate / efert[i].BaseFlow ;
                    }else{
                      ot += efert[j].OnTime  ;          
                    }
                    efert[j].CurrentQty -= ( efert[i].PumpRate * ot ) ;   // need to decrement the  base solenoid/tank                
                  }
                }  
              } 
              vfert[i].lTTG = ot ;       
              if ( ot > 0 ) {   
                vfert[i].bOnOff = true ;
                ActivateOutput(board , fbit , false , 0 ) ;
              }
            }   // bRun
          }
        }       // lttg = 0
        if ( vfert[i].bOnOff ) {
          k++ ;
        }
      break ;    
    }
  }
  return(k) ;
}

//  This will return the number of writes it does or will do --- pass in if you want it to actally save
int SaveCurrentQty(bool bWrite){
int i ;
int j = 0 ;
int k ;
float tmpFloat ;
  for ( i = 0 ; i < MAX_FERT ; i++){
    k = efertAddress + ( i * sizeof(fertigation_t))  + 2 ;
    EEPROM.get(k,tmpFloat) ;
    if ( efert[i].CurrentQty != tmpFloat ){
      j++ ;
 //     Serial.println("mem " + String(efert[i].CurrentQty) + " e2 " + String(tmpFloat));
      if (bWrite) {
        EEPROM.put(k,efert[i].CurrentQty) ;
      }
    }
  }
  if ((bWrite) && ( j > 0 )) {
    EEPROM.commit(); 
  }
  return(j);
}

void handleFert(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
//  String MyNum ;  
//  String MyColor ;
  String MyColor2 ;
  byte mac[6];
  uint8_t iPage = 0 ;
  int iAType = 0 ;
  String MyCheck , MyColor , MyNum , MyCheck2 ;

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
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
        case 5: // Fertigation
          iPage = 2 ;
        break;
      }
    }    
    for ( ii = 0 ; ii < MAX_FERT ; ii++){                             // handle all the fertigation control arguments
      i = String(server.argName(j)).indexOf("xnum" + String(ii));  // this is a parser cheat
      if (i != -1){  // 
        if ( iPage == 0 ){
          efert[ii].DaysEnable = 0 ; // wipe all the data - "shoot them all and let god sort them out"
        }
      }      
      i = String(server.argName(j)).indexOf("xdes" + String(ii));
      if (i != -1){  // Current quantity in system   ( Dont pump if < 0 )
        String(server.arg(j)).toCharArray(efert[ii].description , sizeof(efert[ii].description)) ;        
      }        
      i = String(server.argName(j)).indexOf("xaty" + String(ii));
      if (i != -1){  // 
        efert[ii].AType &= 0x0f ;
        efert[ii].AType |= ( String(server.arg(j)).toInt() << 4 ) ;
      }        
      i = String(server.argName(j)).indexOf("xatm" + String(ii));
      if (i != -1){  // 
        iAType = String(server.arg(j)).toInt()   ;
        iAType = constrain(iAType,0,MAX_FERT );
        efert[ii].AType &= 0xf0 ;
        efert[ii].AType |= ((uint8_t) iAType & 0x0f ) ;
      }        
/*      i = String(server.argName(j)).indexOf("xmty" + String(ii));
      if (i != -1){  // 
        efert[ii].AType = String(server.arg(j)).toInt()  ;
      }*/        
      i = String(server.argName(j)).indexOf("xcqt" + String(ii));
      if (i != -1){  // Current quantity in system   ( Dont pump if < 0 )
        efert[ii].CurrentQty = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xmqt" + String(ii));
      if (i != -1){  // Max quantity - used to reset qty
        efert[ii].MaxQty = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xont" + String(ii));
      if (i != -1){  // On time basis
        efert[ii].OnTime = String(server.arg(j)).toInt() ;
      }        
      i = String(server.argName(j)).indexOf("xoft" + String(ii));
      if (i != -1){  // Off time basis
        efert[ii].OffTime = String(server.arg(j)).toInt() ;
      }        
      i = String(server.argName(j)).indexOf("xbas" + String(ii));
      if (i != -1){  // pump base rate
        efert[ii].BaseFlow = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xpmr" + String(ii));
      if (i != -1){  // pump rate
        efert[ii].PumpRate = String(server.arg(j)).toFloat() ;
      }        
      i = String(server.argName(j)).indexOf("xbba" + String(ii));
      if (i != -1){  // board address
        efert[ii].BoardBit = ( efert[ii].BoardBit & 0x0F ) | (( 0x0F & String(server.arg(j)).toInt())<<4)  ;
      }        
      i = String(server.argName(j)).indexOf("xbbb" + String(ii));
      if (i != -1){  // board bit
        efert[ii].BoardBit = ( efert[ii].BoardBit & 0xF0 ) | ( 0x0F & String(server.arg(j)).toInt())  ;
      }
      for ( kk = 0 ; kk < 8 ; kk++ ) {
        i = String(server.argName(j)).indexOf("x"+String(ii)+"dw"+String(kk));
        if (i != -1){  //  
          if ( String(server.arg(j)).length() == 2 ){ // only put back what we find
            efert[ii].DaysEnable |= ( 0x1 << kk )  ;
          }
        }                
      }              
    }

  }
  
  SendHTTPHeader();

  server.sendContent(F("<br><b>Fertigation Setup</b>"));
  message = F("<table border=1 title='Fertigation Setup'>") ;
  message += F("<tr><th rowspan=2>Tank</th>") ;
  if (iPage==0) {
    message += F("<th colspan=8>Enable</th>");
    message += F("<th>Current Qty</th><th>Max Qty</th><th rowspan=2>Description</th><th rowspan=2>Type</th><th rowspan=2>Master</th>");
  }  
  if (iPage==2) {
    message += F("<th>On Time</th><th>Off time</th><th>Base Flow</th><th>Pump Rate</th><th colspan=2>Connection</th></tr>") ; 
  }
  message += F("<tr>") ;                   // second header row 
  if (iPage==0) {
    for ( j = 0 ; j < 8 ; j++ ){
      message += "<th>" + String(dayarray[j]) + "</th>" ;
    }
    message += F("<th>(l)</th><th>(l)</th>");
  }
  if (iPage==2) {
    message += F("<th>(s)</th><th>(s)</th><th>(l/s)</th><th>(l/s)</th><th>Board</th><th>Bit</th></tr>") ; 
  }
  server.sendContent(message) ;
  message = "" ;
  
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ((efert[i].DaysEnable & 0x80 ) == 0  ) {
      MyColor = F("bgcolor=red") ;
    }else{
      MyColor = F("bgcolor=green") ;
    }
    if (iPage==0) {
      message += "<tr><form method=post action=" + server.uri() + "><td align=center "+String(MyColor)+">"+String(i+1)+"</td>";           
    }
    if (iPage==2) {
      message += "<tr><form method=post action=" + server.uri() + "><td align=center "+String(MyColor)+">"+String(i+1)+" - "+String(efert[i].description)+"</td>";   
      message += F("<input type='hidden' name='command' value='5'>");
    }  
    message += "<input type='hidden' name='xnum"+String(i)+"' value='"+String(i)+"'>";
    if (iPage==0) {
      for (k = 0 ; k < 8 ; k++){      
        MyColor =  ""   ;  
        if ( ( efert[i].DaysEnable & (0x01 << k)) != 0 ){
          MyCheck = F("CHECKED")  ;    
          if ((k != 7 ) && ((efert[i].DaysEnable & 0x80 ) == 0  ) ){ // 
            MyColor =  F("bgcolor=red")   ;  
          }          
        }else{
          MyCheck = ""    ;      
        }
        message += "<td "+String(MyColor)+"><input type='checkbox' name='x"+String(i)+"dw"+ String(k)+"' "+String(MyCheck)+ "></td>";    
      }
      message += "<td><input type='text' name='xcqt"+String(i)+"' value='" + String(efert[i].CurrentQty) + "' maxlength=5 size=2></td><td><input type='text' name='xmqt"+String(i)+"' value='" + String(efert[i].MaxQty) + "' maxlength=5 size=2>";
      message += "<td><input type='text' name='xdes"+String(i)+"' value='" + String(efert[i].description) + "' maxlength=7 size=8></td>";
      message += "<td><select name='xaty"+String(i)+"'>" ;
      switch (efert[i].AType >> 4 ){
        case 2:    // master pump 
          message += F("<option value='1'>Proportioning Pump") ;
          message += F("<option value='2' SELECTED>Master Pump") ;
          message += F("<option value='4'>Chemical Valve") ;
          message += F("<option value='8'>Wash Valve") ;
        break;
        case 4:    // supply valve
          message += F("<option value='1'>Proportioning Pump") ;
          message += F("<option value='2'>Master Pump") ;
          message += F("<option value='4' SELECTED>Chemical Valve") ;
          message += F("<option value='8'>Wash Valve") ;
        break;
        case 8:    // wash valve
          message += F("<option value='1'>Proportioning Pump") ;
          message += F("<option value='2'>Master Pump") ;
          message += F("<option value='4'>Chemical Valve")  ;
          message += F("<option value='8' SELECTED>Wash Valve") ;
        break;
        default:    // garden variety proportioning pump
          message += F("<option value='1' SELECTED>Proportioning Pump") ;
          message += F("<option value='2'>Master Pump") ;
          message += F("<option value='4'>Chemical Valve") ;
          message += F("<option value='8'>Wash Valve") ;
        break;
      }
      message += "</select></td>" ;
      message += "<td><select name='xatm"+String(i)+"'>" ;
      for ( ii = 0 ; ii < MAX_FERT ; ii++){                             // handle all the fertigation control arguments
          if ((efert[i].AType & 0x0f) == ii) {
            message += "<option value='"+String(ii)+"' SELECTED>" + String(ii+1) ;          
          }else{
            message += "<option value='"+String(ii)+"'>" + String(ii+1) ;                        
          }
      }
      message += "</select></td>" ;
//        message += "<td><input type='text' name='xatm"+String(i)+"' value='" + String((efert[i].AType & 0x0f) + 1) + "' maxlength=1 size=3></td>";
    }
    if (iPage==2) {
      message += "<td><input type='text' name='xont"+String(i)+"' value='" + String(efert[i].OnTime) + "' maxlength=5 size=2></td><td><input type='text' name='xoft"+String(i)+"' value='" + String(efert[i].OffTime) + "' maxlength=5 size=2>";
      message += "<td><input type='text' name='xbas"+String(i)+"' value='" + String(efert[i].BaseFlow) + "' maxlength=5 size=2></td><td><input type='text' name='xpmr"+String(i)+"' value='" + String(efert[i].PumpRate) + "' maxlength=5 size=2>";
      message += "<td><input type='text' name='xbba"+String(i)+"' value='" + String((efert[i].BoardBit & 0xf0 )>>4 ) + "' maxlength=5 size=2></td><td><input type='text' name='xbbb"+String(i)+"' value='" + String(efert[i].BoardBit & 0x0F) + "' maxlength=5 size=2>";
    }
    message += F("<td><input type='submit' value='SET'></td></form></tr>") ;
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;


  SendHTTPPageFooter();  
}


void DisplayShowFertQue(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
  String MyValves ;  
  String MyColor ;
  String MyColor2 ;

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 333:  // zero all que
          ZeroFertQue();
        break;
      }
    }
  }
  
  SendHTTPHeader();

  message = F("<br><center><b>Fertigation Data Que</b><br><a href='/?command=333'>Zero/Reset Fertigation Data Que</a><br>\r\n");
  message += F("<table border=1 title='Fertigation Data Que'>\r\n");
  message += F("<tr><th>Log</th><th>Date</th><th>Tank</th><th>Valves</th><th>Amount</th></tr>\r\n");
  server.sendContent(message);
  message = "" ;
  for (int i = 0 ; i < MAX_FERT_LOGS ; i++ ) {
    if ( flq[i].RecDate != 0 ){
      MyColor = F("bgcolor='orange'")  ;
    }else{
      MyColor = F("bgcolor='yellowgreen'")  ;      
    }
    snprintf(buff, BUFF_MAX, "%02d/%02d/%04d %02d:%02d:%02d", day(flq[i].RecDate), month(flq[i].RecDate), year(flq[i].RecDate), hour(flq[i].RecDate), minute(flq[i].RecDate), second(flq[i].RecDate) );     
    MyValves = String(flq[i].Valves,BIN) + " - " + String(flq[i].Valves) ;
    server.sendContent( "<tr><td "+MyColor+">"+String(i)+"</td><td>"+String(buff)+"</th><th>"+flq[i].FertMixID+"</th><th>"+MyValves+"</th><th>"+flq[i].Amount+"</th></tr>\r\n");
  }
  server.sendContent(F("</table>\r\n"));    
  SendHTTPPageFooter();

}

void ZeroFertQue(void){
int i ;
  for (i = 0 ; i < MAX_FERT_LOGS ; i++ ) {
    flq[i].RecDate = 0 ;
    flq[i].FertMixID = 0 ;
    flq[i].Valves = 0 ;
    flq[i].Amount = 0 ;
  }
  return ;
}



