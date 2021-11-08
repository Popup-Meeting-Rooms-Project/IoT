#ifndef CONNECT_WIFI_H
#define CONNECT_WIFI_H

#include <WiFi.h>
#include <Arduino.h>
#include "constants.h"

// Establishes a WiFi connection
void startWifi() {
  // This seemed necessary
  // TODO: work out why!
  delay(10);

  // Log to serial console
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(constants::ssid); // From "constants.h"

  // Pass credentials to the WiFi library
  WiFi.mode(WIFI_STA);
  WiFi.begin(constants::ssid, constants::password); // From "constants.h"

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

#endif  // CONNECT_WIFI_H
