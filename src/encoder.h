#pragma once
#include "Adafruit_seesaw.h"

//Encoder pins
#define SS_SWITCH   24
#define INT_PIN     2
#define SEESAW_ADDR 0x36

//Stepper macros
#define MIN_SPEED 20
#define MAX_SPEED 80
#define MIN_DWELL 0.0
#define MAX_DWELL 7.5
#define MIN_ANGLE 90.0
#define MAX_ANGLE 270.0

//Library classes
class Adafruit_seesaw;
extern Adafruit_seesaw ss;

//Functions
void setupEncoder();
void routeInterrupt();
void readDial(bool clockwiseTurn);
void readButton();