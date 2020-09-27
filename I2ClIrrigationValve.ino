/*
    This project is a distillation of "Project Thor"  which started in 2008
    So in that vein you can expect it's lost a little body but the conceptual "active constituents" are present in a concerntrated form.

    The code is designed to work with/in multiple configurations and hardware based around the ESP8266.

    It was originally fabricated at code works of Plummer Software in Coomealla, Australia circa 2019
    Newer copies can be found at https://github.com/Dougal121/Essence_of_Thor

    The idea is to provide low cost high tech fertigation/irrigation control for real world farming which is scalable from small to medium size farms (0.1 to 100 Ha)
    
*/
#include <ESP8266WiFi.h>
#include <WiFiUDP.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>
//#include <DNSServer.h>
#include <TimeLib.h>
#include <Wire.h>
//#include <SPI.h>
#include <EEPROM.h>
#include <stdio.h>
#include "PCF8574DJP.h"
#include "Adafruit_MCP23017.h"
#include "SSD1306.h"
//#include "SH1106.h"
#include "SH1106Wire.h"
#include "ds3231.h"


#define BUFF_MAX 32
/* Display settings */
#define minRow       0              /* default =   0 */
#define maxRow     127              /* default = 127 */
#define minLine      0              /* default =   0 */
#define maxLine     63              /* default =  63 */

#define LineText     0
#define Line        12

//                 0x11223344
#define MYVER 0x12345679     // change this if you change the structures that hold data that way it will force a "backinthebox" to get safe and sane values from eeprom

const int MAX_EEPROM = 2000 ;
//const byte SETPMODE_PIN = D8 ; 
//const byte FLASH_BTN = D3 ;    // GPIO 0 = FLASH BUTTON 
//const byte SCOPE_PIN = D7 ;
const byte MAX_WIFI_TRIES = 45 ;
const byte PROG_BASE = 192 ;   // where the irrigation valve setup and program information starts in eeprom
const byte MAX_VALVE =  16 ;   // these two easily changed just watch the memory 
const byte MAX_PROGRAM = 4 ;   // valves * program is the biggest memory number ... can do 32 x 4 OK but need to allocate more EEPROM
const byte MAX_FERT = 6 ;      // fertigation units MAXIUM of 8 <- DEAL & CODE BREAKER
const byte MAX_FILTER = 1 ;    // can have more but whats the point ? again max of 8
const byte MAX_CANISTERS = 5;  // maxium of 16 ?? limmit on what the editing page will process
const byte MAX_DESCRIPTION = 8 ;
const byte MAX_PROGRAM_HEADER = 8 ;  // these 2 are the new programmin array method
const byte MAX_SHIFTS = 64 ;
const byte MAX_STARTS = 4 ;
const int  MAX_MINUTES = 10080 ; // maximum minutes in a 7 day cycle
const byte MAX_BOARDS = 16 ;
const byte MAX_MCP23017 = 8 ;    // maximum number of these expanders on a system
PCF8574 IOEXP[16]{0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f} ;
//SSD1306 display(0x3c, 5, 4);   // GPIO 5 = D1, GPIO 4 = D2   - onboard display 0.96" 
SH1106Wire display(0x3c, 4, 5);   // arse about ??? GPIO 5 = D1, GPIO 4 = D2  -- external ones 1.3"


/*
   Basic pin reading and pullup test for the MCP23017 I/O expander
   Connect pin #12 of the expander to Analog 5 (i2c clock)
   Connect pin #13 of the expander to Analog 4 (i2c data)
   Connect pins #15, 16 and 17 of the expander to ground (address selection)
   Connect pin #9 of the expander to 5V (power)
   Connect pin #10 of the expander to ground (common ground)
   Connect pin #18 through a ~10kohm resistor to 5V (reset pin, active low)
   Output #0 is on pin 21 so connect an LED or whatever from that to ground
*/  
Adafruit_MCP23017 mcp[MAX_MCP23017] ;

typedef struct __attribute__((__packed__)) {             // permanent record
  int16_t TypeMaster;        // NoFert 0-1 normal-nofert Type 0-1 normal=master 0-1 nofeedbak-feedback 0-1 and Master valve ID 0-63   2 bits + 6 bits
  uint8_t OnCoilBoardBit;    // 16 boards + 16 Outputs   4 + 4 bits
  uint8_t OffCoilBoardBit;   // 16 boards + 16 Outputs  4 + 4 bits
  uint8_t OnOffPolPulse ;    // 4 bits on 4 bits off pulse and polarity ie 1 + 3 x 50 ms + 50ms base 
  uint8_t FeedbackBoardBit;  // 16 Boards and 16 Outputs 4 + 4 bits
  uint8_t Fertigate ;        // 8 bits for tanks 0 -> 7
  uint8_t Filter;            // 8 bits for filters 0 -> 7  ie nomally 0 off 1 on
  uint8_t bEnable ;          // Enable 
  uint8_t Node ;             // send to node if non zero on node
  uint8_t Valve ;            // send to valve 
  float   Flowrate ;         // L/s  flow ??
  int16_t lDTTG  ;           // default minutes 
  int16_t lTTC ;             // time to clear the line in seconds from the last cycle 
  char    description[MAX_DESCRIPTION] ;   //
} valve_t ;                  // 27 bytes    (16x = 432 )

typedef struct __attribute__((__packed__)) {            // volitile stuff
  bool    bOnOff ;          // on off status
  uint8_t iFB    ;          // feedback status 
  int16_t lATTG  ;          // programed (automatic) minutes
  int16_t lTTG   ;          // minutes  
  int16_t lTTC ;            // time to clear the line after pump has last activated (seconds -- counts down)    
  bool    bNetOnOff ;       // network on off status
} valve__t ;                // 8 bytes   

typedef struct __attribute__((__packed__)) {            // permanent record
  uint8_t AType ;           // top 4 bits 1 normal pump , 2 master pump , 3 chemical solenoid , 4 wash solenoid  --- bottom 3 bits master pump record if solenoide
  uint8_t DaysEnable ;      // seven days plus overall enable bit  bit 0 sun -->> bit 6 sat and bit 7 master enable
  float   CurrentQty ;      // how much is left to be pumped
  float   MaxQty ;          // Tank at full
  int16_t OnTime ;          // (seconds) this get multipled by the flow rate
  int16_t OffTime ;         // (seconds) this is used as is 
  float   BaseFlow ;        // this is a flow rate base divisor - I think ????
  float   PumpRate ;        // (l/s) pump rate obviously
  uint8_t BoardBit ;        // 16 boards + 16 Bits  --- what outpuut this is attached to.
  char    description[MAX_DESCRIPTION] ;  //
} fertigation_t ;           // 31 bytes   (8x = 248)

typedef struct __attribute__((__packed__)) {            // volitile component 
  bool    bRun ;            // pump in run mode 
  bool    bEnable ;         // enable on / off
  bool    bOnOff ;          // on off status
  int16_t lTTG ;
  float   Flowrate ;
} fertigation__t ;          // 24 bytes

typedef struct __attribute__((__packed__)) {                        // Permanent record
  int16_t WashDelay ;                   // wash delay time in seconds   
  int16_t WashTime ;                    // wash time in seconds   
  uint8_t Enable ;                      // Enable filter program 
  uint8_t Canisters ;                   // number of filter can in the bank
  uint8_t bPol ;                        // Polarity of the 8 filter cans flush solenoids  
  uint8_t BoardBit[MAX_CANISTERS] ;     // 16 boards + 16 Bits
} filter_t ;                            // 15 bytes

typedef struct __attribute__((__packed__)) {            // volitile component
  int16_t lTTG ;
  uint8_t NextCan ;         // Next canister 
  bool    bOnOff ;          // on off status
  bool    bFlush ;          // Wash-true Wait-false 
} filter__t ;               //  bytes

typedef struct __attribute__((__packed__)) {         // permanent record
  uint16_t  starttime ;    // start time  8 bit + 8 bit   HH:MM
  uint16_t  runtime ;      // runtime in minutes
  uint8_t   wdays ;        // seven days plus overall enable bit 
} program_t ;            // 5 bytes

typedef struct __attribute__((__packed__)) {
  program_t p[MAX_PROGRAM];
} program_a ;                                  // 5 * 4 * 16 = 320

typedef struct __attribute__((__packed__)) {                        // Permanent record
  uint8_t Address;
  uint8_t Type;
  uint8_t Translate;  
} board_t ;

typedef struct __attribute__((__packed__)) {                        // Permanent record
  uint8_t IOPin[16];
} local_t ;

typedef struct __attribute__((__packed__)) {           //  new programming system closer to what THOR is / does
  char      Description[MAX_DESCRIPTION] ;             // program numer is explicit as the array index
  uint8_t   WDays ;                                    // enable days + master enable
  int16_t   StartTime[MAX_STARTS] ;                    // start time  8 bit + 8 bit   HH:MM  
} program_header_t ;                                   // 17 bytes ?  by say 8 ==> 136 bytes

typedef struct __attribute__((__packed__)) {           // 
  uint8_t   Program ;                                  // 5 bit Shift No - 3 Bits Program No number (8 programs 32 shifts in each)
  uint16_t  ValveNo ;                                  // one bit for every valve  ?
  int16_t   RunTime ;                                  // runtime in minutes
} shift_new_t ;                                        // 4 bytes by say 32 or 64 ???  5 if uint16_t --- 7 if uint32_t

typedef struct __attribute__((__packed__)) {
  program_header_t ph[MAX_PROGRAM_HEADER];             // 136 -- only 8 programs
  shift_new_t      sh[MAX_SHIFTS] ;                    // 256 -- 64 valve shifts (same as before sort of)  320  or  448
} program_new_t ;                                      // 292 whole new program structure so can be wriiten to eeprom in one go  456 or  584

program_new_t     pn;

board_t           eboard[MAX_BOARDS] ;   // board list - address and type
valve_t           evalve[MAX_VALVE] ;    // eprom backed up stuff 
program_a         vp[MAX_VALVE];
fertigation_t     efert[MAX_FERT] ;
filter_t          efilter[MAX_FILTER] ;  // eprom backed up stuff 
valve__t          vvalve[MAX_VALVE] ;    // volitile valve stuff
fertigation__t    vfert[MAX_FERT] ;      // volitile fertigation stuff
filter__t         vfilter[MAX_FILTER] ;  // volitile valve stuff
local_t           elocal ;


typedef struct __attribute__((__packed__)) {     // eeprom stuff
  unsigned int localPort = 2390;          // 2 local port to listen for NTP UDP packets
  unsigned int localPortCtrl = 8666;      // 4 local port to listen for Control UDP packets
  unsigned int RemotePortCtrl = 8664;     // 6 local port to listen for Control UDP packets
  long lProgMethod ;                      // 10
  long lPulseTime ;                       // 14
  long lMaxDisplayValve ;                 // 18
  long lNodeAddress ;                     // 22 
  float fTimeZone ;                       // 26 
  char RCIP[16] ;                         // (192,168,2,255)  30
  char NodeName[16] ;                     // 46 
  char nssid[16] ;                        // 62  
  char npassword[16] ;                    // 78
  time_t AutoOff_t ;                      // 82     auto off until time > this date   
  uint8_t lDisplayOptions  ;              // 83 
  uint8_t lNetworkOptions  ;              // 84 
  uint8_t lSpare1  ;                      // 85 
  uint8_t lSpare2  ;                      // 86 
  char timeServer[24] ;                   // 110   = {"au.pool.ntp.org\0"}
  char cpassword[16] ;                    // 126
  long lVersion  ;                        // 130
  IPAddress IPStatic ;                    // (192,168,0,123)   
  IPAddress IPGateway ;                   // (192,168,0,1)    
  IPAddress IPMask ;                      // (255,255,255,0)   
  IPAddress IPDNS ;                       // (192,168,0,15)   
} general_housekeeping_stuff_t ;          // computer says it's 136 not 130 ??? is my maths crap ????

general_housekeeping_stuff_t ghks ;

typedef struct __attribute__((__packed__)) {
  int16_t lATTG  ;            // programed (automatic) minutes
  int16_t lTTG   ;            // minutes  
  int16_t lTTC ;              // time to clear the line after pump has last activated (seconds -- counts down)    
  uint8_t Fertigate ;         // 8 bits for tanks 0 -> 7
  uint8_t ValveNo ;
  int16_t Node ;
  float   Flowrate ;          // L/s  flow ??
} cnc_v_t ; 

typedef struct __attribute__((__packed__)) {
  int16_t      cmd ;
  int16_t      snode ;
  uint8_t      valves ;          // number to follow
  cnc_v_t      cv[MAX_VALVE] ;   // the most that we can have 
} cnc_t ; 


char cssid[32] = {"Configure_XXXXXXXX\0"} ;
char *host = "Control_00000000\0";                // overwrite these later with correct chip ID
char Toleo[10] = {"Ver 2.5\0"}  ;
char dayarray[8] = {'S','M','T','W','T','F','S','E'} ;
uint8_t mcp_relay_tranlate[16]={7,15,6,14,5,13,4,12,8,0,9,1,10,2,11,3} ;

char buff[BUFF_MAX]; 

IPAddress MyIP ;
IPAddress MyIPC  ;

const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   //buffer to hold incoming and outgoing packets

byte rtc_sec ;
byte rtc_min ;
byte rtc_hour ;
byte rtc_fert_hour ;
float rtc_temp ;
long lScanCtr = 0 ;
long lScanLast = 0 ;
bool bConfig = false ;
bool hasRTC = false ;
int  efertAddress ;
long lRebootCode = 0 ;
uint8_t rtc_status ;
struct ts tc;  
bool bPrevConnectionStatus = false;
unsigned long lTimeNext = 0 ;           // next network retry

int iTestBoard = 0 ;
int iTestMode = -1 ; 
int iTestTime = 3 ;
int iTestTimer = 0 ;
int iTestInc = 1 ; 
int iTestCoil = 0 ;
int bSaveReq = 0 ;
int iUploadPos = 0 ;
bool bDoTimeUpdate = false ;
long  MyCheckSum ;
long  MyTestSum ;
long lTimePrev ;
long lTimePrev2 ;
long lMinUpTime = 0 ;

WiFiUDP ntpudp;
WiFiUDP ctrludp;

ESP8266WebServer server(80);
ESP8266WebServer OTAWebServer(81);
ESP8266HTTPUpdateServer OTAWebUpdater;
//DNSServer dnsServer;

//void BackInTheBoxMemory(void);




/*
void FloatToModbusWords(float src_value , uint16_t * dest_lo , uint16_t * dest_hi ) {
  uint16_t tempdata[2] ;
  float *tf ;
  tf = (float * )&tempdata[0]  ;
  *tf = src_value ;
  *dest_lo = tempdata[1] ;
  *dest_hi = tempdata[0] ;
}

float FloatFromModbusWords( uint16_t dest_lo , uint16_t dest_hi ) {
  uint16_t tempdata[2] ;
  float *tf ;
  tf = (float * )&tempdata[0]  ;
  tempdata[1] = dest_lo ;
  tempdata[0] = dest_hi  ;
  return (*tf) ;
}

int NumberOK (float target) {
  int tmp = 0 ;
  tmp = isnan(target);
  if ( tmp != 1 ) {
    tmp = isinf(target);
  }
  return (tmp);
}
*/



//  ############################################## SETUP  #################################################################  SETUP   #############################
void setup() {
int i , k , j = 0; 
  
  lRebootCode = random(1,+2147483640) ;  // want to change it straight away

  ghks.lMaxDisplayValve = MAX_VALVE ;
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println("");          // new line after the startup burst
 
  pinMode(BUILTIN_LED,OUTPUT);  //  D4 builtin LED
//  pinMode(SETPMODE_PIN,INPUT_PULLUP);
  
  pinMode(12,OUTPUT);  // D6      ok setup 4 the outputs bar the i2c to be outputs
  pinMode(13,OUTPUT);  // D7 
  pinMode(14,OUTPUT);  // D5
  pinMode(15,OUTPUT);  // D8

  EEPROM.begin(MAX_EEPROM);
  LoadParamsFromEEPROM(true);  

  display.init();
  if (( ghks.lDisplayOptions & 0x01 ) != 0 ) {  // if bit one on then flip the display
    display.flipScreenVertically();
  }

  /* show start screen */
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);  
  display.setFont(ArialMT_Plain_16);
  display.drawString(63, 0, "Irrigation");
  display.drawString(63, 16, "Controler");
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);  
  display.drawString(0, 40, "Copyright (c) 2020");
  display.drawString(0, 50, "Dougal Plummer");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);  
  display.drawString(127, 50, String(Toleo));
  display.display();
    
  for (i = 0 ; i < MAX_VALVE ; i++ ) {   // setup the defaults in the 3 volitile structs that support data comin back from eeprom
    vvalve[i].lATTG = 0 ;
    vvalve[i].lTTG = 0 ;
    vvalve[i].bOnOff = false ;
  }

  for (i = 0 ; i < MAX_FERT ; i++ ) {
    vfert[i].bRun = false ;
    vfert[i].bEnable = false ;
    vfert[i].lTTG = 0 ;
    vfert[i].bOnOff = false ;
    vfert[i].Flowrate = 0.0 ;  
  }

  for (i = 0 ; i < MAX_FILTER ; i++ ) {
    vfilter[i].NextCan = 0 ;
    vfilter[i].bOnOff = false ;
    vfilter[i].bFlush = false ;
    vfilter[i].lTTG = 0 ;    
  }

  for (i = 0 ; i < MAX_MCP23017 ; i++ ) {   // initalise all the boards in the expander structure/array
      mcp[i].begin(i);              // initalise if board address as 0x20 -> 0x27
      for (j = 0 ; j < 16 ; j++ ){  // for the 16 bits
        mcp[i].pinMode(j, OUTPUT);  
        mcp[i].digitalWrite( j , true) ;   // relay board are generally false is on , true is off (arse about)           
      }
  }
  
  for (i = 0 ; i < MAX_BOARDS ; i++ ) {
    switch ( eboard[i].Type ){
      case 0:  // PCF8574
        for ( j = 0 ; j < 16 ; j++ ) {
          if ( eboard[i].Address == IOEXP[j].getaddress() ) {   // check which boards are on what address
            IOEXP[j].begin(0xFF) ;                              // start all the I2C expanders and set outputs to high           
          }
        }
      break;
      case 1:  // MCP23017
        if ((eboard[i].Address >= 0x20 ) && (eboard[i].Address <= 0x27 )) {
          k = eboard[i].Address - 0x20 ;  // this is the translated address
//          mcp[i].begin(k);              // initalise if board address is between the bounds
          for (j = 0 ; j < 16 ; j++ ){    // for the 16 bits
            if (CheckIfMCPInput(i,j)){
              mcp[k].pinMode(j, INPUT);
            }else{
              mcp[k].pinMode(j, OUTPUT);  
              mcp[k].digitalWrite( j , true) ;   // relay board are generally false is on , true is off (arse about)           
            }
          }
        }  
      break;
      case 2:  // Local I/O Pins
/*          for (j = 0 ; j < 16 ; j++ ){  // for the 16 bits
            if (CheckIfMCPInput(i,j)){
              mcp[k].pinMode(j, INPUT);
            }else{
              mcp[k].pinMode(j, OUTPUT);                
            }
          }*/
      break;
      case 3:  // None
      break ;
    }
  }
  j = 0 ;
  delay(1000);
//  Serial.println("Chip ID " + String(ESP.getChipId(), HEX));
//  Serial.println("Configuring WiFi...");

  display.setFont(ArialMT_Plain_10);

  if ( MYVER != ghks.lVersion ) {
//  if ( false ) {
    BackInTheBoxMemory();         // load defaults if blank memory detected but dont save user can still restore from eeprom
    Serial.println("Loading memory defaults...");
    delay(2000);
  }

  WiFi.disconnect();
  Serial.println("Configuring soft access point...");
  WiFi.mode(WIFI_AP_STA);  // we are having our cake and eating it eee har
  sprintf(cssid,"Configure_%08X\0",ESP.getChipId());
  if ( cssid[0] == 0 || cssid[1] == 0 ){   // pick a default setup ssid if none
    sprintf(ghks.cpassword,"\0");
  }
  MyIPC = IPAddress (192, 168, 5 +(ESP.getChipId() & 0x7f ) , 1);
  WiFi.softAPConfig(MyIPC,MyIPC,IPAddress (255, 255, 255 , 0));  
  Serial.println("Starting access point...");
  Serial.print("SSID: ");
  Serial.println(cssid);
  Serial.print("Password: >");
  Serial.print(ghks.cpassword);
  Serial.println("< " + String(ghks.cpassword[0]));
  if (( ghks.cpassword[0] == 0 ) || ( ghks.cpassword[0] == 0xff)){
    WiFi.softAP((char*)cssid);                   // no passowrd
  }else{
    WiFi.softAP((char*)cssid,(char*) ghks.cpassword);
  }
  MyIPC = WiFi.softAPIP();  // get back the address to verify what happened
  Serial.print("Soft AP IP address: ");
  snprintf(buff, BUFF_MAX, ">> IP %03u.%03u.%03u.%03u <<", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);      
  Serial.println(buff);
  
  bConfig = false ;   // are we in factory configuratin mode
  display.display();
  if ( ghks.lNetworkOptions != 0 ) {
    WiFi.config(ghks.IPStatic,ghks.IPGateway,ghks.IPMask,ghks.IPDNS ); 
  }
  if ( ghks.npassword[0] == 0 ){
    WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point      
  }else{
    WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
  }
  while (( WiFi.status() != WL_CONNECTED ) && ( j < MAX_WIFI_TRIES )) {
    j = j + 1 ;
    delay(500);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Chip ID " + String(ESP.getChipId(), HEX) );
    display.drawString(0, 9, String("SSID:") );
    display.drawString(0, 18, String("Password:") );
    display.drawString(0, 36 , String(1.0*j/2) + String(" (s)" ));   
    display.drawString(42, 36 , String(ghks.NodeName));   
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128 , 0, String(WiFi.RSSI()));
    display.drawString(128, 9, String(ghks.nssid) );
    display.drawString(128, 18, String(ghks.npassword) );
    display.drawString(j*4, 27 , String(">") );
    snprintf(buff, BUFF_MAX, ">>  IP %03u.%03u.%03u.%03u <<", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);            
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(63 , 54 ,  String(buff) );
    display.display();     
    digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED));
  } 
  if ( j >= MAX_WIFI_TRIES ) {
     bConfig = true ;
     WiFi.disconnect();
/*     IPAddress localIp(192, 168, 5 +(ESP.getChipId() & 0x7f ) , 1);
     IPAddress MaskIp(255, 255, 255 , 0);
     WiFi.softAPConfig(localIp,localIp,MaskIp);
     sprintf(ssid,"Configure_%08X\0",ESP.getChipId());
     WiFi.softAP(ssid); // configure mode no password
     MyIP = WiFi.softAPIP();
     Serial.print("Soft AP IP address: ");
     Serial.println(MyIP);
     display.drawString(0, 22, "Soft AP IP address: "+String(MyIP) );
     display.display();*/
  }else{
     Serial.println("");
     Serial.println("WiFi connected");  
     Serial.print("IP address: ");
     MyIP =  WiFi.localIP() ;
//     Serial.println(MyIP) ;
     snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);            
     Serial.println(buff);
     display.drawString(0 , 53 ,  String(buff) );
//     display.drawString(0, 53, "IP "+String(MyIP) );
     display.display();
  }
  if (ghks.localPortCtrl == ghks.localPort ){             // bump the NTP port up if they ar the same
    ghks.localPort++ ;
  }
//    Serial.println("Starting UDP");
    ntpudp.begin(ghks.localPort);                      // this is the recieve on NTP port
    display.drawString(0, 44, "NTP UDP " );
    display.display();
//    Serial.print("NTP Local UDP port: ");
//    Serial.println(ntpudp.localPort());
    ctrludp.begin(ghks.localPortCtrl);                 // recieve on the control port
    display.drawString(64, 44, "CTRL UDP " );
    display.display();
    Serial.print("Control Local UDP port: ");
    Serial.println(ctrludp.localPort());
                                                // end of the normal setup
 
  sprintf(host,"Control_%08X\0",ESP.getChipId());
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
//    Serial.println("MDNS responder started");
//    Serial.print("You can now connect to http://");
//    Serial.print(host);
//    Serial.println(".local");
  }

  server.on("/", handleRoot);
  server.on("/setup", handleRoot);
  server.on("/filt", handleRoot);
  server.on("/fert", handleRoot);
  server.on("/vsss", handleRoot);
  server.on("/prog", handleRoot);
  server.on("/prognew", handleProgramNew);
  server.on("/scan", i2cScan);
  server.on("/iosc", ioScan);
  server.on("/stime", handleRoot);
  server.on("/btest", handleRoot);
  server.on("/info", handleInfo);
  server.on("/iolocal", ioLocalMap);
  server.on("/eeprom", DisplayEEPROM);
  server.on("/backup", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_POST,  handleRoot, handleFileUpload);
  server.onNotFound(handleNotFound);  
  server.begin();
//  Serial.println("HTTP server started");
 
//  dnsServer.setTTL(300);
//  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
//  dnsServer.start(53,"injector.local",myIP);
  tc.mon = 0 ;
  tc.wday = 0 ;
  DS3231_init(DS3231_INTCN); // look for a rtc
  DS3231_get(&tc);
  rtc_status = DS3231_get_sreg();
  if (((tc.mon < 1 )|| (tc.mon > 12 ))&& (tc.wday>8)){  // no rtc to load off
    Serial.println("What NO DS3231 RTC ?");
  }else{
    setTime((int)tc.hour,(int)tc.min,(int)tc.sec,(int)tc.mday,(int)tc.mon,(int)tc.year ) ; // set the internal RTC
    hasRTC = true ;
    Serial.println("Look like it has DS3231 RTC ?");
    rtc_temp = DS3231_get_treg(); 
    rtc_hour = hour();  // dont need to updte the time if we have an RTC onboard --- assume its working ok
  }
  rtc_min = minute();
  rtc_sec = second();
  OTAWebUpdater.setup(&OTAWebServer);
  OTAWebServer.begin();  

  randomSeed(now());                       // now we prolly have a good time setting use this to roll the dice for reboot code
  lRebootCode = random(1,+2147483640) ;
}

//  ##############################  LOOP   #############################  LOOP  ##########################################  LOOP  ###################################
void loop() {
long lTime ;  
long lRet ;
int i , j , k  ;
int x , y ;
int board ;
uint8_t OnPol ;
uint8_t OffPol ;
int OnPulse ;
int OffPulse ;
bool bSendCtrlPacket ;
bool bDirty = false ;
bool bDirty2 = false ;
long lTD ;
  
  server.handleClient();
  OTAWebServer.handleClient();

  lTime = millis() ;
  iTestTime = constrain(iTestTime,2,50);
  iTestMode = constrain(iTestMode,-1,16);
/*  if (digitalRead(FLASH_BTN) == LOW) { // what to do if the button be pressed
  }*/
      
  lScanCtr++ ;
  bSendCtrlPacket = false ;
  if ( rtc_sec != second()){
    display.clear();
    if ( iTestMode != -1 ){                      // board test mode
      ActivateOutput(iTestBoard,iTestCoil,LOW,(ghks.lPulseTime % 128 ));  
      iTestTimer++ ; // count the econd while in test mode
      if (( iTestTimer % iTestTime ) == ( iTestTime - 1 )){       
        iTestMode++ ;
        if ( iTestInc == 1 ) {
          iTestCoil++ ;    // increment the coil
          if ( iTestCoil >= MaxBoardOutputs(iTestBoard )){
            iTestCoil = 0 ;
          }
        }
        if ( iTestMode >= MaxBoardOutputs(iTestBoard )){
          iTestMode = -1 ; // switch off if we have done all 16 channels
        }
      }
      display.setTextAlignment(TEXT_ALIGN_RIGHT);                   // put an indicator up to show we are in this mode
      display.drawString(127 , 30, "Testing " + String(iTestBoard));              
      display.setTextAlignment(TEXT_ALIGN_RIGHT);                   // put an indicator up to show we are in this mode
      if (iTestMode == -1 ) {
        display.drawString(127 , 41, "Done !!!" );
      }else{
        display.drawString(127 , 41, String(iTestMode));
      }
    }  
    
//      display.drawLine(minRow, 63, maxRow, 63);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
    display.drawString(0 , LineText, String(buff) );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127 , LineText, String(WiFi.RSSI()));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    switch (( rtc_sec >> 1 ) & 0x03){
      case 1:
        snprintf(buff, BUFF_MAX, "IP %03u.%03u.%03u.%03u", MyIP[0],MyIP[1],MyIP[2],MyIP[3]);      
      break;
      case 2:
        snprintf(buff, BUFF_MAX, ">>  IP %03u.%03u.%03u.%03u <<", MyIPC[0],MyIPC[1],MyIPC[2],MyIPC[3]);            
      break;
      case 3:
       snprintf(buff, BUFF_MAX, "%s - %d", ghks.NodeName,ghks.lNodeAddress );   
      break;
      default:
        snprintf(buff, BUFF_MAX, "%s", cssid );            
      break;
    }
    display.drawString(64 , 53 ,  String(buff) );
    if (ghks.lProgMethod == 0 ){
      UpdateATTG() ;                                          // update the program switching Time To Go timers
    }else{
      UpdateATTGNew() ;                                       // update the program switching Time To Go timers      
    }      
    for (i = 0 ; i < MAX_VALVE ; i++ ) {              // scan all the valves for masters then look for matching subordinates then update TTGS from them if required
      if (( evalve[i].TypeMaster & 0x80 ) != 0 ){      // This is the master valve check
        vvalve[i].lTTG = 0 ;
        if ( evalve[i].Node != 0 ){
           evalve[i].Fertigate = 0 ;
           evalve[i].Flowrate = 0 ;
        }
        for (k = 0 ; k < MAX_VALVE ; k++ ) {          // scan the list again  
          j = ( evalve[k].TypeMaster & 0x3f ) - 1 ;        // look for a subordinate valve
          if (j == i){
            if ( evalve[i].bEnable ){
              if ( vvalve[i].lATTG < vvalve[k].lATTG ){
                vvalve[i].lATTG = vvalve[k].lATTG ;
              }
            }  
            if ( vvalve[i].lTTG < vvalve[k].lTTG ){
              vvalve[i].lTTG = vvalve[k].lTTG ;
            }
            if (((vvalve[k].lTTG > 0 )||(vvalve[k].lATTG)) && ( evalve[i].Node != 0 )){  // if subordinate going then add the other stuff
              evalve[i].Fertigate |= evalve[k].Fertigate ;
              evalve[i].Flowrate += evalve[k].Flowrate ;
            }
          }    
        }
      }
      if ( i < 16 ) {
        x = i * 8 ;
        y = 15 ;       
      }else{
        x = (i-16) * 8 ;
        y = 23 ;       
      }  
      if ( vvalve[i].bOnOff ){   // valve on
        if (( evalve[i].TypeMaster & 0x40 ) != 0x00  ){  // look if it has feedback
          display.drawCircle(x+3, y, 3);
          display.drawLine(x+1, y - 2, x+5, y + 2);
          display.drawLine(x+1, y + 2, x+5, y - 2);        
        }else{
          display.fillCircle(x+3, y, 3);
        }  
      }else{                      // valve off
        if (( vvalve[i].iFB & 0x01 ) != 0x00  ){   // feedback status as if it is off or not
          display.drawCircle(x+3, y, 3);
        }else{  
          display.drawCircle(x+3, y, 1);
        }
      }
      if ( vvalve[i].lTTC > 0 ){   // decriment the seconds to clear  --- first pass
        vvalve[i].lTTC--;
        bDirty2 = true ;
      }
    }                                                        // end first pass of valves
    for (i = 0 ; i < MAX_FERT ; i++ ) { 
      if ( efert[i].CurrentQty > 0 ) {          
        if ( vfert[i].bRun ){
          display.fillCircle(i*8+3, 33, 3);
        }else{
          display.drawCircle(i*8+3, 33, 1);
        }      
      }else{
          display.drawCircle(i*8+3, 33, 3);
//        display.drawLine(i*8+1, 31, i*8+5, 35);
//        display.drawLine(i*8+1, 35, i*8+5, 31);        
      }
      if ( vfert[i].bOnOff ){
        display.fillCircle(i*8+3, 41, 3);
        bDirty = true ;
      }else{
        display.drawCircle(i*8+3, 41, 1);
      }       
    }
    display.display();
    for (i = 0 ; i < MAX_VALVE ; i++ ) {
      if ( bDirty ) {
        vvalve[i].lTTC = evalve[i].lTTC ;                                          // if dirty then start all the timers again
      }
      if (( evalve[i].TypeMaster & 0x40 ) == 0 ){                                 // do feeback first
        if ( vvalve[i].bOnOff ){
          vvalve[i].iFB  = 0xff ; 
        }else{
          vvalve[i].iFB  = 0xfe ;                
        }
      }else{
//        vvalve[i].iFB  = 0x01 & !IOEXP[((evalve[i].FeedbackBoardBit & 0xf0 ) >> 4 )].readButton((evalve[i].FeedbackBoardBit & 0x0f ));              
        vvalve[i].iFB  = 0x01 & GetIntput(((evalve[i].FeedbackBoardBit & 0xf0 ) >> 4 ) , (evalve[i].FeedbackBoardBit & 0x0f ));
      }
      OnPol = ((evalve[i].OnOffPolPulse & 0x80  ) >> 7  ) ;
      OffPol = ((evalve[i].OnOffPolPulse & 0x08  ) >> 3  ) ;
      OnPulse = SolPulseWidth((int)((evalve[i].OnOffPolPulse & 0x70  ) >> 4  )) ;
      OffPulse = SolPulseWidth((int)((evalve[i].OnOffPolPulse & 0x07  )  )) ;
      OnPol = LOW ;
      OffPol = HIGH ;
//      OnPulse =  ghks.lPulseTime % 128 ;
//      OffPulse =  ghks.lPulseTime % 128 ;
      if (((vvalve[i].lTTG > 0 )|| (vvalve[i].lATTG > 0))&&(!vvalve[i].bOnOff)){
        vvalve[i].bOnOff = true ;
        board = ( evalve[i].OnCoilBoardBit & 0xf0 ) >> 4 ; 
        board = 0 ;                                                                        // hard code for the moment
        if ( evalve[i].OffCoilBoardBit == evalve[i].OnCoilBoardBit ) {                     // 16 x  8 bit expanders
//          IOEXP[board].write( evalve[i].OnCoilBoardBit , OnPol );                        // hold on 
          ActivateOutput((( evalve[i].OnCoilBoardBit & 0xf0 ) >>4 ) , (evalve[i].OnCoilBoardBit & 0x0f ) , OnPol , 0 ) ;
        }else{
//          IOEXP[board].pulsepin( evalve[i].OnCoilBoardBit , OnPulse , OnPol );           // Pulse On          
//          Serial.println("On Pulse " + String(OnPulse));
          ActivateOutput((( evalve[i].OnCoilBoardBit & 0xf0 ) >>4 ) , (evalve[i].OnCoilBoardBit & 0x0f ) , OnPol , OnPulse ) ;
          delay(ghks.lPulseTime % 128 ); 
        }        
      }
      if ((vvalve[i].lTTG <= 0 )&&(vvalve[i].lATTG <= 0 )&&(vvalve[i].bOnOff)){
        vvalve[i].bOnOff = false ;
        board = ( evalve[i].OffCoilBoardBit & 0xf0 ) >> 4 ; 
        board = 0 ;
        if ( evalve[i].OffCoilBoardBit == evalve[i].OnCoilBoardBit ) {
//          IOEXP[board].write( evalve[i].OffCoilBoardBit , !OnPol );                      // hold off
          ActivateOutput((( evalve[i].OffCoilBoardBit & 0xf0 ) >>4 ) , (evalve[i].OffCoilBoardBit & 0x0f ) , OffPol , 0 ) ;
        }else{
//          IOEXP[board].pulsepin( evalve[i].OffCoilBoardBit , OffPulse , OffPol );        // Pulse Off 
//          Serial.println("Off Pulse " + String(OffPulse));
          ActivateOutput((( evalve[i].OffCoilBoardBit & 0xf0 ) >>4 ) , (evalve[i].OffCoilBoardBit & 0x0f ) , OnPol , OffPulse ) ;
          delay(ghks.lPulseTime % 128 ); 
        }
      }
      if ( vvalve[i].bNetOnOff != vvalve[i].bOnOff ){
        vvalve[i].bNetOnOff = vvalve[i].bOnOff ;
//        if ( ( evalve[i].Node != 0 )){                                                   // not sure if we should have this check
          bSendCtrlPacket = true ;        
//        }
      }
    }
    digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED));
    rtc_sec = second() ;
    lScanLast = lScanCtr ;
    lScanCtr = 0 ;
//    if (ghks.lProgMethod == 0 ){
//      UpdateATTG() ;                                          // update the program switching Time To Go timers
//    }else{
//      UpdateATTGNew() ;                                       // update the program switching Time To Go timers      
//    }
    if (( evalve[i].TypeMaster & 0x100 ) != 0 ){               // wee patch if this is a fert bar valve on then the line is clear of fertiliser
      if ( evalve[i].bEnable && !bDirty2 && ( vvalve[i].lATTG == 0 )){   // this is active
        vvalve[i].lATTG = 2 ;
      }
    }     
    fertigation_min();
    fertigation_sec();
  }                         // end of the once per second stuff

  if (rtc_hour != hour()){
    if ( !bConfig ) { // ie we have a network
      sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour
    }else{
      if ( hasRTC ){
        DS3231_get(&tc);
        setTime((int)tc.hour,(int)tc.min,(int)tc.sec,(int)tc.mday,(int)tc.mon,(int)tc.year ) ; // set the internal RTC
      }
    }
    rtc_hour = hour();
  }
  if ( rtc_min != minute()){
    bSendCtrlPacket = true ;
    lMinUpTime++ ;
    for (i = 0 ; i < MAX_VALVE ; i++ ) {
      if ( vvalve[i].lATTG > 0 ){
        vvalve[i].lATTG -- ;
      }else{
        if ( vvalve[i].lTTG > 0 ){
          vvalve[i].lTTG -- ;
        }
      }
    }  
    if (( year() < 1980 )|| (bDoTimeUpdate)) {  // not the correct time try to fix every minute
      if ( !bConfig ) { // ie we have a network
        sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  
        bDoTimeUpdate = false ;
      }
    }
    if ( hasRTC ){
      rtc_temp = DS3231_get_treg(); 
    }
    rtc_min = minute() ;
    fertigation_min();
    if (WiFi.isConnected())  {
      MyIP =  WiFi.localIP() ;
    }
  }
  if ((minute() == 0) && (hour() != rtc_fert_hour)){
    SaveCurrentQty(true);
    rtc_fert_hour = hour() ;
//    Serial.println("Fert Save");
  }
  if (second() > 4 ){
    if ( ntpudp.parsePacket() ) {
      processNTPpacket();
    }
  }

  lRet = ctrludp.parsePacket() ;
  if ( lRet != 0 ) {
    processCtrlUDPpacket(lRet);
  }

  if (bSendCtrlPacket){
    sendCTRLpacket() ;
    bSendCtrlPacket = false ;
  }
  if (lTimePrev > ( lTime + 100000 )){ // Housekeeping --- has wrapped around so back to zero
    lTimePrev = lTime ; // skip a bit 
    Serial.println("Wrap around");
  }
  
//  digitalWrite(SCOPE_PIN,!digitalRead(SCOPE_PIN));  // my scope says we are doing this loop at an unreasonable speed except when we do web stuff
  filter_scan();
  filter_sec();
//  dnsServer.processNextRequest();




  if (!WiFi.isConnected())  {
    lTD = (long)lTimeNext-(long) millis() ;
    if (( abs(lTD)>40000)||(bPrevConnectionStatus)){ // trying to get roll over protection and a 30 second retry
      lTimeNext = millis() - 1 ;
/*      Serial.print(millis());
      Serial.print(" ");
      Serial.print(lTimeNext);
      Serial.print(" ");
      Serial.println(abs(lTD));*/
    }
    bPrevConnectionStatus = false;
    if ( lTimeNext < millis() ){
      Serial.println(String(buff )+ " Trying to reconnect WiFi ");
      WiFi.disconnect(false);
//      Serial.println("Connecting to WiFi...");
      WiFi.mode(WIFI_AP_STA);
      if ( ghks.lNetworkOptions != 0 ) {            // use ixed IP
        WiFi.config(ghks.IPStatic, ghks.IPGateway, ghks.IPMask, ghks.IPDNS );
      }
      if ( ghks.npassword[0] == 0 ) {
        WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point
      } else {
        WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
      }
      lTimeNext = millis() + 30000 ;
    }
  }else{
    if ( !bPrevConnectionStatus  ){
      MyIP = WiFi.localIP() ;
      bPrevConnectionStatus = true ;
    }
  }  



}   //  ################### BOTTOM OF LOOP ########################





