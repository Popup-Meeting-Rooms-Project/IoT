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

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); 

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  assertEqual(WiFi.status(), WL_CONNECTED);
}

test(thisThing) {
  assertStringCaseEqual("hello", "Hello");
}

#endif  // TESTS_H
