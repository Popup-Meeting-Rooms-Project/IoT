#ifndef PIR_H
#define PIR_H

#include <Arduino.h>

namespace pir {
  // PIR sensor
  extern const int sensorPin = 15; // GPIO15
  volatile bool stateChanged = false;

  /**
   * Handle detected motion
   * 
   * This function is called as an interrupt so it needs to contain
   * the bare minimum of code. It sets the stateChanged flag, which
   * should be caught in the loop() function of the main sketch.
   * Interrupts cannot use function calls that also use interrupts.
   * These include calls to Serial and the WiFi library.
   * 
   * @see loop
   */
  void IRAM_ATTR toggleMotionDetected() {
    stateChanged = true;
  }
}

#endif  // PIR_H
