# IoT

## Table of Contents

1. [Hardware](#hardware)
   - [1.1. Components](#hardware-components)
   - [1.2. Circuit setup](#hardware-circuit-setup)
2. [ESP32 Microcontroller](#esp32-microcontroller)
   - [2.1. Software installation](#esp32-software-installation)
   - [2.2. Dependencies](#esp32-dependencies)
   - [2.3. constants.h](#esp32-constants.h)
   - [2.4. Usage](#esp32-usage)
   - [2.5. Polling connected devices](#esp32-polling-connected-devices)
   - [2.6. Program structure](#esp32-program-structure)
   - [2.7. Over-the-air firmware update](#esp32-over-the-air-firmware-update)
3. [MQTT Broker](#mqtt-broker)
   - [3.1. Users](#mqtt-broker-users)
   - [3.2. Topics](#mqtt-broker-topics)
   - [3.3. Access Control List](#mqtt-broker-access-control-list)


## <a name="hardware"></a>1. Hardware
### <a name="hardware-components"></a>1.1. Components
Only two hardware components are needed for this project:
- An ESP32 microcontroller
- A passive infrared sensor (PIR sensor) for motion detection

Regarding the microcontroller, any ESP32 series board can be used, as long as the board has a 32-bit MCU and 2.4 GHz Wi-Fi.

For the PIR sensor, any PIR sensor can be chosen, as long as it meets the following conditions:
- around 120 degrees field of view
- operating voltage of 3.3V or 5V
- output signal voltage of 3.3V

### <a name="hardware-circuit-setup"></a>1.2. Circuit setup

Follow these steps to connect the PIR sensor to the ESP32:

1. Connect the signal ("OUT") pin of the sensor to GPIO 15 _(Note that the GPIO pin in the program is probably not the same as the pin number on the ESP32 board)_
3. Search for your ESP32 board's pinout and check which physical pin maps to GPIO 15
   - [ESP32 series](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
   - [ESP32-S2](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/hw-reference/esp32s2/user-guide-saola-1-v1.2.html)
4. Connect the sensor's "GND" pin to a "GND" pin on the ESP32
5. Connect the sensor's "VOC" or voltage in pin to a voltage supply pin on the ESP32
   - The ESP32 can supply 3.3V or 5V
   - The one to choose depends on the model of your PIR sensor

## <a name="esp32-microcontroller"></a>2. ESP32 Microcontroller

### <a name="esp32-software-installation"></a>2.1. Software installation

Follow these steps to install the software on the ESP32:

1. Install the Arduino IDE and [set up your ESP32 board](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
2. Clone the [IoT repository](https://github.com/Popup-Meeting-Rooms-Project/IoT)
3. Open the Arduino IDE and then open `esp32sketch.ino` from the `esp32sketch` folder
4. Install the libraries used by the sketch _(See [2.2. Dependencies](#esp32-dependencies))_
5. Create the `constants.h` file _(See [2.3. constants.h](#esp32-constants.h))_
8. Connect the ESP32 to your computer via USB
9. In the Arduino IDE, click on the "Upload" icon (arrow pointing right)
  - This compiles the sketch and flashes it onto the ESP32

### <a name="esp32-dependencies"></a>2.2. Dependencies

The ESP32 sketches uses the following libraries:
* [WiFi](https://www.arduino.cc/en/Reference/WiFi)  - _This is a built-in library. You should not need to install it._
* [PubSubClient](https://pubsubclient.knolleary.net/) - MQTT Client
* [ArduinoJson](https://arduinojson.org/) - JSON Parser

Follow these steps to install the libraries that are not built in:

1. Open the Arduino IDE
2. Go to `Tools` > `Manage Libraries...`
3. Search for the libraries by name and click "Install"

### <a name="esp32-constants.h"></a>2.3. constants.h

The `constants.h`file contains the network parameters (SSID and password) of the WiFi network to which the ESP32 will connect as well as the MQTT broker URL / IP address.

`esp32sketch.ino` cannot work without the `constants.h` file and this latter must be situated in the `esp32sketch` directory _(the same directory as the `esp32sketch.ino` file)_.

File content:

```c
const char* ssid = ".....";
const char* password = ".....";
const char* mqttServer = ".....";
```

### <a name="esp32-usage"></a>2.4. Usage

When you have completed the [Software installation](#esp32-software-installation), you can start using the ESP32 by following these steps:

1. Open the Arduino IDE
2. Connect the ESP32 to your computer via USB if not already done
3. Open the serial monitor in the Arduino IDE (`Tools` > `Serial Monitor`)
4. Press the reset button on the ESP32 ("RST")
5. The serial monitor will track the progress of the ESP32 over the following steps:
  - Connecting to the WiFi
  - Connecting to the MQTT broker
6. The serial monitor will output messages when the status of motion detection changes _(The serial monitor will display the JSON message sent to the `hh-iot-mqtt/outTopic` topic. More on topics at [3.2. Topics](#mqtt-broker-topics))_
7. Poll the connected devices following the steps under "Polling connected devices"

### <a name="esp32-polling-connected-devices"></a>2.5. Polling connected devices

The sketch makes the ESP32 subscribe to the `hh-iot-mqtt/inTopic` topic. To poll all the connected devices, publish the following message to `hh-iot-mqtt/inTopic`:

```json
{
  "command": "status"
}
```
_More on topics at [3.2. Topics](#mqtt-broker-topics)._

Follow these steps to publish MQTT messages from the command line:

1. Install [Eclipse Mosquitto](https://mosquitto.org/download/)
2. In the directory where you installed Eclipse Mosquitto, enter the following command:

```bash
./mosquitto_pub -h test.mosquitto.org -t "hh-iot-mqtt/inTopic" -m '{ \"command\": \"status\" }'
```

### <a name="esp32-program-structure"></a>2.6. Program structure

The program works as follows:
1. In the global scope, the program instantiates the following variables:
  - Objects
    - `WiFiClient`: for handling the WiFi connection
    - `PubSubClient`: for handling MQTT messaging
  - Variables
    - `mqttTopicIn`: the MQTT topic for inbound messages to the ESP32
    - `mqttTopicOut`: the MQTT topic for outbound status messages from the ESP32
    - `deviceMac`: the ESP32's MAC address, which is sent as the device ID in outbound JSON status messages
    - `msg`: a char array holding messages
    - `sensorPin`: the GPIO pin connected to the PIR sensor
    - `sensorState`: ```LOW``` if the sensor does not detect motion or ```HIGH``` if it does detect motion
    - `stateChanged`: a boolean indicated whether the program needs to send a message
2. The program contains the following functions
```c
void IRAM_ATTR toggleMotionDetected()
```
This function is assigned to an interrupt. It is called when the PIR sensor state changes, and it sets `stateChanged` to true.

It was important to keep this function as short as possible, because the ESP32 will crash if the interrupt function takes too long to execute or if it tries to call any other functions that use interrupts. This means that we cannot include the logic for publishing MQTT messages within the interrupt function itself. Similarly, we shouldn't call any methods of the `Serial` object, because this relies on interrupts.

```c
void setup()
```
This function is the first function called when the program is executed. It does the following:
- Connects to the serial monitor
- Calls `start_wifi()` to establish a WiFi connection
- Sets the basic parameters for the MQTT client
- Attaches an interrupt to the GPIO connected to the PIR sensor

```c
void start_wifi()
```
This function establishes a WiFi connection using the credential specified in `constants.h`

```c
void messageReceived(char* topic, byte* payload, unsigned int length)
```
This function handles incoming messages in the `hh-iot-mqtt/inTopic` topic. If the incoming message is the JSON message specified under "Polling connected devices" above, the function calls `publishStatus()` to respond to the message.

The parameters for this function are injected by PubSubClient, which calls the method when a message is received. There are no explicit calls to this function in `esp32sketch.ino`.

```c
void loop()
```
This function checks whether the MQTT client is still connected to the server. If the connection has been dropped, it calls `reconnect()`.
The loop function also checks whether the boolean `stateChanged` has been set to `true`. If `stateChanged` is `true`, it calls `publishStatus()` and sets `stateChanged` back to `false`.

```c
void publishStatus()
```
This function publishes the ESP32's current status. It calls `buildMessage(bool)`, which returns a JSON messsage. It then publishes an MQTT message in the `hh-iot-mqtt/outTopic` containing the JSON message returned by `buildMessage`:

```c
void buildMessage(bool motionState)
```
This function creates a JSON message to communicate the ESP32's status and saves it in the global `msg` variable rather than returning it directly.

The JSON message has the following format:
```json
{
  "sensor": "0C-DD-24-A7-88-0D",
  "detected": true,
  "firmwareVersion": 1.0
}
```
where ```"sensor"``` is a string containing the MAC address of the ESP32, and `"detected"` is a boolean indicating whether the PIR sensor detects motion.

```c
void reconnect()
```
This function connects to the MQTT server. At the moment, it connects using a randomised client ID. If the connection fails, it loops until it succeeds. When it succeeds, it publishes a message in `hh-iot-mqtt/outTopic`.


### <a name="esp32-over-the-air-firmware-update"></a>2.7. Over-the-air firmware update

The sketch enables the firmware to be updated from a file on a remote server.
The update is triggered by sending an MQTT message to the `hh-iot-mqtt/inTopic` topic in the following format:

```json
{
    "command": "update",
    "uri": firmwareUri
}
```

where `firmwareUri` is the fully qualified URI of a compiled sketch, usually having the `.bin` suffix.
For example: `http://www.example.com/firware/lastest.bin`

#### Firmware update via local web server

As an interim solution, the ESP32 also runs a local web server, which enables updates to be installed.
Navigate to the endpoint `/update` at the ESP32's IP address and follow the instructions there.

## <a name="mqtt-broker"></a>3. MQTT Broker

### <a name="mqtt-broker-users"></a>3.1. Users
### <a name="mqtt-broker-topics"></a>3.2. Topics
### <a name="mqtt-broker-access-control-list"></a>3.3. Access Control List
