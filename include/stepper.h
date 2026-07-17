#pragma once

#define PUL_PIN 11
#define DIR_PIN 12

//Globals and/or Structs
struct Stepper {
    float step_delay = 50;
    int step_number = 0;
    int loop_until = 50; //Amount of steps to make rotation angle
};

extern Stepper stepper;
extern float step_degrees;

//Functons
void setupStepper();
void moveCW();
void moveCCW();
void adjustSpeedAndAngle();
void moveByAngle();