# IoT

Contains the files that are compiled to run on a microcontroller.

The microcontroller is an ESP32(-S2), and the PIR motion detector can be selected  
from variety of different options depending on availability of detectors.  
Contraints for selecting PIR are:  
+ around 120 degrees field of view  
+ operating voltage either 3,3V or 5V  
+ output signal voltage 3,3V.  
  
Adjustability is ootional.   [edit this pls].

## constants.h

esp32sketch.ino requires the file "constants.h". This file contains the WiFi SSID and password, as well as the MQTT server address.

Format:

```c
const char* ssid = ".....";
const char* password = ".....";
const char* mqttServer = "test.mosquitto.org";
```
