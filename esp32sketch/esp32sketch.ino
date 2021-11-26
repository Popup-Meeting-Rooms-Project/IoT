/*
 * Multidisciplinary software project
 * Sketch to control an ESP32(-S2)
 * MQTT sections derived from the PubSubClient example sketch
 * https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
 */

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include "connect_wifi.h"
#include "constants.h"
#include "firmware.h"
#include "mqtt.h"
#include "pir.h"
#include "autoupdate.h"
#include "tests.h"

// PIR sensor
extern const int sensorPin;
extern volatile bool stateChanged;

// Calls the toggleMotionDetected function from the pir.h file
void IRAM_ATTR toggleMotionDetected();

void messageReceived(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware version ");
  Serial.println(firmwareVersion);

  // Prepare unit tests
  aunit::TestRunner::include("*");
  
  // Calls the start_wifi function from the wifi.h file
  startWifi();

  // Start web server to enable OTA firmware updates
  wifiConnected::server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "This is the default Elegant OTA web server. Go to the endpoint /update to upload a new firmware version");
  });

  // Start ElegantOTA to allow firmware to be uploaded to this individual board
  AsyncElegantOTA.begin(&wifiConnected::server);
  wifiConnected::server.begin();
  Serial.println("HTTP server started");

  // The second parameter here is the port number
  // Port 1883 is the default unencrypted MQTT port
  wifiConnected::client.setServer(constants::mqttServer, constants::mqttPort);
  
  // Bind a callback function to the PubSubClient
  // This is called when a message is received
  wifiConnected::client.setCallback(messageReceived);

  // Attach the interrupt to handle the PIR sensor
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), toggleMotionDetected, CHANGE);
}

// Keeps the connection alive and checks for state changes
void loop() {
  // Unit tests
  aunit::TestRunner::run();

  // Reconnect MQTT if necessary
  if (!wifiConnected::client.connected()) {
    reconnect();
  }
  wifiConnected::client.loop();

  // Publish MQTT message if sensor state changes
  if (stateChanged) {
    publishStatus();
    stateChanged = false;

    /*
     * Prevent new messages from being sent for five seconds
     * This is to avoid "spamming" the MQTT server with a constant stream
     * of "detected" / "undetected" messages
     * The value of the delay may need to be fine-tuned
     */
    delay(5000);
  }

  // Check if updated firmware is available
  autoupdate::autoupdateCheck(autoupdate::interval);
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
  if (command == "update") {
    updateFirmware(doc["uri"]);
  }
}
