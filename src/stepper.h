#pragma once

//Globals and/or Structs
struct Stepper {
    float step_delay = 50;
    int step_number = 0;
    int loop_until = 50; //Amount of steps to make rotation angle
};

extern Stepper stepper;

//Functons
void setupStepper();
void moveCW();
void moveCCW();
void adjustSpeedAndAngle();
void moveByAngle();