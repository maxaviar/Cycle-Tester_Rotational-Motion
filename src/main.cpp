#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Adafruit_seesaw.h"
#include <seesaw_neopixel.h>

//Encoder pins
#define SS_SWITCH   24
#define INT_PIN     2

#define SEESAW_ADDR 0x36

//SSD1306 OLED (I2C mode) Pins
#define SDA_PIN A4
#define SCL_PIN A5

//OLED macros
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //  QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SPEED_POS 0
#define DWELL_POS 1
#define ANGLE_POS 2
#define PAGE_POS 3
#define COUNT_POS 4
#define START_POS 5

//Stepper macros
#define STEP_DEGREES 1.8
#define MIN_SPEED 20
#define MAX_SPEED 80
#define MIN_DWELL 0.0
#define MAX_DWELL 7.5
#define MIN_ANGLE 90.0
#define MAX_ANGLE 270.0

Adafruit_seesaw ss;

// For measuring encoder position
/*  C/Arduino optimizes and stores variables a certain way to help
    with memory management, but this could mess up data adjusted during
    an interrupt function since they can run at any point during the loop().
    Making a variable "volatile" prevents that optimization. */
volatile bool interruptFlag = false;
volatile unsigned long lastTime = 0;
const unsigned long debounceDelay = 200;
int32_t encoder_position = 0;
bool lastButtonState = false;

int speed = 20; // in steps/sec. | Ranges from 20 to 40
float dwell = 1; // in sec | Ranges from 0 to 7.5
float rotation_angle = 90; // 90 to 270
int page_number = 0;
bool start = false;
bool rst_count = false;
bool run = false;

float step_delay = 50;
int step_number = 0;
int loop_for_x_steps = 50;
int counter_position = 0;
long count = 0;

void setupEncoder();
void routeInterrupt();
void readDial(bool clockwiseTurn);
void readButton();

void setupOLED();
void displayCount();

void setupStepper();
void moveCW();
void moveCCW();
void adjustSpeedAndAngle();
void moveByAngle();

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");
  Wire.end();
  delay(500);
  //while (!Serial) delay(10);

   // --- MANUALLY UNCLOG THE I2C BUS ---
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDA_PIN, INPUT_PULLUP);
  
  // Toggle the clock pin 10 times to force the encoder to release the line
  for (int i = 0; i < 10; i++) {
    digitalWrite(SCL_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(SCL_PIN, HIGH);
    delayMicroseconds(5);
  }

  setupEncoder();
  delay(200);
  setupOLED();
  delay(200);
  setupStepper();

  Serial.println("Setup complete");
}

void loop() {
  //Will have to change this later to update count with every rotation
  displayCount();
  adjustSpeedAndAngle();

  if(interruptFlag) {
    interruptFlag = false;
    Serial.println("int marked false1");

    int32_t new_position = ss.getEncoderPosition();
    bool currButtonState = !ss.digitalRead(SS_SWITCH);
    bool clockwiseTurn = (new_position > encoder_position);

    if (currButtonState && !lastButtonState) {
      readButton();
      delay(200);
    }
    Serial.println("checked button");

    // did we move around
    if (encoder_position != new_position) {
      encoder_position = new_position;      // and save for next round
      readDial(clockwiseTurn);
    }
    Serial.println("chceked dial");
    lastButtonState = currButtonState;
  }
  
  while(run && !interruptFlag) {
    moveByAngle();
    if (interruptFlag) {
      analogWrite(3, 0);
      analogWrite(11, 0); 
    }
  }


  // don't overwhelm serial port
  delay(50);
}

void setupEncoder() {
  Serial.println("Looking for seesaw!");
  
  if (!ss.begin(SEESAW_ADDR)) {
    Serial.println("Couldn't find seesaw on default address");
    while(1) delay(10);
  }
  Serial.println("seesaw started");

  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  if (version  != 4991){
    Serial.print("Wrong firmware loaded? ");
    Serial.println(version);
    while(1) delay(10);
  }
  Serial.println("Found Product 4991");
  
  // get starting position
  encoder_position = ss.getEncoderPosition();

  Serial.println("Turning on interrupts");
  delay(10);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), routeInterrupt, FALLING);
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);
  ss.setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
  ss.enableEncoderInterrupt();  Serial.println("Encoder pins setup complete");
}

void routeInterrupt() {
  Serial.println("Int ran");
  interruptFlag = true;
}

void readDial(bool clockwiseTurn) {
  if (counter_position == SPEED_POS) {
    if((clockwiseTurn) && (speed < MAX_SPEED)) speed++;
    else if ((!clockwiseTurn) && (speed > MIN_SPEED)) speed--;
  }
  else if (counter_position == DWELL_POS) { //Figure out why only this one goes negative
    if((clockwiseTurn) && (dwell < MAX_DWELL)) dwell+=0.1;
    else if ((!clockwiseTurn) && (dwell > MIN_DWELL)) dwell-=0.1;
  }
  else if (counter_position == ANGLE_POS){
    if((clockwiseTurn) && (rotation_angle < MAX_ANGLE)) rotation_angle+=STEP_DEGREES;
    else if ((!clockwiseTurn) && (rotation_angle > MIN_ANGLE)) rotation_angle-=STEP_DEGREES;
  }
  else if (counter_position == PAGE_POS) {
    if(clockwiseTurn) page_number = 1;
    else if (!clockwiseTurn) page_number = 0;
  }
  else if (counter_position == COUNT_POS) {
    if(clockwiseTurn) rst_count = true;
    else if (!clockwiseTurn) rst_count = false;
  }
  else if (counter_position == START_POS){
    if(clockwiseTurn) start = true;
    else if (!clockwiseTurn) start = false;
  }
}

void readButton() {
  Serial.println("Button pressed");
  if (run) {
      run = false;
      start = false;
      counter_position = 0;
    }
  else if ((counter_position == COUNT_POS) && rst_count) {
    rst_count = false;
    count = 0;
    counter_position = 0;
    page_number = 0;
  }
  else if ((counter_position == START_POS) && start) {
    run = true;
    page_number = 0;
    delay(50);
  }
  else if (page_number == 0) {
    counter_position = (counter_position+1) % 4;
  }
  else {
    counter_position = (counter_position+1) % 6;
    if (counter_position == 0) {
        counter_position = PAGE_POS;
    }
  }
}

void setupOLED(){
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
 
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // draw a single pixel
  display.drawPixel(10, 10, SH110X_WHITE);
  
  // Show the display buffer on the hardware.
  display.display();
  delay(2000);
  display.clearDisplay();
}

void displayCount(){
  display.clearDisplay();

  if (page_number == 0){
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.print("Speed: ");
    display.print(speed);
    display.print(" step/sec");
    if (counter_position == SPEED_POS) display.println(" *");

    display.setCursor(0,10);
    display.print("Dwell: ");
    display.print(dwell);
    display.print(" sec");
    if (counter_position == DWELL_POS) display.println(" *");

    display.setCursor(0,20);
    display.print("Angle: ");
    display.print(rotation_angle);
    display.print(" deg");
    if (counter_position == ANGLE_POS) display.println(" *");

    display.setCursor(0,30);
    display.print("Count = ");
    display.print(count);
  
    display.setCursor(0,55);
    if (!run){
      display.print("Page (1/2)");
      if (counter_position == PAGE_POS) display.println(" *");
    }
    else {
      display.println("Press button to stop");
    }
  }

  else {
    display.setCursor(0,20);
    display.print("Reset?");
    if (!rst_count)
      display.print(" No");
    else
      display.print(" Yes");
    if (counter_position == COUNT_POS)
      display.println(" *");

    display.setCursor(0,30);
    display.print("Start? ");
    if (start)
      display.print("Yes");
    else
      display.print("No");
    
    if (counter_position == START_POS)
      display.println(" *");

    display.setCursor(0,55);
    display.print("Page (2/2)");
    if (counter_position == PAGE_POS) display.println(" *");
  } 
  display.display();
}

void setupStepper() {
  //establish motor direction toggle pins
  pinMode(12, OUTPUT); //CH A -- HIGH = forwards and LOW = backwards???
  pinMode(13, OUTPUT); //CH B -- HIGH = forwards and LOW = backwards???

  //establish motor brake pins
  pinMode(9, OUTPUT); //brake (disable) CH A
  pinMode(8, OUTPUT); //brake (disable) CH B
}

void moveCW() { //Find reset function
  if (step_number == 0){  
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, HIGH);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A
    
    step_number++;
    delay(step_delay);
  }
  else if (step_number == 1) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, HIGH);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B

    step_number++;
    delay(step_delay);
  }
  else if (step_number == 2) {
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, LOW);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A

    step_number++;
    delay(step_delay);
  }
  else if (step_number == 3) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, LOW);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B
    
    step_number = 0;
    delay(step_delay);
  }
}

void moveCCW() {
  if (step_number == 0){  
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, HIGH);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A
    
    step_number++;
    delay(step_delay);
  }
  else if (step_number == 1) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, LOW);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B

    step_number++;
    delay(step_delay);
  }
  else if (step_number == 2) {
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, LOW);   //Sets direction of CH A
    analogWrite(3, 255);   //Moves CH A

    step_number++;
    delay(step_delay);
  }
  else if (step_number == 3) {
    digitalWrite(9, HIGH);  //DISABLE CH A
    digitalWrite(8, LOW); //ENABLE CH B

    digitalWrite(13, HIGH);   //Sets direction of CH B
    analogWrite(11, 255);   //Moves CH B
    
    step_number = 0;
    delay(step_delay);
  }
}

void adjustSpeedAndAngle() {
  step_delay = 1000/speed;
  loop_for_x_steps = rotation_angle / STEP_DEGREES;
}

void moveByAngle() {
  for (int i=0; (i<loop_for_x_steps); i++) {
    moveCW();
    if (interruptFlag) return;
  }
  count++;
  displayCount();
  delay(dwell*1000);

  for (int j=loop_for_x_steps; (j>0); j--) {
    moveCCW();
    if (interruptFlag) return;
  }
  count++;
  displayCount();
  delay(dwell*1000);
}