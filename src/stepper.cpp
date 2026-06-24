#include "common.h"
#include "stepper.h"
#include "display.h"

Stepper stepper;

void setupStepper() {
    //establish motor direction toggle pins
    pinMode(12, OUTPUT); //CH A -- HIGH = forwards and LOW = backwards???
    pinMode(13, OUTPUT); //CH B -- HIGH = forwards and LOW = backwards???

    //establish motor brake pins
    pinMode(9, OUTPUT); //brake (disable) CH A
    pinMode(8, OUTPUT); //brake (disable) CH B
}

void moveCW() { //Find reset function
  if (stepper.step_number == 0){  
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, HIGH);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A
    
    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 1) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, HIGH);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B

    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 2) {
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, LOW);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A

    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 3) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, LOW);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B
    
    stepper.step_number = 0;
    delay(stepper.step_delay);
  }
}

void moveCCW() {
  if (stepper.step_number == 0){  
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, HIGH);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A
    
    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 1) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, LOW);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B

    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 2) {
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, LOW);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A

    stepper.step_number++;
    delay(stepper.step_delay);
  }
  else if (stepper.step_number == 3) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, HIGH);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B
    
    stepper.step_number = 0;
    delay(stepper.step_delay);
  }
}

void adjustSpeedAndAngle() {
  stepper.step_delay = 1000/(settings.speed * GEAR_RATIO);
  stepper.loop_until = (settings.rotation_angle / STEP_DEGREES) * GEAR_RATIO                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ;
}

void moveByAngle() {
    for (int i=0; (i<stepper.loop_until); i++) {
    moveCW();
    if (interruptFlag) return;
  }
  settings.count++;
  displayCount();
  delay(settings.dwell*1000);

  for (int j=stepper.loop_until; (j>0); j--) {
    moveCCW();
    if (interruptFlag) return;
  }
  settings.count++;
  displayCount();
  delay(settings.dwell*1000);
}