#ifndef PIR_H
#define PIR_H

#include <Arduino.h>

// PIR sensor
extern const int sensorPin = 15; // GPIO15
volatile bool stateChanged = false;

// Handles detected motion
/*  This function is called as an interrupt so it needs to contain
 *  the bare minimum of code.
 *  Interrupts cannot use function calls that also use interrupts.
 *  These include calls to Serial and the WiFi library.
 */
void IRAM_ATTR toggleMotionDetected() {
  stateChanged = true;
}

#endif  // PIR_H
