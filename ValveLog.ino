void DisplayRTCEEPROM() {
  uint8_t i[32] ;
  uint16_t ii[16] ;
  uint32_t iiii[8] ;
  int j , k ;
  int r  = 0  ;
  int b = 0 ;
  int d = 1 ;
  int iAddr = 0 ; 
  int  address = 0 ;
  int  iLen = 256 ;
  char buff[10];
  String message ;

  for (uint8_t j=0; j<server.args(); j++){
    k = String(server.argName(j)).indexOf("RADIX");
    if (k != -1){  // have a request to set the time zone
      r = String(server.arg(j)).toInt() ;
    }
    k = String(server.argName(j)).indexOf("ADDR");
    if (k != -1){  // have a request to set the time zone
      iAddr = String(server.arg(j)).toInt() ;
    }
    k = String(server.argName(j)).indexOf("LEN");
    if (k != -1){  // have a request to set the time zone
      iLen = String(server.arg(j)).toInt() ;
      if ( iLen < 32 ) iLen = 32 ;
      if ( iLen > 1024 ) iLen = 1024 ;
      iLen -=  (iLen % 32)  ;
      iLen += 32 ;
    }
    k = String(server.argName(j)).indexOf("BITS");
    if (k != -1){  // have a request to set the time zone
      b = String(server.arg(j)).toInt() ;
      switch(b){
        case 32:
          d = 4 ;
        break;
        case 16:
          d = 2 ;
        break;
        default:
          d = 1 ;
        break;
      }
    }
  }  
//  SerialOutParams();
  SendHTTPHeader();
  if ( hasRTC ){
    message = "<br><form method=post action=" + server.uri() + ">";
    message += "Base Address: <input type='text' name='ADDR' value='" + String(iAddr) + "' maxlength=4 size=4>" ;  
    message += "Len: <input type='text' name='LEN' value='" + String(iLen) + "' maxlength=4 size=4>" ;  
    message += "Radix: <select name=RADIX>" ;
    switch(r){
      case 2:
        message += F("<option value='2' SELECTED>Binary"); 
        message += F("<option value='8'>Octal"); 
        message += F("<option value='10'>Decimal"); 
        message += F("<option value='16'>Hexadecimal"); 
        message += F("<option value='1'>ASCII"); 
      break;
      case 8:
        message += F("<option value='2'>Binary"); 
        message += F("<option value='8' SELECTED>Octal"); 
        message += F("<option value='10'>Decimal"); 
        message += F("<option value='16'>Hexadecimal"); 
        message += F("<option value='1'>ASCII"); 
      break;
      case 10:
        message += F("<option value='2'>Binary"); 
        message += F("<option value='8'>Octal"); 
        message += F("<option value='10' SELECTED>Decimal"); 
        message += F("<option value='16'>Hexadecimal"); 
        message += F("<option value='1'>ASCII"); 
      break;
      case 1:
        message += F("<option value='2'>Binary"); 
        message += F("<option value='8'>Octal"); 
        message += F("<option value='10'>Decimal"); 
        message += F("<option value='16'>Hexadecimal"); 
        message += F("<option value='1' SELECTED>ASCII"); 
      break;
      default:
        message += F("<option value='2'>Binary"); 
        message += F("<option value='8'>Octal"); 
        message += F("<option value='10'>Decimal"); 
        message += F("<option value='16' SELECTED>Hexadecimal"); 
        message += F("<option value='1'>ASCII"); 
      break;
    }
    message += F("</select>");
    server.sendContent(message);
    message = F("Bits: <select name=BITS>");
    switch(b){
      case 32:
        message += F("<option value='8'>8 Bit - Byte"); 
        message += F("<option value='16'>16 Bit - Word"); 
        message += F("<option value='32' SELECTED>32 Bit - DWord"); 
      break;
      case 16:
        message += F("<option value='8'>8 Bit - Byte"); 
        message += F("<option value='16' SELECTED>16 Bit - Word"); 
        message += F("<option value='32'>32 Bit - DWord"); 
      break;
      default:
        message += F("<option value='8' SELECTED>8 Bit - Byte"); 
        message += F("<option value='16'>16 Bit - Word"); 
        message += F("<option value='32'>32 Bit - DWord"); 
      break;
    }
    message += F("</select>");
    server.sendContent(message);
    message = F("<input type='submit' value='SET'></form><br><table border=1 title='EEPROM Contents'><tr><th>.</th>");
   // table header
    for (k = 0; k < 32; k+=d) {
      message += "<th>"+String(k,HEX)+"</th>";
    }
    message += F("</tr>");
    server.sendContent(message);
    message = "" ;
    for (address = 0; address < iLen  ; address+=d ) {
      if (address % 32 == 0) {
        message += F("<tr>");  // start the line 
        message += "<td align=center><b>"+String(((address+iAddr) & 0xFFE0),HEX)+"</b></td>";
        switch(b){  // read all the data
          case 16:
            rtceeprom.eeprom_read(address+iAddr,(byte *)&ii,32);
          break;
          case 32:
            rtceeprom.eeprom_read(address+iAddr,(byte *)&iiii,32);
          break;
          default: // byte 8
            rtceeprom.eeprom_read(address+iAddr,(byte *)&i,32);
          }  
      }
      switch(b){
        case 16:
  //        rtceeprom.eeprom_read(address,(byte *)&ii,32);
          switch(r){
            case 8:
              message += "<td>"+String(ii[(address % 32)/2],OCT)+"</td>";
            break;
            case 10:
              message += "<td>"+String(ii[(address % 32)/2],DEC)+"</td>";
            break;
            case 2:
              message += "<td>"+String(ii[(address % 32)/2],BIN)+"</td>";
            break;
            default:
              message += "<td>"+String(ii[(address % 32)/2],HEX)+"</td>";
            break;
          }  
        break;
        case 32:
  //        rtceeprom.eeprom_read(address,(byte *)&iiii,4);
          switch(r){
            case 8:
              message += "<td>"+String(iiii[(address % 32)/4],OCT)+"</td>";
            break;
            case 10:
              message += "<td>"+String(iiii[(address % 32)/4],DEC)+"</td>";
            break;
            case 2:
              message += "<td>"+String(iiii[(address % 32)/4],BIN)+"</td>";
            break;
            default:
              message += "<td>"+String(iiii[(address % 32)/4],HEX)+"</td>";
            break;
          }  
        break;
        default: // byte 8
  //        rtceeprom.eeprom_read(address,(byte *)&i,32);
          switch(r){
            case 8:
              message += "<td>"+String(i[address % 32],OCT)+"</td>";
            break;
            case 10:
              message += "<td>"+String(i[address % 32],DEC)+"</td>";
            break;
            case 2:
              message += "<td>"+String(i[address % 32],BIN)+"</td>";
            break;
            case 1:   //ascii
              if ( isPrintable(char(i[address % 32])) ){
                message += "<td>"+String(char(i[address % 32]))+"</td>";
              }else{
                message += "<td>-</td>";              
              }
            break;
            default:
              message += "<td>"+String(i[address % 32],HEX)+"</td>";
            break;
          }  
        break;
      }
      if (address % 32 == 31) {
        message += F("</tr>");
        server.sendContent(message);
        message = "" ;
      }
      
    }
    message += F("</tr></table>");
    server.sendContent(message);
  }else{
    message += F("NO RTC / EEPROM for Valve Logs");
    server.sendContent(message);
  }
  SendHTTPPageFooter() ;
}

void ReadValveLogsFromEEPROM(void){
  long lStart = millis();
  if ( hasRTC ){
    rtceeprom.eeprom_read(0,(byte *)&rtcVTDates,sizeof(valve_totals_dates_t)) ; // read in each block
    rtceeprom.eeprom_read((2*sizeof(valve_totals_t)),(byte *)&rtcVT[0], MAX_VALVE * sizeof(valve_totals_t)) ; // read in each block +2 to leave room for the dates
    Serial.println("RTC EEPROM Read Time " + String((millis()-lStart)/1000,3) + " (s)");
    bValveLogDirty = false ; 
  }else{
    Serial.println("No RTC - No EEPROM to read");    
  }
  return;
}
void WriteValveLogsToEEPROM(void){
  long lStart = millis();
  if ( hasRTC && bValveLogDirty ){
    rtceeprom.eeprom_write(0,(byte *)&rtcVTDates,sizeof(valve_totals_dates_t)) ; // write blocks
    rtceeprom.eeprom_write((2*sizeof(valve_totals_t)),(byte *)&rtcVT[0],MAX_VALVE * sizeof(valve_totals_t)) ; // read in each block
    snprintf(buff, BUFF_MAX, "%02d/%02d/%04d", day(), month(), year() );     
    Serial.println(String(buff) + " RTC EEPROM Write Time " + String((millis()-lStart)/1000,3) + " (s)");
    bValveLogDirty = false ; 
  }else{
    if ( bValveLogDirty ){
      Serial.println("No RTC - No EEPROM to write");        
    }else{
      Serial.println("Log Clean Nothing worth writting to EEPROM");        
    }
  }
  return;  
}

void ZeroValveLogsMemory(int iReset){
  int j ;
  int i ;
  for ( j = 0 ; j < MAX_VALVE ; j++){
    if ((iReset & 0x0001) != 0 ){
      for ( i = 0 ; i < MAX_DAYS ; i++){
        rtcVT[j].Daily[i] = 0  ; 
      }
    }
    if ((iReset & 0x0002) != 0 ){
      for ( i = 0 ; i < MAX_WEEKS ; i++){
        rtcVT[j].Weekly[i] = 0 ;
      }
    }
    if ((iReset & 0x0004) != 0 ){
      for ( i = 0 ; i < MAX_MONTHS ; i++){
        rtcVT[j].Monthly[i] = 0 ;
      }
    }
  }
  if ((iReset & 0x0001) != 0 ){
    for ( i = 0 ; i < MAX_DAYS ; i++){
      rtcVTDates.Daily[i] = 0  ; 
    }
  }
  if ((iReset & 0x0002) != 0 ){
    for ( i = 0 ; i < MAX_WEEKS ; i++){
      rtcVTDates.Weekly[i] = 0 ;
    }
  }
  if ((iReset & 0x0004) != 0 ){
    for ( i = 0 ; i < MAX_MONTHS ; i++){
      rtcVTDates.Monthly[i] = 0 ;
    }
  } 
  return;  
}

void UpDateValveLogs(){
  int i , j = 0 ;
  bool bOn ;
  time_t myTime = now() ;

  if ( year(myTime) > 2019 ) { // reset all the totals assuming the clock is not bullshit
    j = weekday(myTime) ;
//    Serial.println("Valve Total Zero Update " + String(j));
    if (abs( myTime - rtcVTDates.Daily[j-1] ) > SECS_PER_DAY ) {
      rtcVTDates.Daily[j-1] = myTime ;
      for (i = 0 ; i < MAX_VALVE ; i++ ) {            // scan all the valves 
        rtcVT[i].Daily[j-1] = 0 ;                     // set all of that valve for the date to zero
      }
    }   
    j = WeekOfYear(myTime);
//    Serial.println("Valve Total Zero Update => " + String(j));

    if (abs( myTime - rtcVTDates.Weekly[j-1] ) > SECS_PER_WEEK ) {
      rtcVTDates.Weekly[j-1] = myTime ;
      for (i = 0 ; i < MAX_VALVE ; i++ ) {                // scan all the valves         
        rtcVT[i].Weekly[j-1] = 0 ;                        // set all of that valve for the date to zero
      }
    }          
  

    if ( year(rtcVTDates.Monthly[month(myTime)-1]) !=  year(myTime) ){
      rtcVTDates.Monthly[month(myTime)-1] = myTime ;
      for (i = 0 ; i < MAX_VALVE ; i++ ) {                // scan all the valves         
        rtcVT[i].Monthly[month(myTime)-1] = 0 ;
      }
    }
  }
//  Serial.println("Valve Totalizers Update ");

  for (i = 0 ; i < MAX_VALVE ; i++ ) {                // scan all the valves         
    if (( evalve[i].TypeMaster & 0x40 ) != 0x00  ){   // look if it has feedback
      if (( vvalve[i].iFB & 0x01 ) != 0x00  ){      // feedback status as if it is off or not
        bOn = true ; 
      }else{
        bOn = false ;                 
      }
    }else{
      if ( vvalve[i].bOnOff ){                        // valve that are on
        bOn = true ; 
      }else{
        bOn = false ;
      }
    }
    if (bOn) {
      j = WeekOfYear(myTime);
//      Serial.println("Week => " + String(j) );
      rtcVT[i].Daily[weekday(myTime)-1]++  ; 
      rtcVT[i].Weekly[j-1]++ ;
      rtcVT[i].Monthly[month(myTime)-1]++ ;    
      bValveLogDirty = true ;   
    }    
  }
}


int WeekOfYear(time_t myDate){
  int julian = DayOfYear(myDate);     // Jan 1 = 1, Jan 2 = 2, etc...
  int dow = dayOfWeek(myDate);        // Sun = 0, Mon = 1, etc...
  int dowJan1 = dayOfWeek(FirstDayOfYear(myDate)) ;  // find out first of year's day
  int weekNum = ((julian + 6) / 7);      // probably better.  CHECK THIS LINE. (See comments.)
  if (dow < dowJan1)                     // adjust for being after Saturday of week #1
    ++weekNum;
  if ( weekNum < 1 )  
    weekNum = 1 ;
  if ( weekNum > 53 )  
    weekNum = 53 ;
  return (weekNum);
}


time_t FirstDayOfYear(time_t myDate){
tmElements_t tm;
time_t myTime ;
    tm.Year = CalendarYrToTm(year(myDate)) ;  // dont leave home without the calendar year to tm function
//    Serial.println("Year ->" +String(tm.Year));
    tm.Month = 1 ;
    tm.Day = 1 ;
    tm.Hour = 0 ;
    tm.Minute = 0 ;
    tm.Second = 0 ;
    myTime = makeTime(tm) ;
//    snprintf(buff, BUFF_MAX, "First Day of year  %04d/%02d/%02d", year(myTime), month(myTime), day(myTime) );     
//    Serial.println(buff);
    return(myTime);
}
int DayOfYear(time_t myDate){
int iED1Jan ;  
int iEDNow = elapsedDays(myDate);
    iED1Jan = elapsedDays(FirstDayOfYear(myDate)) ;     
    return((iEDNow-iED1Jan)+1) ;
}
String MAH(int iMin){
  if (iMin == 0 ){
    return(String("-:-"));
  }else{
    if ( iMin > 1440 ){
      snprintf(buff, BUFF_MAX, "%d:%02d:%02d",(iMin/1440),((iMin/60)%24),(iMin%60));
    }else{
      snprintf(buff, BUFF_MAX, "%02d:%02d",((iMin/60)%24),(iMin%60));
    }
  }
  return(String(buff));
}

void DisplayValveLog() {
  uint8_t i ;
  uint16_t ii ;
  uint32_t iiii ;
  int j , k ;
  int r  = 0  ;
  int b = 0 ;
  int d = 1 ;
  int  address;
  bool bDownLoad = false ;
  char buff[10];
  String message ;
  String bgcolor ;
  String bgcolor2 ;
  time_t myTime ;
  time_t myTime2 ;
  time_t myBaseTime ;
  
  for (uint8_t j=0; j<server.args(); j++){
    i = String(server.argName(j)).indexOf("command");
    if (i != -1){  // 
      switch (String(server.arg(j)).toInt()){
        case 998:
          ReadValveLogsFromEEPROM();
          iValveLogTTG = (ghks.ValveLogOptions & 0x1f) * 10 + 20 ; // read data drop eemprom and over right memory
        break;
        case 997:
          WriteValveLogsToEEPROM();
          iValveLogTTG = (ghks.ValveLogOptions & 0x1f) * 10 + 20 ; // write data from memory and over write eeprom data
        break;
        case 999:
          ZeroValveLogsMemory(15);
        break;        
      }  
    }        
    k = String(server.argName(j)).indexOf("RADIX");
    if (k != -1){  // have a request to set the time zone
      r = String(server.arg(j)).toInt() ;
    }
    i = String(server.argName(j)).indexOf("download");
    if (i != -1){  // 
      bDownLoad = true ;
    } 
  }  
//  SerialOutParams();
  if ( bDownLoad ){ 
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/octet-stream; charset=utf-8", "");
    message = "Days" ;
    for ( i = 0 ; i < MAX_VALVE ; i++){
      message += "," + String(evalve[i].description) ; 
    }
    message += "\r\n" ;
    server.sendContent(message) ;
    for ( j = 0 ; j < MAX_DAYS ; j++){
      myTime = rtcVTDates.Daily[j] ;
      snprintf(buff, BUFF_MAX, "%02d/%02d/%04d", day(myTime), month(myTime), year(myTime) );     
      message += String(buff) ;
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "," + String(rtcVT[i].Daily[j])  ; 
      }
      message += "\r\n" ;
      server.sendContent(message) ;
      message = "" ;
    }
    message += "Weeks\r\n" ;

    for ( j = 0 ; j < MAX_WEEKS ; j++){
      myTime = rtcVTDates.Weekly[j] ;
      snprintf(buff, BUFF_MAX, "%02d/%02d/%04d", day(myTime), month(myTime), year(myTime) );     
      message += String(buff) ;
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "," + String(rtcVT[i].Weekly[j])  ; 
      }
      message += "\r\n" ;
      server.sendContent(message) ;
      message = "" ;
    }
    message += "Months\r\n" ;

    for ( j = 0 ; j < MAX_MONTHS ; j++){
      myTime = rtcVTDates.Monthly[j] ;
      snprintf(buff, BUFF_MAX, "%02d/%02d/%04d", day(myTime), month(myTime), year(myTime) );     
      message += String(buff) ;
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "," + String(rtcVT[i].Monthly[j])  ; 
      }
      message += "\r\n" ;
      server.sendContent(message) ;
      message = "" ;
    }   
    message += "\r\n\r\n" ;
    server.sendContent(message) ;
    
  }else{   // not downloading send it user as HTML
    SendHTTPHeader();
    message = F("<br><b>Valve Logging</b><br><a href='/?command=999'>Reset All Valve Log to Zero</a><br><table border=1 title='Valve Logging'>") ;
//    message += F("");
    
    message += "<tr><th>Day</th><th colspan="+String(MAX_VALVE)+">Daily Valve Totals</th></tr>" ; 
    if (bValveLogDirty){
      bgcolor = "bgcolor='yellow' title='Valve Log Cache is Dirty'" ;
    }else{
      bgcolor = "" ;    
    }
    message += "<tr><th "+ bgcolor +">.</th>" ; 
    for ( i = 0 ; i < MAX_VALVE ; i++){
      message += "<th align=center>" + String(i+1)+ "<br>" + String(evalve[i].description) + "</th>" ; 
    }
    message += "</tr>" ;
    myBaseTime = previousSunday(now()) ;
    for ( j = 0 ; j < MAX_DAYS ; j++){
      myTime = rtcVTDates.Daily[j] ;
      if ((month(myTime)==month(now())) && (day(myTime)==day(now())) ){
        bgcolor = "bgcolor='yellow'" ;
        bgcolor2 = "bgcolor='#ffff80'" ;
      }else{
        bgcolor = "" ;     
        bgcolor2 = "" ;
      }
      snprintf(buff, BUFF_MAX, "%02d/%02d/%04d", day(myTime), month(myTime), year(myTime) );     
      if (myTime == 0 ){
        message += "<tr><td " + bgcolor + " align=center>-<br>-</td>" ;
      }else{
        message += "<tr><td " + bgcolor + " align=center>"+DayString(dayOfWeek(myTime),false)+"<br>" + String(buff) + "</td>" ;
      }
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "<td " + bgcolor2 + " align=center>" + MAH(rtcVT[i].Daily[j]) + "</td>" ; 
      }
      message += "</tr>" ;   
      server.sendContent(message) ;
      message = "" ;      
    }
    
    message += "<tr><th>Week</th><th colspan="+String(MAX_VALVE)+">Weelky Valve Totals</th></tr>" ; 
    message += "<tr><th>.</th>" ; 
    for ( i = 0 ; i < MAX_VALVE ; i++){
      message += "<th align=center>" + String(i+1)+ "<br>" + String(evalve[i].description) + "</th>" ; 
    }
    message += "</tr>" ;
    
    myBaseTime = previousSunday(FirstDayOfYear(now())) ;
    for ( j = 0 ; j < MAX_WEEKS ; j++){
      myTime = myBaseTime + (j*SECS_PER_DAY*7) ;
      myTime2 = myTime + (SECS_PER_DAY*6) ;
      snprintf(buff, BUFF_MAX, "%02d/%02d/%04d - %02d/%02d/%04d", day(myTime), month(myTime), year(myTime), day(myTime2), month(myTime2), year(myTime2) );     
      if ( j == 0 ){
        k = WeekOfYear(myTime2); 
      }else{
        k = WeekOfYear(myTime);
      }
      if ((myTime<= now()) && (now()<(myTime2+SECS_PER_DAY))){
        bgcolor = "bgcolor='yellow'" ;
        bgcolor2 = "bgcolor='#ffff80'" ;
      }else{
        bgcolor = "" ;    
        bgcolor2 = "" ;
      }
      message += "<tr><td " + bgcolor + " title='"+String(buff)+"'>Week " + String(k) + "</td>" ;
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "<td " + bgcolor2 + " align=center>" + MAH(rtcVT[i].Weekly[j]) + "</td>" ; 
      }
      message += "</tr>" ;   
      server.sendContent(message) ;
      message = "" ;      
    }
  
    message += "<tr><th>Month</th><th colspan="+String(MAX_VALVE)+">Monthly Valve Totals</th></tr>" ; 
    message += "<tr><th>.</th>" ; 
    for ( i = 0 ; i < MAX_VALVE ; i++){
      message += "<th align=center>" + String(i+1)+ "<br>" + String(evalve[i].description) + "</th>" ; 
    }
    message += "</tr>" ;
    
    for ( j = 0 ; j < MAX_MONTHS ; j++){
      if (j==(month(now())-1)) {
        bgcolor = "bgcolor='yellow'";
        bgcolor2 = "bgcolor='#ffff80'" ;
      }else{
        bgcolor = "" ;     
        bgcolor2 = "" ;
      }
      message += "<tr><td " + bgcolor + " title='"+String(year(rtcVTDates.Monthly[j]))+"' align=center>"+MonthString(j+1,false)+"</td>" ;
      for ( i = 0 ; i < MAX_VALVE ; i++){
        message += "<td " + bgcolor2 + " align=center>" + MAH(rtcVT[i].Monthly[j]) + "</td>" ; 
      }
      message += "</tr>" ;
      server.sendContent(message) ; 
      message = "" ;
    }
    
    message = "<tr><th colspan=4><a href='/?command=998'>Read Valve Log from EEPROM</a></th><th colspan="+String(MAX_VALVE-7)+"><form method='GET' action=" + server.uri() + ".csv action='' enctype='multipart/form-data'><input type='hidden' name='download' value='doit'><input type='submit' value='Download'></form></th><th colspan=4><a href='/?command=997'>Write Valve Log to EEPROM</a><br></th></tr>" ;
    message += "</table><br>" ;
    server.sendContent(message) ; 
    SendHTTPPageFooter() ;
  }
}

String DayString(int i, bool bLong){
  switch(i){
    case 1:
      if ( bLong ){
        return("Sunday");
      }else{
        return("Sun");
      }
    break;
    case 2:
      if ( bLong ){
        return("Monday");
      }else{
        return("Mon");
      }
    break;
    case 3:
      if ( bLong ){
        return("Tuesday");
      }else{
        return("Tue");
      }
    break;
    case 4:
      if ( bLong ){
        return("Wednesday");
      }else{
        return("Wed");
      }
    break;
    case 5:
      if ( bLong ){
        return("Thursday");
      }else{
        return("Thr");
      }
    break;
    case 6:
      if ( bLong ){
        return("Friday");
      }else{
        return("Fri");
      }
    break;
    case 7:
      if ( bLong ){
        return("Saturday");
      }else{
        return("Sat");
      }
    break;
    default:
      if ( bLong ){
        return(" -- * -- ");
      }else{
        return("-*-");
      }
    break;
  }
}

String MonthString(int i, bool bLong){
  switch(i){
    case 1:
      if ( bLong ){
        return("January");
      }else{
        return("Jan");
      }
    break;
    case 2:
      if ( bLong ){
        return("February");
      }else{
        return("Feb");
      }
    break;
    case 3:
      if ( bLong ){
        return("March");
      }else{
        return("Mar");
      }
    break;
    case 4:
      if ( bLong ){
        return("April");
      }else{
        return("Apr");
      }
    break;
    case 5:
      if ( bLong ){
        return("May");
      }else{
        return("May");
      }
    break;
    case 6:
      if ( bLong ){
        return("June");
      }else{
        return("Jun");
      }
    break;
    case 7:
      if ( bLong ){
        return("July");
      }else{
        return("Jul");
      }
    break;
    case 8:
      if ( bLong ){
        return("August");
      }else{
        return("Aug");
      }
    break;
    case 9:
      if ( bLong ){
        return("Spetember");
      }else{
        return("Sep");
      }
    break;
    case 10:
      if ( bLong ){
        return("October");
      }else{
        return("Oct");
      }
    break;
    case 11:
      if ( bLong ){
        return("November");
      }else{
        return("Nov");
      }
    break;
    case 12:
      if ( bLong ){
        return("December");
      }else{
        return("Dec");
      }
    break;
    default:
      if ( bLong ){
        return(" -- * -- ");
      }else{
        return("-*-");
      }
    break;
  }
}



