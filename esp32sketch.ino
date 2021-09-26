/*
 * Multidisciplinary software project
 * Sketch to control an ESP32(-S2)
 */

#include <WiFi.h>
#include <PubSubClient.h> // MQTT library

/*
 * WiFi credentials and MQTT server address in a separate file
 * 
 * const char* ssid = ".....";
 * const char* password = ".....";
 * const char* mqtt_server = ".....";
 */
#include "constants.h"
const char* mqttTopicIn = "hh-iot-mqtt/inTopic";
const char* mqttTopicOut = "hh-iot-mqtt/outTopic";

// Instantiate a WiFiClient and passes it to the MQTT library
WiFiClient espClient;
PubSubClient client(espClient);

// Set basic variables
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup() {
  Serial.begin(115200);
  start_wifi();

  // The second parameter here is the port number
  // Port 1883 is the default unencrypted MQTT port
  client.setServer(mqttServer, 1883);

  // Bind a callback function to the PubSubClient
  // This is called when a message is received
  /*
   * This is a PoC at the moment. We could use it to allow
   * the front-end/back-end to poll IoT devices for their current status
   */
  client.setCallback(messageReceived);
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
  Serial.print("Message received: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Keeps the connection alive and publishes a regular pulse of messages
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send a pulse of messages, one every two seconds
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "Hello world #%ld", value);

    // Log the message
    Serial.print("Publishing message: ");
    Serial.println(msg);

    // Publish
    client.publish(mqttTopicOut, msg);
  }
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

    if (client.connect(clientID.c_str())) {
      // Success
      Serial.println("Connected");

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
