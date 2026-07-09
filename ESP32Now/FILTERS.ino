int filter_scan (void){
int i ,k ;
  for ( k = 0 ; k < MAX_FILTER ; k++){
    if ( efilter[k].Enable != 0 ) {
      vfilter[k].bOnOff = false ;   //reset them all
    }
  }
  for ( i = 0 ; i < MAX_VALVE ; i++ ) {             // for each valve work out which turn filter units on
    if (( evalve[i].Filter != 0 )&&( vvalve[i].bOnOff == true )){
      for ( k = 0 ; k < 8  ; k++ ) {                // for each valve work out which turn filter units on
        if (( evalve[i].Filter & ( 0x01 << k )) != 0 ){
          vfilter[k].bOnOff = true ;  // set them on if they need be active
        }
      }
    }
  }  
  for ( k = i = 0 ; i < MAX_FILTER ; i++){
    if ( vfilter[i].bOnOff ) {
      k++ ;   //count them
    }
  }
  return(k);
}

int filter_sec (void){
uint8_t fbit , board ;
int i ,k ;
bool bPol ;
  for ( i = 0 ; i < MAX_FILTER ; i++){
    if ( efilter[i].Enable != 0 ) {
      board = ( efilter[i].BoardBit[k] & 0xf0 ) >> 4 ;
      fbit = ( efilter[i].BoardBit[k] & 0xf0 ) ;
      if ((bPol & ( 0x01 << k )) != 0 ) {
        bPol = false ;
      }else{
        bPol = true ;
      }
      if ( vfilter[i].bOnOff ) {
        k = vfilter[i].NextCan ;
        if (( k > 7 )||( k < 0 )){
          k = 0 ;
        }
        vfilter[i].lTTG-- ;
        if ( vfilter[i].lTTG == 0 ) {
          if ( vfilter[i].bFlush ) {
//            IOEXP[board].write(fbit , bPol  );   // Off active HIGH
            ActivateOutput(board , fbit , bPol , 0 ) ;
            
            vfilter[i].lTTG = efilter[i].WashDelay ;
            vfilter[i].bFlush = false ;
            if ( ++vfilter[i].NextCan > efilter[i].Canisters ) {
              vfilter[i].NextCan = 0 ;
            }
          }else{
//            IOEXP[board].write(fbit , !bPol );   // On active LOW
            ActivateOutput(board , fbit , !bPol , 0 ) ;
            vfilter[i].lTTG = efilter[i].WashTime ;
            vfilter[i].bFlush = true ;
          }
        }
      }else{
//        IOEXP[board].write(fbit , bPol  );   // Off active HIGH    
        ActivateOutput(board , fbit , bPol , 0 ) ;
      }
    }
  }
  return(0);
}






void handleFilt(){
  int i , ii , iTmp , iX ;
  uint8_t j , k , kk ;
  String message ;  
//  String MyNum ;  
//  String MyColor ;
  String MyColor2 ;
  String MyCheck , MyColor , MyNum , MyCheck2 ;

  SerialOutParams();
  
  for (uint8_t j=0; j<server.args(); j++){
    for ( ii = 0 ; ii < MAX_FILTER ; ii++){  // handle all the filter control arguments
      i = String(server.argName(j)).indexOf("fx" + String(ii));  // this is a parser cheat
      if (i != -1){  // 
        efilter[ii].Enable = 0 ; // wipe all the data - "shoot them all and let god sort them out"
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "d" );
      if (i != -1){  // wash delay
        efilter[ii].WashDelay = String(server.arg(j)).toInt() ;
        if ( efilter[ii].WashDelay < 0 ) {
          efilter[ii].WashDelay = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "t" );
      if (i != -1){  // wash time
        efilter[ii].WashTime = String(server.arg(j)).toInt() ;
        if ( efilter[ii].WashTime < 0 ) {
          efilter[ii].WashTime = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f" + String(ii) + "n" );
      if (i != -1){  // canisters
        efilter[ii].Canisters = String(server.arg(j)).toInt() ;
        if ( efilter[ii].Canisters > 8 ){
          efilter[ii].Canisters = 8 ;
        }
        if ( efilter[ii].Canisters < 0 ){
          efilter[ii].Canisters = 0 ;
        }
      }        
      i = String(server.argName(j)).indexOf("f"+String(ii)+"p" );
      if (i != -1){  // polarity
        efilter[ii].bPol = 0xff & String(server.arg(j)).toInt()  ;
        if ( efilter[ii].bPol > 1 ) {
          efilter[ii].bPol = 1 ;
        }
        if ( efilter[ii].bPol < -1 ) {
          efilter[ii].bPol = -1 ;
        }
      }      
      i = String(server.argName(j)).indexOf("fe" + String(ii) );
      if (i != -1){  // polarity
        if ( String(server.arg(j)).length() == 2 ){
          efilter[ii].Enable = 1 ;            
        }
      }      
      for ( kk = 0 ; kk < MAX_CANISTERS ; kk++ ) {
        i = String(server.argName(j)).indexOf("f"+String(ii)+"a"+String(kk));
        if (i != -1){  // for each output
          efilter[ii].BoardBit[kk] = ( efilter[ii].BoardBit[kk] & 0x0f ) | (( 0x0f & String(server.arg(j)).toInt()) << 4 ) ;
        }  
        i = String(server.argName(j)).indexOf("f"+String(ii)+"b"+String(kk));
        if (i != -1){  // for each output
          efilter[ii].BoardBit[kk] = ( efilter[ii].BoardBit[kk] & 0xf0 ) | ( 0x0f & String(server.arg(j)).toInt()) ;
        }  
      }
            
    }

  }
  
  SendHTTPHeader();
  server.sendContent(F("<br><b>Filter Setup</b>"));
  message = F("<table border=1 title='Filter Setup'>") ;
  message += F("<tr><th rowspan=2>Bank</th><th rowspan=2>Enable</th><th rowspan=2>Wash Delay</th><th rowspan=2>Wash Time</th><th rowspan=2>Canisters</th>") ; 
  message += F("<th rowspan=2>Polarity</th><th colspan=17>Canister Control Relays</th></tr>") ;
  message += F("<tr>");
  for ( j = 0 ; ( j < MAX_CANISTERS )  ; j++){
    message += F("<th>Board</th><th>Bit</th>");
  }
  message += F("</tr>") ; 
  server.sendContent(message) ;
  message = "" ;
  for ( i = 0 ; i < MAX_FILTER ; i++){  // to the number of filter prgram in the chip
    if ( efilter[i].Enable == 0 ) {
      MyColor = F("bgcolor=red") ;
      MyCheck = F("")  ;    
    }else{
      MyColor = F("bgcolor=green") ;
      MyCheck = F("CHECKED")  ;    
    }
    message += "<tr><form method=post action=" + server.uri() + "><input type='hidden' name='fx"+String(i)+"' value='"+String(i)+"'><td align=center " + String(MyColor) + ">" + String(i+1) + "</td>" ;
    message += "<td align=center><input type='checkbox' name='fe"+String(i)+"' "+String(MyCheck)+"></td>" ;
//      Serial.println(String(efilter[i].WashDelay));
    message += "<td><input type='text' name='f"+String(i)+"d' value='" + String(efilter[i].WashDelay) + "' maxlength=6 size=6></td>" ; 
    message += "<td><input type='text' name='f"+String(i)+"t' value='" + String(efilter[i].WashTime) + "' maxlength=4 size=4></td>";         
    message += "<td><input type='text' name='f"+String(i)+"n' value='" + String(efilter[i].Canisters) + "' maxlength=1 size=4></td>";
    message += "<td><input type='text' name='f"+String(i)+"p' value='" + String(efilter[i].bPol) + "' maxlength=3 size=3></td>";
    server.sendContent(message) ;
    message = "" ;
    for ( j = 0 ; ( j < MAX_CANISTERS )  ; j++){
      message += "<td><input type='text' name='f" + String(i) + "a" + String(j) + "' value='" + String((efilter[i].BoardBit[j] & 0xf0)>>4) + "' maxlength=2 size=2></td>" ;
      message += "<td><input type='text' name='f" + String(i) + "b" + String(j) + "' value='" + String((efilter[i].BoardBit[j] & 0x0f)) + "' maxlength=2 size=2></td>";         
    }  
    message += F("<td><input type='submit' value='SET'></td></form></tr>") ;
    server.sendContent(message) ;
    message = "" ;
  }
  message = F("</table>") ;
  server.sendContent(message) ;

  SendHTTPPageFooter();  
}

