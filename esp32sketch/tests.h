#ifndef TESTS_H
#define TESTS_H

#include <AUnit.h>

test(example) {

  String Mac = WiFi.macAddress();
  
  #define MSG_BUFFER_SIZE (128)
  char msg[MSG_BUFFER_SIZE];

  Serial.println("Building JSON message");
  DynamicJsonDocument doc(128);
  doc["sensor"] = Mac;
  doc["detected"] = true;
  doc["firmwareVersion"] = 0.1;
  serializeJson(doc, msg);
  Serial.println(msg);

  assertStringCaseEqual(msg, "{\"sensor\":\""+ Mac +"\",\"detected\":true,\"firmwareVersion\":0.1}");
  
}

test(Connectivity){


  assertEqual(WiFi.status(), WL_CONNECTED);
}

test(FirmwarePoll){

  
  int totalLength; 
  int currentLength = 0;
  
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


test(thisThing) {
  assertStringCaseEqual("hello", "Hello");
}

#endif  // TESTS_H
