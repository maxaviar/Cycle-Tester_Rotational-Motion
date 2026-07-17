#pragma once
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//OLED macros
#define i2c_Address 0x3c //initialize with the I2C addr 0x3c, try 0x3d if this throws an error
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //  QT-PY / XIAO

//Library classes
class Adafruit_SH1106G;
extern Adafruit_SH1106G display;

//Functions
void setupOLED();
void displayCount();
