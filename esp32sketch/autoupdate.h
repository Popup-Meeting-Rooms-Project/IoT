#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

namespace autoupdate {
  const long interval = 3600000; // One hour in milliseconds
  unsigned long previousMillis = 0;

  // Gets the latest firmware version number
  /*
   * This function expects to receive a JSON object from the URI constants::firmwareVersionUri
   * with the version number as a double variable type
   * {
   *   "version": 2.5
   * }
   */
  double fetchServerFirmwareVersion() {
    // HTTP get request to autoupdate::firmwareVersionUri
    HTTPClient httpClient;
    Serial.print("Polling ");
    Serial.println(constants::firmwareVersionUri);
    httpClient.begin(constants::firmwareVersionUri);
    int resp = httpClient.GET();
    Serial.print("HTTP request status ");
    Serial.println(resp);
  
    if (resp > 0) {
      // Get the version number from the JSON object received
      DynamicJsonDocument versionDoc(2048);
      deserializeJson(versionDoc, httpClient.getStream());
      double versionNumber = versionDoc["version"];
  
      return versionNumber;
    }
    else {
      return 0.0;
    }
  }

  /*
   * At the given autoupdate::interval, checks whether a newer
   * version of the firmware exists on the backend server.
   * If so, calls the updateFirmware(uri) function to get and install
   * the new version
   */
  void autoupdateCheck(long interval) {
    unsigned long currentMillis = millis();

    // If enough time has elapsed, get the latest version of the firmware from the server
    if (currentMillis - autoupdate::previousMillis >= interval) {
      double serverFirmwareVersion = autoupdate::fetchServerFirmwareVersion();
      if (serverFirmwareVersion > firmwareVersion) {
        // If there is a new version available, do the update
        updateFirmware(constants::firmwareUpdateUri);
      }
    }

    // Reset the timer
    autoupdate::previousMillis = currentMillis;
  }
}

#endif  // AUTOUPDATE_H
