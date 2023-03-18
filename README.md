# Essence of Thor
Open Source Irrigation Controller Circa 2019<br>

This code is designed for the ESP platform<br>
First versions running on the ESP8266 <br>
Compiled in the arduino IDE and trageting LOLIN (WeMos) ESP8266/ESP32 compatible bopards.<br>

More details on construction
https://www.hackster.io/dougal/essence-of-thor-part-1-4a83a1

<b>I/O Support for</b><br>
MCP23017 (via I2C)<br>
PCF8574  (via I2C)<br>
LocalCPU I/O<br>

<b>2019-2020 Irrigation season progressing well 7 major bugs exorsised so far</b><br>
<br>
09/03/2020 Uploaded stuff from Jan 2020 - Static IP address and Backup/Restore program memory<br>
03/05/2020 Load in fixes for connection and UDP without DNS lookups.<br>
16/05/2020 Restore and backup of users config to PC<br>
27/09/2020 Fertigation uplink between nodes working - also two more bugs in GUI fixed<br>
06/04/2021 Valve logging working ... Started on email alarms.<br>
29/05/2022 Alarms working, Self reboot 2 modes working, Pressure alarms. AO valve type for always on programs<br>
07/06/2022 Testing and debugging with actual pressure transducer, added new valve type that always runs it's program(no tempary offline)<br>
27/06/2022 All of the above running in the feild. Endurance testing in progress....<br>
02/10/2022 Field testing going well minor updates to pages to trak a maybe problem with WiFi.
16/10/2022 Added a automated bus scan and alarm due to interboard cable issues, emails when it looses com with an i/o device also added the ability to shut of fertigation if in ADC alarm (ie low or high pressures)  May have fixed a long standing issue with web interfaces SensFooter function - time will tell.

<br>
<b>2022-2023 Irrigation season</b><br>
<br>
26/02/2023 ESP332 version going on 915MHz TTGO LoRa Board.<br>
04/03/2023        Put CRC 16 on control packets and sort of upto range testing the LoRa under real world conditions.<br>
18/03/2023        It the weekend and I dont have a life so lost of small fixes and the start of moisture monitoring

