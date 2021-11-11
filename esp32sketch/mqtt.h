#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h> // MQTT library
#include <ArduinoJson.h>
#include <Arduino.h>

#include "constants.h"
#include "pir.h"

namespace wifiConnected {
  // Instantiate a WiFiClient and pass it to the MQTT library
  WiFiClient espClient;
  PubSubClient client(espClient);
  AsyncWebServer server(80); // Start a web server listening on port 80
}

extern const char mqttTopicIn[] = "hh-iot-mqtt/inTopic";
extern const char mqttTopicOut[] = "hh-iot-mqtt/outTopic";
const double firmwareVersion = 1.2;

String deviceMac = WiFi.macAddress();

// MQTT message
#define MSG_BUFFER_SIZE (128)
char msg[MSG_BUFFER_SIZE];

// Builds the JSON message
/*
 * {
 *  [
 *    "sensor": the MAC address,
 *    "detected": true or false
 *  ]
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
  wifiConnected::client.publish(mqttTopicOut, msg);
}

// Reconnects to the MQTT broker
void reconnect() {
  // Loop until reconnected
  while (!wifiConnected::client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Client ID
    // TODO: work out how to concatenate the MAC address into this
    // if this is important
    String clientID = "ESP32-";
    clientID += String(random(0xffff), HEX);

    if (wifiConnected::client.connect(clientID.c_str(), constants::mqttUser, constants::mqttPassword)) {
      // Success
      Serial.print("Connected with device ID ");
      Serial.println(clientID);

      // Publish a message to show the device is connected
      String message = "Device connected: ";
      message += clientID;
      message.toCharArray(msg, MSG_BUFFER_SIZE);
      wifiConnected::client.publish(mqttTopicOut, msg);

      // Subscribe to the topic
      wifiConnected::client.subscribe(mqttTopicIn);
    }
    else {
      // Failed
      Serial.print("Reconnect failed. RC=");
      Serial.print(wifiConnected::client.state());
      Serial.println(" Trying again in 5 seconds");

      // Wait
      delay(5000);
    }
  }
}

#endif // MQTT_H
