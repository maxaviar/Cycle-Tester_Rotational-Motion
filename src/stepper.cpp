#include "common.h"
#include "stepper.h"
#include "display.h"

Stepper stepper;
float step_degrees;

void setupStepper() {
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PUL_PIN, OUTPUT);

  step_degrees = FULLSTEP_DEGREES / MICROSTEP;
}

void moveCW() {
  digitalWrite(DIR_PIN, HIGH); //Set direction
  delayMicroseconds(10);

  digitalWrite(PUL_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(PUL_PIN, LOW);
  delayMicroseconds(stepper.step_delay - 10);
}

void moveCCW() {
  digitalWrite(DIR_PIN, LOW); //Set direction
  delayMicroseconds(10);

  digitalWrite(PUL_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(PUL_PIN, LOW);
  delayMicroseconds(stepper.step_delay - 10);
}

void adjustSpeedAndAngle() {
  stepper.step_delay = 10*(settings.speed * GEAR_RATIO);
  stepper.loop_until = (settings.rotation_angle / step_degrees) * GEAR_RATIO;
}

void moveByAngle() {
  //First go counterclockwise, stopping the entire function if interrupted
  for (int i=0; (i<stepper.loop_until); i++) {
    if (interruptFlag) {
      return;
    }

    moveCCW();
  }
  settings.count++;
  displayCount();
  
  //Allows the stepper to be interrupted while dwelling
  unsigned long st_time = millis();
  unsigned long end_time = settings.dwell;
  while ((millis() < (end_time + st_time)) && !interruptFlag);

  //Same as above but clockwise now
  for (int j=stepper.loop_until; (j>0); j--) {
    if (interruptFlag) {
      return;
    }

    moveCW();
  }
  settings.count++;
  displayCount();

  st_time = millis();
  while ((millis() < (end_time + st_time)) && !interruptFlag);
}