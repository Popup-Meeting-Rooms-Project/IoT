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

test(thisThing) {
  assertStringCaseEqual("hello", "Hello");
}

#endif  // TESTS_H
