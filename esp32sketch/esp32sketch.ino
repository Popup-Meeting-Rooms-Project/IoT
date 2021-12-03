/*
 * Multidisciplinary software project
 * Sketch to control an ESP32(-S2)
 */

#include "connect_wifi.h"
#include "autoupdate.h"
#include "constants.h"
#include "mqtt.h"
#include "pir.h"
#include "tests.h"

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware version ");
  Serial.println(constants::releaseTagName);

  // Prepare unit tests
  aunit::TestRunner::include("*");
  
  // Connects to the WiFi network
  connect_wifi::startWifi();

  // Check for updates
  // The argument here is the number of milliseconds after startup that a check should be performed
  // Zero forces an immediate check
  autoupdate::autoupdateCheck(0);

  // Settings for the connection to the MQTT broker
  mqtt::client.setServer(constants::mqttServer, constants::mqttPort);
  
  // Bind a callback function to the PubSubClient
  // This is called when a message is received
  mqtt::client.setCallback(mqtt::messageReceived);

  // Attach the interrupt to handle the PIR sensor
  pinMode(pir::sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pir::sensorPin), pir::toggleMotionDetected, CHANGE);
}

// Keeps the connection alive and checks for state changes
void loop() {
  // Unit tests
  aunit::TestRunner::run();

  // Reconnect MQTT if necessary
  if (!mqtt::client.connected()) {
    mqtt::reconnect();
  }
  mqtt::client.loop();

  // Publish MQTT message if sensor state changes
  if (pir::stateChanged) {
    mqtt::publishStatus();
    pir::stateChanged = false;

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
