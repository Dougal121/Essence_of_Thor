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

2019-2020 Irrigation season progressing well 7 major bugs exorsised so far<br>
<br>
09/03/2020 Uploaded stuff from Jan 2020 - Static IP address and Backup/Restore program memory<br>
03/05/2020 Load in fixes for connection and UDP without DNS lookups.<br>
16/05/2020 Restore and backup of users config to PC<br>
27/09/2020 Fertigation uplink between nodes working - also two more bugs in GUI fixed<br>
06/04/2021 Valve logging working ... Started on email alarms.<br>
29/05/2022 Alarms working, Self reboot 2 modes working, Pressure alarms. AO valve type for always on programs<br>
07/06/2022 Testing and debugging with actual pressure transducer, added new valve type that always runs it's program(no tempary offline)<br>
