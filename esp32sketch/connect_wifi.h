#ifndef CONNECT_WIFI_H
#define CONNECT_WIFI_H

#include <WiFi.h>
#include <Arduino.h>
#include "constants.h"

namespace connect_wifi {
  /**
   * Establishes a WiFi connection
   * 
   * @param const char* Name of the SSID
   * @param const char* Password for the SSID
   */
  void startWifi(const char* ssid, const char* password) {
    delay(10);
  
    // Log to serial console
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    // Pass credentials to the WiFi library
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  
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
}

#endif  // CONNECT_WIFI_H
