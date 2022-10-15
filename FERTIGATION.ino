


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
       if (( evalve[i].Fertigate != 0 ) && (( evalve[i].TypeMaster & 0x80 ) == 0 ))  {                   // check if switched on first and check its not a master valve
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
        if ((( efert[j].DaysEnable & 0x80 ) != 0 )&&(!bFertDisable)){             // if it is enabled then start output
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
bool bLowTank = false ;
bool bEmptyTank = false ;
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
                QueFertData(i, ( efert[i].PumpRate * ot ));              
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
                    QueFertData(j, ( efert[i].PumpRate * ot ));              
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
    if (( vfert[i].CurrentQtyPrev > 0 ) && ( efert[i].CurrentQty <= 0  )) {
      bEmptyTank = true;
    }
    if (( vfert[i].CurrentQtyPrev > SMTP.LowTankQty ) && ( efert[i].CurrentQty <= SMTP.LowTankQty  )) {
      bLowTank = true ;
    }
    vfert[i].CurrentQtyPrev = efert[i].CurrentQty ;
  }
  if ( bLowTank  && SMTP.bUseEmail){
    SendEmailToClient(1);  // tank low on chemical    
  }
  if ( bEmptyTank  && SMTP.bUseEmail){
    SendEmailToClient(0);  // tank out of chemical
  }  
  return(k) ;
}

bool IsDoingFert(void){
int i ;
bool bOn = false ;
  for ( i = 0 ; i < MAX_FERT ; i++){
    if ( vfert[i].bOnOff ){
      bOn = true ;
    }
  }
  return(bOn);
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

int QueFertData(int TankID, float Amount){
time_t MyTime = now() ;  
int i, j = 0 ;
int32_t MyValves = 0 ;

//  Serial.println("Start Que Fert Data");
  if ((ghks.ValveLogOptions & 0x40 )!=0 ){
    for (i = 0 ; i < MAX_FERT_LOGS ; i++ ) {
      if ( flq[i].RecDate == 0 ){   // the first empty record fill it
        j = i ;
        break ;  
      }else{
        if (flq[i].RecDate < MyTime) {  // otherwise look for the oldest record
          MyTime = flq[i].RecDate ;
          j = i ;
        }
      }
    }
    for (i = 0 ; i < MAX_VALVE ; i++ ) {  // work out the valve binary thingo  
      if ( vvalve[i].bOnOff != 0 ){
        MyValves += (int32_t)( 0x01 << i ) ; 
      }
    }
//    Serial.println("Que Record " + String(j) + " Tank "+ String(TankID) + " Valves " + String(MyValves)+ " Amount " + String(Amount));
    flq[j].RecDate = now() ;
    flq[j].FertMixID = TankID ;
    flq[j].Valves = MyValves ;
    flq[j].Amount = Amount ;

  }  
  return(j) ;
}

int SendFertQueData(void){
int j = 0 ;   
HTTPClient http;
String url ;
int httpCode ;

  if (WiFi.isConnected())  {
//    Serial.println("Start Send Fert Data");
    
    for (int i = 0 ; i < MAX_FERT_LOGS ; i++ ) {
       if ( flq[i].RecDate != 0 ){ 
         url = "http://" + String(ghks.servername) + "/fert.asp?node="+String(ghks.lNodeAddress)+"&RecDate="+String(flq[i].RecDate)+"&FertMixID="+String(flq[i].FertMixID)+"&Valves="+String(flq[i].Valves)+"&Amount="+String(flq[i].Amount);
         http.begin(url);                      //HTTP
         httpCode = http.GET();
         if ( httpCode ==  HTTP_CODE_OK ){    // if processed reset the que entry
           Serial.println("Sent Fert Data");
           flq[i].RecDate = 0 ;
           j++;
         }
         http.end();
         if ( httpCode !=  HTTP_CODE_OK ){    // if processed reset the que entry
            Serial.println("FAIL Send Fert Data");
            break;
         }
       }
    }
//    Serial.println("End Send Fert Data");
  }else{
    Serial.println("No Network to Send Fert Data");
    j = -1 ; // error code for no network  
  }
  return(j);
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

