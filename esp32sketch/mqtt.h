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
  #define MSG_BUFFER_SIZE (256)
  char msg[MSG_BUFFER_SIZE];
  
  /**
   * Builds the JSON message
   * Example:
   * {
   *  "sensor": WiFi.macAddress();
   *  "detected": true|false,
   *  "firmwareVersion": constants::releaseTagName
   * }
   * 
   * @param bool Whether motion is detected
   */
  void buildMessage(bool motionState) {
    Serial.println("Building JSON message");
    DynamicJsonDocument doc(256);
    doc["sensor"] = WiFi.macAddress();
    doc["detected"] = motionState;

    /*
     * To add new sensors to the board, add their readings to the JSON message here
     */
    // doc["co2"] = co2sensor::getState();
    // doc["temperature"] = tempSensor::getState();
    
    doc["firmwareVersion"] = constants::releaseTagName;
    serializeJson(doc, msg);
    Serial.println(msg);
  }
  
  /**
   * Publishes a JSON message via MQTT to show the sensor state
   * 
   * @see buildMessage
   */
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
  
  /**
   * Reconnects to the MQTT broker
   */
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

  /**
   * Handle incoming messages
   * This is a callback function bound to the PubSubClient in setup()
   * 
   * @see setup
   * @param char* Topic name
   * @param byte* Message
   * @param unsigned int Message length
   */
  void messageReceived(char* topic, byte* payload, unsigned int length) {
    Serial.println("Received MQTT message");

    // Attempt to deserialise the JSON message
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
