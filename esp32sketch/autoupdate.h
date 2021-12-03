#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include <Update.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace autoupdate {
  // Timing variables
  const long interval = 3600000; // One hour in milliseconds
  unsigned long previousMillis = 0;

  // Firmware update variables
  int totalLength; // Total firmware update size
  int currentLength = 0; // Current size of written firmware
  
  HTTPClient httpClient;

  // Forward declarations
  void updateFirmware(const char* firmwareUri);
  void applyFirmware(uint8_t *data, size_t len);
  
  /**
   * At the given autoupdate::interval, checks whether a newer
   * version of the firmware exists on the backend server.
   * If so, calls the updateFirmware(uri) function to get and install
   * the new version
   * 
   * @param long interval The number of milliseconds between checks
   */
  void autoupdateCheck(long interval) {
    unsigned long currentMillis = millis();

    // If enough time has elapsed, get the latest version of the firmware from the server
    if (currentMillis - previousMillis >= interval) {

      // HTTP get request to autoupdate::firmwareVersionUri
      // The URI should be a Github endpoint that returns details about the latest release
      Serial.print("Polling ");
      Serial.println(constants::firmwareVersionUri);
      httpClient.begin(constants::firmwareVersionUri);
      int resp = httpClient.GET();
      Serial.print("HTTP request status ");
      Serial.println(resp);
    
      if (resp > 0) {
        // Get the version number from the JSON object received
        DynamicJsonDocument versionDoc(4096);
        deserializeJson(versionDoc, httpClient.getStream());
        httpClient.end();
        String serverReleaseTag = versionDoc["tag_name"];
        Serial.print("Server tag name is ");
        Serial.println(serverReleaseTag);
        Serial.print("Current release installed on this device is ");
        Serial.println(constants::releaseTagName);

        if (serverReleaseTag != constants::releaseTagName) {
          JsonObject assets_0 = versionDoc["assets"][0];
          const char* serverReleaseUri = assets_0["browser_download_url"];
          Serial.print("New release download URI is ");
          Serial.println(serverReleaseUri);
          Serial.println("Calling update function");
          updateFirmware(serverReleaseUri);
        }
      }
      else {
        Serial.println("Check failed");
        httpClient.end();
        return;
      }
    }

    // Reset the timer
    autoupdate::previousMillis = currentMillis;
  }
  
  /**
   * Writes the firmware update to the ESP32
   * 
   * @param unit8_t The data to write
   * @param size_t Number of bytes to write
   */ 
  void applyFirmware(uint8_t *data, size_t len) {
    Update.write(data, len);
    currentLength += len;
  
    // Output a dot to show progress
    Serial.print(".");
  
    // If the current length of the firmware written to the ESP32 is less than the total firmware size, 
    // go back and repeat
    if (currentLength != totalLength) {
      return;
    }
    else {
      Update.end(true);
      Serial.println("Update successful.");
      Serial.print("Total size of new firmware: ");
      Serial.println(currentLength);
      Serial.println("Restarting...");
  
      // Restart to use new firmware
      ESP.restart();
    }
  }
  
  /**
   * Updates the firmware with the file at the given firmwareUri
   * Based on https://github.com/kurimawxx00/webota-esp32/blob/main/WebOTA.ino
   * 
   * @param const char* The URI of the binary to download
   */
  void updateFirmware(const char* firmwareUri) {

    Serial.println("Firmware update message received");
    Serial.print("Connecting to ");
    Serial.println(firmwareUri);

    // The next line is important because Github gives a URL that redirects with a 302 status
    httpClient.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpClient.begin(firmwareUri);
    int resp = httpClient.GET();
    Serial.print("Response: ");
    Serial.println(resp);
  
    if (resp > 0) {
      totalLength = httpClient.getSize();
      int len = totalLength;
  
      // Start the updater
      Update.begin(UPDATE_SIZE_UNKNOWN);
      Serial.print("Firmware file size: ");
      Serial.println(totalLength);
  
      // Creates a 128-byte read buffer
      uint8_t buff[128] = { 0 };
  
      // Gets the TCP stream
      WiFiClient * stream = httpClient.getStreamPtr();
  
      // Read data from server
      Serial.println("Downloading");
      while (httpClient.connected() && (len > 0 || len == -1)) {
        size_t size = stream->available();
        if (size) {
          // Read up to 128 bytes
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
  
          // Pass this to the update function
          applyFirmware(buff, c);
  
          if (len > 0) {
            len -= c;
          }
        }
        delay(1);
      }
    }
    else {
      Serial.println("Cannot download firmware file");
    }
    httpClient.end();
  }
}

#endif  // AUTOUPDATE_H
