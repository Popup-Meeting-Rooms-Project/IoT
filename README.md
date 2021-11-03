# IoT

Contains the files that are compiled to run on a microcontroller.

The microcontroller is an ESP32(-S2), and the PIR motion detector can be selected  
from variety of different options depending on availability of detectors.  
Contraints for selecting PIR are:  
+ around 120 degrees field of view  
+ operating voltage either 3,3V or 5V  
+ output signal voltage 3,3V.  
  
Adjustability is ootional.   [edit this pls].

## Installation

Follow these steps to install the software on an ESP32.

0. Install the Arduino IDE and [set up your ESP32 board](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
1. Clone the IoT repository
2. Open the Arduino IDE and then open ```esp32sketch.ino``` from the folder esp32sketch
3. Check you have installed all the libraries used by the sketch
  + See "Libraries used" below
4. Create the ```constants.h``` file
  + See "constants.h" below
  + ```constants.h``` contains the SSID and password of the WiFi access point where the ESP32 will be used
5. Save ```constants.h``` in your cloned ```esp32sketch``` directory (the same directory as the ```esp32sketch.ino``` file)
6. Connect your PIR sensor to the ESP32
  + Connect the signal ("OUT") pin of the sensor to GPIO 15
  + Note that the GPIO pin in the program is probably not the same as the pin number on the ESP32 board
  + Search for your ESP32 board's pinout and check which physical pin maps to GPIO 15
    + [ESP32 series](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
    + [ESP32-S2](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-saola-1-v1.2.html)
  + Connect the sensor's "GND" pin to a "GND" pin on the ESP32
  + Connect the sensor's "VOC" or voltage in pin to a voltage supply pin on the ESP32
    + The ESP32 can supply 5 V or 3.3 V
    + The one to choose depends on the model of your PIR sensor
8. Connect your ESP32 to your computer via USB
9. In the Arduino IDE, click the "Upload" icon (arrow pointing right)
  + This compiles the sketch and flashes it onto the ESP32

## Libraries used

The ESP32 sketch uses the following libraries:
+ [WiFi](https://www.arduino.cc/en/Reference/WiFi)
  + This is a built-in library, so you should not need to install it
+ [PubSubClient](https://pubsubclient.knolleary.net/)
+ [ArduinoJson](https://arduinojson.org/)

Follow these steps to install the libraries that are not built in:

1. Open the Arduino IDE
2. Go to Tools > Manage Libraries...
3. Search for the libraries by name and click "Install"

## constants.h

```esp32sketch.ino``` requires the file ```constants.h```. This file contains the WiFi SSID and password, as well as the MQTT server address.

Format:

```c
const char* ssid = ".....";
const char* password = ".....";
const char* mqttServer = "test.mosquitto.org";
```

## Usage

First follow the instructions under "Installation". When you have installed the sketch on an ESP32, follow these steps.

1. Open the Arduino IDE
2. Make sure the ESP32 is connected to your computer via USB
3. Open the serial monitor in the Arduino IDE
  + Tools > Serial Monitor
4. Press the reset button on the ESP32 ("RST")
5. The serial monitor will track the progress of the ESP32 over the following steps
  + Connecting to the WiFi
  + Connecting to the MQTT server
6. The serial monitor will output messages when the status of motion detection changes
  + The serial monitor will display the JSON message sent to the ```hh-iot-mqtt/outTopic``` topic
7. Poll the connected devices following the steps under "Polling connected devices"

## Polling connected devices

The sketch makes the ESP32 subscribe to the ```hh-iot-mqtt/inTopic``` topic. To poll all the connected devices, publish the following message to ```hh-iot-mqtt/inTopic```:

```json
{
  "command": "status"
}
```

Follow these steps to publish MQTT messages from the command line:

1. Install [Eclipse Mosquitto](https://mosquitto.org/download/)
2. In the directory where you installed Eclipse Mosquitto, enter the following command:

```bash
./mosquitto_pub -h test.mosquitto.org -t "hh-iot-mqtt/inTopic" -m '{ \"command\": \"status\" }'
```

## Program structure

The program works as follows:
1. In the global scope, the program instantiates the following variables:
  + Objects
    + ```WiFiClient```: for handling the WiFi connection
    + ```PubSubClient```: for handling MQTT messaging
  + Variables
    + ```mqttTopicIn```: the MQTT topic for inbound messages to the ESP32
    + ```mqttTopicOut```: the MQTT topic for outbound status messages from the ESP32
    + ```deviceMac```: the ESP32's MAC address, which is sent as the device ID in outbound JSON status messages
    + ```msg```: a char array holding messages
    + ```sensorPin```: the GPIO pin connected to the PIR sensor
    + ```sensorState```: ```LOW``` if the sensor does not detect motion or ```HIGH``` if it does detect motion
    + ```stateChanged```: a boolean indicated whether the program needs to send a message
2. The program contains the following functions
```c
void IRAM_ATTR toggleMotionDetected()
```
This function is assigned to an interrupt. It is called when the PIR sensor state changes, and it sets ```stateChanged``` to true.

It was important to keep this function as short as possible, because the ESP32 will crash if the interrupt function takes too long to execute or if it tries to call any other functions that use interrupts. This means that we cannot include the logic for publishing MQTT messages within the interrupt function itself. Similarly, we shouldn't call any methods of the ```Serial``` object, because this relies on interrupts.

```c
void setup()
```
This function is the first function called when the program is executed. It does the following:
+ Connects to the serial monitor
+ Calls ```start_wifi()``` to establish a WiFi connection
+ Sets the basic parameters for the MQTT client
+ Attaches an interrupt to the GPIO connected to the PIR sensor

```c
void start_wifi()
```
This function establishes a WiFi connection using the credential specified in ```constants.h```

```c
void messageReceived(char* topic, byte* payload, unsigned int length)
```
This function handles incoming messages in the ```hh-iot-mqtt/inTopic``` topic. If the incoming message is the JSON message specified under "Polling connected devices" above, the function calls ```publishStatus()``` to respond to the message.

The parameters for this function are injected by PubSubClient, which calls the method when a message is received. There are no explicit calls to this function in ```esp32sketch.ino```.

```c
void loop()
```
This function checks whether the MQTT client is still connected to the server. If the connection has been dropped, it calls ```reconnect()```.
The loop function also checks whether the boolean ```stateChanged``` has been set to ```true```. If ```stateChanged``` is ```true```, it calls ```publishStatus()``` and sets ```stateChanged``` back to ```false```.

```c
void publishStatus()
```
This function publishes the ESP32's current status. It calls ```buildMessage(bool)```, which returns a JSON messsage. It then publishes an MQTT message in the ```hh-iot-mqtt/outTopic``` containing the JSON message returned by ```buildMessage```:

```c
void buildMessage(bool motionState)
```
This function creates a JSON message to communicate the ESP32's status and saves it in the global ```msg``` variable rather than returning it directly.

The JSON message has the following format:
```json
{
  "sensor": "0C-DD-24-A7-88-0D",
  "detected": true,
  "firmwareVersion": 1.0
}
```
where ```"sensor"``` is a string containing the MAC address of the ESP32, and ```"detected"``` is a boolean indicating whether the PIR sensor detects motion.

```c
void reconnect()
```
This function connects to the MQTT server. At the moment, it connects using a randomised client ID. If the connection fails, it loops until it succeeds. When it succeeds, it publishes a message in ```hh-iot-mqtt/outTopic```.
