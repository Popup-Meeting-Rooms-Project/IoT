/*
 * Multidisciplinary software project
 * Sketch to control an ESP32(-S2)
 * MQTT sections derived from the PubSubClient example sketch
 * https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
 */

#include <WiFi.h>
#include <PubSubClient.h> // MQTT library
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

/*
 * WiFi and MQTT server credentials in a separate file
 * 
 * const char* ssid = ".....";
 * const char* password = ".....";
 * IPAddress mqttServer(..., ..., ..., ...); 
 * const char* mqttUser = ".....";
 * const char* mqttPassword = ".....";
 * const int mqttPort = ....;
 */
#include "constants.h"

<<<<<<< Updated upstream
const char* mqttTopicIn = "hh-iot-mqtt/inTopic";
const char* mqttTopicOut = "hh-iot-mqtt/outTopic";
const double firmwareVersion = 1.0;
=======
// Names of the MQTT topics
const char* mqttTopicIn = "hh-iot-mqtt/inTopic"; // Inbound messages from other devices to the ESP32 devices
const char* mqttTopicOut = "hh-iot-mqtt/outTopic"; // Outbound messages from the ESP32 devices
const double firmwareVersion = 1.1;
>>>>>>> Stashed changes

// Instantiate a WiFiClient and pass it to the MQTT library, start a web server
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80); // Start a web server listening on port 80

// Device MAC address (serves as the device ID,
// which the backend and frontend will map to a physical room)
String deviceMac = WiFi.macAddress();

// MQTT message
#define MSG_BUFFER_SIZE (128)
char msg[MSG_BUFFER_SIZE];

// PIR sensor
const int sensorPin = 15; // GPIO15
volatile bool stateChanged = false;

// Handles detected motion
/*  This function is called as an interrupt so it needs to contain
 *  the bare minimum of code.
 *  Interrupts cannot use function calls that also use interrupts.
 *  These include calls to Serial and the WiFi library.
 */
void IRAM_ATTR toggleMotionDetected() {
  stateChanged = true;
}

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware version ");
  Serial.println(firmwareVersion);
  start_wifi();

  // Start web server to enable OTA firmware updates
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "This is the default Elegant OTA web server. Go to the endpoint /update to upload a new firmware version");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");

  // Pass the MQTT configuration variables to the PubSubClient MQTT object
  client.setServer(mqttServer, mqttPort);

  // Bind a callback function to the PubSubClient
  // This is called when a message is received
  client.setCallback(messageReceived);

  // Attach the interrupt to handle the PIR sensor
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), toggleMotionDetected, CHANGE);
}

// Establishes a WiFi connection
void start_wifi() {
  // This seemed necessary
  // TODO: work out why!
  delay(10);

  // Log to serial console
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); // From "constants.h"

  // Pass credentials to the WiFi library
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // From "constants.h"

  // Loop until connection is ready
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connected
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Displays an inbound message
// This is a callback function bound to the PubSubClient in setup()
void messageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.println("Received MQTT message");
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  String command = doc["command"];
  Serial.print("Command: ");
  Serial.println(command);
  // If the deserialised JSON contains the key "command" with the value "status",
  // publish an MQTT message containing the device status
  if (command == "status") {
    publishStatus();
  }
}

// Keeps the connection alive and checks for state changes
void loop() {
  // Handles the MQTT portion of the loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Reacts to changes in the PIR sensor state
  if (stateChanged) {
    publishStatus();
    stateChanged = false;
  }
}

// Publishes an MQTT message with the current status
void publishStatus() {
  if (digitalRead(sensorPin) == HIGH) {
    Serial.println("Motion detected");
    buildMessage(true);
  }
  else {
    Serial.println("Motion no longer detected");
    buildMessage(false);
  }
  client.publish(mqttTopicOut, msg);
}

// Builds the JSON message
/*
 * {
 *    "sensor": the MAC address,
 *    "detected": true or false,
 *    "firmwareVersion": the version number
 * }
 */
void buildMessage(bool motionState) {
  Serial.println("Building JSON message");
  DynamicJsonDocument doc(128);
  doc["sensor"] = deviceMac;
  doc["detected"] = motionState;
  doc["firmwareVersion"] = firmwareVersion;
  serializeJson(doc, msg);
  Serial.println(msg);
}

// Reconnects to the MQTT broker
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Client ID
    // TODO: work out how to concatenate the MAC address into this
    // if this is important
    String clientID = "ESP32-";
    clientID += String(random(0xffff), HEX);

    if (client.connect(clientID.c_str(), mqttUser, mqttPassword)) {
      // Success
      Serial.print("Connected with device ID ");
      Serial.println(clientID);

      // Publish a message to show the device is connected
      String message = "Device connected: ";
      message += clientID;
      message.toCharArray(msg, MSG_BUFFER_SIZE);
      client.publish(mqttTopicOut, msg);

      // Subscribe to the topic
      client.subscribe(mqttTopicIn);
    }
    else {
      // Failed
      Serial.print("Reconnect failed. RC=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds");

      // Wait
      delay(5000);
    }
  }
}
