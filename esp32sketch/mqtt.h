#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h> // MQTT library
#include <ArduinoJson.h>
#include <Arduino.h>

#include "constants.h"
#include "pir.h"

/*
 * Handles interactions with the MQTT broker
 * Some sections derived from the PubSubClient example sketch
 * https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
 */
namespace mqtt {
  // Instantiate a WiFiClient and pass it to the MQTT library
  WiFiClient espClient;
  PubSubClient client(espClient);

  const char mqttTopicIn[] = "hh-iot-mqtt/inTopic";
  const char mqttTopicOut[] = "hh-iot-mqtt/outTopic";
  String deviceMac = WiFi.macAddress();
  
  // MQTT message
  #define MSG_BUFFER_SIZE (128)
  char msg[MSG_BUFFER_SIZE];
  
  // Builds the JSON message
  /*
   * {
   *  "sensor": WiFi.macAddress();
   *  "detected": true|false,
   *  "firmwareVersion": constants::releaseTagName
   * }
   */
  void buildMessage(bool motionState) {
    Serial.println("Building JSON message");
    DynamicJsonDocument doc(128);
    doc["sensor"] = WiFi.macAddress();
    doc["detected"] = motionState;
    doc["firmwareVersion"] = constants::releaseTagName;
    serializeJson(doc, msg);
    Serial.println(msg);
  }
  
  // Publishes an MQTT message with the current status
  void publishStatus() {
    if (digitalRead(pir::sensorPin) == HIGH) {
      Serial.println("Motion detected");
      buildMessage(true);
    }
    else {
      Serial.println("Motion no longer detected");
      buildMessage(false);
    }
    client.publish(mqttTopicOut, msg);
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
  
      if (client.connect(clientID.c_str(), constants::mqttUser, constants::mqttPassword)) {
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
    // If the "command" is "update",
    // poll Github for the latest release tag,
    // and download and apply the latest release if it is not the same as the installed version
    if (command == "update") {
      autoupdate::autoupdateCheck(0);
    }
  }
}

#endif // MQTT_H
