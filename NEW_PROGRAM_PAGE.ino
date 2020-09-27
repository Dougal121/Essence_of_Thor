uint8_t NewProgramStatus (uint8_t target){
int k ;  

  if ((( pn.ph[target].StartTime[0] == -1 ) && ( pn.ph[target].StartTime[1] == -1 ) && ( pn.ph[target].StartTime[2] == -1 )&& ( pn.ph[target].StartTime[3] == -1 )) || ((pn.ph[target].WDays & 0x80) == 0 ) || ((pn.ph[target].WDays & 0x7f) == 0) ) { // no start
    return(2); 
  }
  k = 0 ;
  for ( int i = 0 ; i < MAX_SHIFTS ; i++ ){
    if ( pn.sh[i].Program  == target ){  // if the shift belongs to the program
      if ( ( pn.sh[i].RunTime > 0 ) && ( pn.sh[i].ValveNo != 0 ) ) {
        k++ ; // count how many shifts are good to go
      }
    }
  }
  if ( k > 0 ){
    return(1);  
  }else{
    return(2);
  }
}

uint8_t NewShiftStatus (uint8_t target){
int k ;  
  k = pn.sh[target].Program  ; // program
  if ((( pn.ph[k].StartTime[0] == -1 ) && ( pn.ph[k].StartTime[1] == -1 ) && ( pn.ph[k].StartTime[2] == -1 ) && ( pn.ph[k].StartTime[3] == -1 )) || ((pn.ph[k].WDays & 0x80) == 0 ) || ((pn.ph[k].WDays & 0x7f) == 0) ) { // no start
    return(3);   // won't go because of program
  }
  if ( ( pn.sh[target].RunTime > 0 ) && ( pn.sh[target].ValveNo != 0 ) ) {
    return(1);  // ok green
  }else{
    return(2);  // no go red  
  }
}




void handleProgramNew(){
  int i , ii , iTmp , iX ,iRun ;
  uint8_t j , k , kk ;
  int iProgNum ;
  String message ;  
  String MyNum ;  
  String MyColor ;
  String MyColor2 ;
  String MyCheck ;
  String MyStart ;

  SerialOutParams();
  iProgNum = -1 ;
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("prgm");
    if (i != -1){  // look at the program number else it stays zero
      iProgNum = String(server.arg(j)).toInt() ;  
      iProgNum = constrain(iProgNum,-1,MAX_PROGRAM_HEADER);
/*      MyNum = String(iProgNum) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }*/        
    }

    for ( ii = 0 ; ii < MAX_SHIFTS ; ii++){  // handle all the valve control commands for any and all valves
      MyNum = String(ii) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }      
      i = String(server.argName(j)).indexOf("s" + MyNum + "rt");
      if (i != -1){  // 
        iRun = String(server.arg(j)).toInt()  ;
        iRun = constrain(iRun,0,MAX_MINUTES) ;
        pn.sh[ii].RunTime = iRun ;
      }        
      i = String(server.argName(j)).indexOf("s" + MyNum + "pr");
      if (i != -1){  // 
        pn.sh[ii].Program = String(server.arg(j)).toInt()  ;
        pn.sh[ii].Program = constrain(pn.sh[ii].Program,0,7);
        pn.sh[ii].ValveNo = 0 ;
      }        
      for ( k = 0 ; k < ghks.lMaxDisplayValve ; k++){  // handle all the valve control commands for any and all valves
        MyStart = String(k) ;
        if ( k < 10 ) {
          MyStart = "0" + MyStart ;
        }
        i = String(server.argName(j)).indexOf("s" + MyNum + "vp" + MyStart );
        if (i != -1){  // 
          pn.sh[ii].ValveNo |= ((uint16_t) 0x01 << k ) ;
        }                
      }  
    }


    for ( ii = 0 ; ii < MAX_PROGRAM_HEADER ; ii++){  // handle all the valve control commands for any and all valves
      MyNum = String(ii) ;
      if ( MyNum.length() == 1 ) {
        MyNum = "0" + MyNum ;
      }      
      i = String(server.argName(j)).indexOf("pt" + MyNum);
      if (i != -1){  // 
        String(server.arg(j)).toCharArray( pn.ph[ii].Description , MAX_DESCRIPTION ) ;
        pn.ph[ii].WDays = 0 ;
      }        
      for ( k = 0 ; k < MAX_STARTS ; k++){  // handle all the valve control commands for any and all valves
        i = String(server.argName(j)).indexOf("ps" + MyNum + "st" + String(k) );
        if (i != -1){  //          
          i = String(server.arg(j)).indexOf("X" );
          iX = String(server.arg(j)).indexOf("x" );
          if ((i != -1) || (iX != -1)){  //          
            pn.ph[ii].StartTime[k] = -1 ;
          }else{
            iTmp = String(server.arg(j)).toInt() ; 
            iTmp = constrain(iTmp,-1,2359);
            pn.ph[ii].StartTime[k] = iTmp ;            
          }
        }      
      }    
      for ( k = 0 ; k < 8 ; k++){  // handle all the valve control commands for any and all valves
        i = String(server.argName(j)).indexOf("pn" + MyNum + "dw" + String(k) );
        if (i != -1){  // 
          pn.ph[ii].WDays |= ( 0x01 << k ) ;
        }        
        
      }
    }

    
  }  // for j searching through parameters

  SendHTTPHeader();

//  server.sendContent(F("<br><center><b>Programing New</b><br>"));
  message = F("<b>Programs</b><br><table border=1 title='Programing New'>");
  message += F("<tr><th rowspan=2 colspan=2>Program</th>");
  j = iProgNum ;
  message += F("<th colspan=8>Days</th>");
  message += F("<th colspan=4>Starts HHMM</th><th rowspan=2>.</th>");
  message += F("</tr><tr>");
  for ( i = 0 ; i < 8 ; i++ ){
    message += "<th>" + String(dayarray[i]) + "</th>" ;
  }
  for ( i = 0 ; i < MAX_STARTS ; i++ ){
    message += "<th>" + String(i+1) + "</th>" ;
  }
  message +=F("</tr>");   
  server.sendContent(message) ; // End of Table Header
  
  for ( i = 0 ; i < MAX_PROGRAM_HEADER ; i++ ){
    switch (NewProgramStatus(i)){
      case 0:
        MyColor = ""   ;  
      break;  
      case 1:
        MyColor = F("bgcolor=green")   ;  
      break;
      default:
        MyColor = F("bgcolor=red")   ;  
      break;
    }
    message = "<tr><form method=post action='" + server.uri() + "'><input type='hidden' name='prgm' value='" + String(i) + "'><td "+ MyColor + "><b>";   
    MyNum = String(i) ;
    if ( MyNum.length() == 1 ){
      MyNum = "0" + MyNum ;
    }
    message += String(i+1)+ "</td><td><input type='text' name='pt" + MyNum + "' value='" + String(pn.ph[i].Description) + "' maxlength=8 size=5>" ; 
    message += F("</td>");   


    if ((( pn.ph[i].WDays & 0x7f ) != 0  ) && (( pn.ph[i].WDays & 0x80 ) == 0  )) { // selected days but not enabled
      MyColor2 =  F("bgcolor=red")  ;  
    }else{
      if (( pn.ph[i].StartTime[0] == -1 ) && ( pn.ph[i].StartTime[1] == -1 ) && ( pn.ph[i].StartTime[2] == -1 )&& ( pn.ph[i].StartTime[3] == -1 )) {
        MyColor2 =  F("bgcolor=red")  ;        
      }else{
        MyColor2 =  ""  ;  
      }
    }
    
    for (k = 0 ; k < 8 ; k++){      
      MyColor =  ""   ;  
      if ( ( pn.ph[i].WDays & (0x01 << k)) != 0 ){
        MyCheck = F("CHECKED")  ; 
        MyColor = MyColor2 ;      
      }else{
        MyCheck = "" ;      
        MyColor = "" ;      
      }
      message += "<td "+String(MyColor)+"><input type='checkbox' name='pn" + MyNum + "dw" + String(k)+"' "+String(MyCheck)+ "></td>";    
    }
    MyColor =  F("bgcolor=orange") ;  // check if no start times
    for (k = 0 ; k < MAX_STARTS ; k++){      
      if (( pn.ph[i].StartTime[k] >= 0 ) &&  (( pn.ph[i].WDays & 0x80 ) != 0  ) ){ // selected days but not enabled
        MyColor =  "" ;  
      }          
    }
    for (k = 0 ; k < MAX_STARTS ; k++){      
      if ( pn.ph[i].StartTime[k] >= 0 ){
        MyStart = String((pn.ph[i].StartTime[k])) ;
        iX = MyStart.length() ;
        if ( iX < 4 ) {
          for (iTmp = 0 ; iTmp < (4 - iX)  ; iTmp++  ){
            MyStart = "0" + MyStart ;
          }
        }
      }else{
        MyStart = String("X") ;        
      }
      message += "<td bgcolor=" + MyColor + "><input type='text' name='ps" + MyNum + "st" + String(k) + "' value='" + MyStart + "' maxlength=4 size=2></td>" ;
    }
    message += "<td><input type='submit' value='SET'></td></form></tr>" ;
    server.sendContent(message) ; // End of Table Header
//    j = iProgNum ;
  }
  server.sendContent( F("</table></form><br>"));   // End of combo with program header
    
  message = "<br><form method=post action=" + server.uri() + ">" ;
  message += F("Program Filter: <select name='prgm'>") ;
  message += "<option value='-1'>ALL" ;
  for ( j = 0 ; j < MAX_PROGRAM_HEADER ; j++ ){
    if ( j == iProgNum ){
      MyColor = F("SELECTED");
    }else{
      MyColor = "" ;        
    }
    message += "<option value='"+String(j)+ "' " + MyColor + " >" + String(j+1) + " - " + String(pn.ph[j].Description) ;
  }
  message += F("</select><input type='submit' value='SET'></form><br><br>");
  server.sendContent(message) ; // End of combo with program header

//  server.sendContent(String(iProgNum)+"<br><br>") ; // End of combo with program header
  message = F("<b>Program Shifts</b><br><table border=1 title='Program Shifts'>");
  message += F("<tr><th rowspan=2>Program</th><th rowspan=2>Shift</th>");
  message += "<th colspan="+String(ghks.lMaxDisplayValve)+">Valve</th><th rowspan=2>Runtime<br> (min)</th><th rowspan=2>.</th></tr><tr>";
  for ( k = 0 ; k < ghks.lMaxDisplayValve ; k++ ){
    message += "<th title='" + String(evalve[k].description) + "'>" + String(k+1)+ "</th>" ;  
  }  
  message += F("</tr>");
  server.sendContent(message) ;
  for ( i = 0 ; i < MAX_SHIFTS ; i++ ){
    MyNum = String(i) ;
    if ( MyNum.length() == 1 ){
      MyNum = "0" + MyNum ;
    }
    j = pn.sh[i].Program  ;
    if (( j == iProgNum ) || ( pn.sh[i].RunTime == 0 ) || ( iProgNum == -1 )) {
      switch (NewShiftStatus(i)){
        case 3:
          MyColor = F("bgcolor=orange")   ;  
        break;  
        case 1:
          MyColor = F("bgcolor=green")   ;  
        break;
        case 2:
          MyColor = F("bgcolor=red")   ;  
        break;
        default:
          MyColor = F("")   ;  
        break;
      }     
      message = "<tr><td " + MyColor + "><form method=post action=" + server.uri() + "><input type='hidden' name='prgm' value='" + String(iProgNum) + "'>" ;    

      message += "<select name='s" + MyNum + "pr'>" ;
      for ( j = 0 ; j < MAX_PROGRAM_HEADER ; j++ ){
        if ( j == pn.sh[i].Program ){
          MyColor = F("SELECTED");
        }else{
          MyColor = "" ;        
        }
        message += "<option value='"+String(j)+ "' " + MyColor + " >" + String(j+1) + " - " + String(pn.ph[j].Description) ;
      }
      message += F("</select></td>");
      server.sendContent(message) ;
      
      message = "<td>" + String(i+1) + "</td'>" ;
      server.sendContent(message) ;
      message = "" ;
      for ( k = 0 ; k < ghks.lMaxDisplayValve ; k++ ){
        if (( pn.sh[i].ValveNo & (0x01 << k )) != 0 ){
          MyCheck = F("CHECKED") ;    
          if ( pn.sh[i].RunTime == 0 ) {
            MyColor =  F("bgcolor=red") ;  // check if no start times
          }
        }else{
          MyColor = "" ;  // 
          MyCheck = "" ;      
        }
        MyStart = String(k) ;
        if ( k < 10 ) {
          MyStart = "0" + MyStart ;
        }
        message += "<td title='" + String(evalve[k].description) + "' " + String(MyColor) + "><input type='checkbox' name='s" + MyNum + "vp"+ MyStart +"' "+String(MyCheck)+ "></td>";    
      }
      message += "<td><input type='text' name='s" + MyNum + "rt' value='" + String((pn.sh[i].RunTime)) + "' maxlength=5 size=2></td>" ;  
      message += "<td><input type='submit' value='SET' title='" +String(i) + "'></td></form></tr>"; 
      server.sendContent(message) ;
    }  
  }                                                                            // bottom of max_shifts loop (i)
  server.sendContent(F("</table><br>"));    
  SendHTTPPageFooter();
}



