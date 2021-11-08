#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Update.h>
#include <HTTPClient.h>

// Variables to handle lengths of firmware updates
int totalLength; // Total firmware update size
int currentLength = 0; // Current size of written firmware

// Writes the firmware update to the ESP32
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

// Updates the firmware with the file at the given firmwareUri
// Based on https://github.com/kurimawxx00/webota-esp32/blob/main/WebOTA.ino
void updateFirmware(String firmwareUri) {
  Serial.println("Firmware update message received");
  Serial.print("Connecting to ");
  Serial.println(firmwareUri);
  HTTPClient httpClient;
  httpClient.begin(firmwareUri);
  int resp = httpClient.GET();
  Serial.print("Response: ");
  Serial.println(resp);

  // TODO: work out if this should be comparing resp == 200
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

#endif // FIRMWARE_H
