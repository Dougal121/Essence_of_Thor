 /*

 0- 1: NTP version number and mode
 2- 3: Peer clock stratum
 4- 7: Peer poll interval
 8-11: Peer clock precision
12-19: Root delay
20-27: Root dispersion
28-35: Reference ID
36-43: Reference timestamp
44-51: Origin timestamp
52-59: Receive timestamp
60-67: Transmit timestamp
68-75: Key Identifier (optional)
76-79: CRC/MAC field

In this structure, the first 4 bytes (bytes 40-43) represent the seconds part of the reference timestamp, and the last 4 bytes represent the fractional part of the reference timestamp.

*/

unsigned long processNTPpacket(void){
    int iYear = year() ;
    ntpudp.read(packetBuffer, NTP_PACKET_SIZE);                                         // the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long. First, esxtract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);                  // combine the four bytes (two words) into a long integer
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;                             // this is NTP time (seconds since Jan 1 1900):
    const unsigned long seventyYears = 2208988800UL;                                    // now convert NTP time into everyday time:     Unix time starts on Jan 1 1970. In seconds, that's 2208988800:   
    unsigned long epoch = secsSince1900 - seventyYears + long(SECS_PER_HOUR * ghks.fTimeZone );   // subtract seventy years:
    unsigned long timediff = epoch - now();
    timediff = abs(timediff) ;  // dont use to set RTC if more than 30 min out


    if (((year(epoch) > 2019 ) && (( timediff < 1800 ) || ( iYear == 1970 ))) || bManSet ){
        setTime((time_t)epoch);                                                             // update the clock
        snprintf(buff, BUFF_MAX, "RTC %d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
        tc.sec = second();     
        tc.min = minute();     
        tc.hour = hour();   
        tc.wday = dayOfWeek((time_t)epoch);            
        tc.mday = day();  
        tc.mon = month();   
        tc.year = year();       
        if ( hasRTC ){
          DS3231_set(tc);                       // set the RTC as well
        }
        bManSet = false ;
    }else{
      Serial.println(F("*** Time NOT set to RTC check times ***"));  
      snprintf(buff, BUFF_MAX, "NTP  %d/%02d/%02d %02d:%02d:%02d", year(epoch), month(epoch), day(epoch) , hour(epoch), minute(epoch), second(epoch));          
      Serial.println( String(buff)) ;       
      snprintf(buff, BUFF_MAX, "RTC %d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
    }
    Serial.println( String(buff)) ;       
    
    Serial.print(F("Unix time = "));
    Serial.println(epoch);                                                              // print Unix time:
    rtc_hour = hour() ;                                                                 // stop this happining again now it's updated
}

unsigned long sendNTPpacket(char* address){
byte packetBuffer[ NTP_PACKET_SIZE];     //buffer to hold incoming and outgoing packets  
IPAddress ntpIP ;
IPAddress MyIP;

  MyIP =  WiFi.localIP() ;                  
  if (WiFi.isConnected() && (MyIP[0] != 0 )&& (MyIP[4] != 0 ))  {
    Serial.println("sending NTP packet...");
    memset(packetBuffer, 0, NTP_PACKET_SIZE);    // set all bytes in the buffer to 0
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
  
    WiFi.hostByName(address, ntpIP);
    snprintf(buff, BUFF_MAX, "%u.%u.%u.%u\0", ntpIP[0], ntpIP[1], ntpIP[2], ntpIP[3]);
    Serial.print("NTP IP address: "+String(address)+" ->  ");
    Serial.println(buff);
  
    if  (( ntpIP[0] == 0 ) &&  ( ntpIP[1] == 0 ) && ( ntpIP[2] == 0 ) && ( ntpIP[3] == 0 )){
      Serial.println("No DNS - no point sending NTP to 0.0.0.0 ");      
    }else{
      // all NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      ntpudp.beginPacket(buff, 123); //NTP requests are to port 123
      ntpudp.write(packetBuffer, NTP_PACKET_SIZE);
      ntpudp.endPacket();
    }
  }else{
    Serial.println("No WiFi - no point trying to send NTP packet...");
  }  
}


// Calculates a 32-bit CRC using the polynomial 0x04C11DB7
// and returns the CRC value
unsigned long calculateNTPCRC32(unsigned char* data, int length)
{
  unsigned long crc = 0x00000000;
  const unsigned long polynomial = 0x04C11DB7;

  for (int i = 0; i < length; i++)
  {
    crc ^= (unsigned long)data[i] << 24; // XOR the next byte of data with the CRC and shift it to the left
    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x80000000) // if the MSB is set
      {
        crc = (crc << 1) ^ polynomial; // shift the CRC left by one bit and XOR with the polynomial
      }
      else
      {
        crc <<= 1; // shift the CRC left by one bit
      }
    }
  }
  
  return crc; // return the final CRC value
}

// Calculates a 32-bit CRC using the polynomial 0xEDB88320
// and returns the CRC value
unsigned long calculateCRC32(unsigned char* data, int length)
{
  unsigned long crc = 0xFFFFFFFF;
  
  for (int i = 0; i < length; i++)
  {
    crc ^= data[i]; // XOR the next byte of data with the CRC
    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x00000001) // if the LSB is set
      {
        crc = (crc >> 1) ^ 0xEDB88320UL; // shift the CRC right by one bit and XOR with the polynomial
      }
      else
      {
        crc >>= 1; // shift the CRC right by one bit
      }
    }
  }
  
  return ~crc; // complement the final CRC value before returning it
}

