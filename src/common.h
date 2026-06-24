#pragma once
#include <Arduino.h>

//I2C Pins
#define SDA_PIN A4
#define SCL_PIN A5

//UI Positions
#define SPEED_POS 0
#define DWELL_POS 1
#define ANGLE_POS 2
#define PAGE_POS 3
#define COUNT_POS 4
#define START_POS 5

#define STEP_DEGREES 1.8
#define GEAR_RATIO 4

//  Global variables
//  For measuring encoder position
/*  C/Arduino optimizes and stores variables a certain way to help
    with memory management, but this could mess up data adjusted during
    an interrupt function since they can run at any point during the loop().
    Making a variable "volatile" prevents that optimization. */
extern volatile bool interruptFlag;
extern int32_t encoder_position;
extern bool lastButtonState;

struct Settings {
    int speed = 40; // in steps/sec. | Ranges from 40 to 80
    float dwell = 0; // in sec | Ranges from 0 to 7.5
    float rotation_angle = 90; // 90 to 270
    bool start = false;
    bool rst_count = false;
    bool run = false;
    long count = 0;
    int position = 0; // was counter_position
    int page_number = 0;
};

extern Settings settings;