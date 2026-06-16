#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//Rotary Encoder Pins
#define CLK_PIN 2 //Supports interrupts, used for counting encoder ticks
#define SW_PIN 3 //Supports interrupts, used for counting encoder ticks
#define DT_PIN 4 

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
#define MAX_SPEED 40
#define MIN_DWELL 0.0
#define MAX_DWELL 7.5
#define MIN_ANGLE 90.0
#define MAX_ANGLE 270.0

// For measuring encoder position
/*  C/Arduino optimizes and stores variables a certain way to help
    with memory management, but this could mess up data adjusted during
    an interrupt function since they can run at any point during the loop().
    Making a variable "volatile" prevents that optimization. */

volatile int speed = 20; // in steps/sec. | Ranges from 20 to 40
volatile float dwell = 1; // in sec | Ranges from 0 to 7.5
volatile float rotation_angle = 90; // 90 to 270
volatile int page_number = 0;
volatile bool start = false;
volatile bool rst_count = false;
volatile bool run = false;

float step_delay = 50;
int step_number = 0;
int loop_for_x_steps = 50;
int counter_position = 0;
long count = 0;

//Function prototypes separated by which component they are most relevant to
void setupEncoder();
void readDial();
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
  
  delay(500);
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

  while(run) {
    moveByAngle();
  }
}

void setupEncoder() {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), readDial, RISING);
  attachInterrupt(digitalPinToInterrupt(SW_PIN), readButton, FALLING); // Maybe on CHANGE instead of LOW?

  
  Serial.println("Encoder pins setup complete");
}

void readDial() {
  if (counter_position == SPEED_POS) {
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (speed < MAX_SPEED)) speed++;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (speed > MIN_SPEED)) speed--;
  }
  else if (counter_position == DWELL_POS) { //Figure out why only this one goes negative
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (dwell < MAX_DWELL)) dwell+=0.1;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (dwell > MIN_DWELL)) dwell-=0.1;
  }
  else if (counter_position == ANGLE_POS){
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (rotation_angle < MAX_ANGLE)) rotation_angle+=STEP_DEGREES;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (rotation_angle > MIN_ANGLE)) rotation_angle-=STEP_DEGREES;
  }
  else if (counter_position == PAGE_POS) {
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN))) page_number = (page_number+1)%2;
  }
  else if (counter_position == COUNT_POS) {
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN))) rst_count = !rst_count;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN))) rst_count = !rst_count;
  }
  else if (counter_position == START_POS){
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN))) start = !start;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN))) start = !start;
  }
}

void readButton() {
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
  delay(250); //for debouncing
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

void moveCW() {
  if (step_number == 0){  
    digitalWrite(9, LOW);  //ENABLE CH A
    digitalWrite(8, HIGH); //DISABLE CH B

    digitalWrite(12, HIGH);   //Sets direction of CH A
    analogWrite(10, 255);   //Moves CH A
    
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
    analogWrite(10, 255);   //Moves CH A

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
    analogWrite(10, 255);   //Moves CH A
    
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
    analogWrite(10, 255);   //Moves CH A

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
  for (int i=0; (i<loop_for_x_steps) && run; i++) moveCW();
  if (run) count++;
  displayCount();
  delay(dwell*1000);

  for (int j=loop_for_x_steps; (j>0) && run; j--) moveCCW();
  if (run) count++;
  displayCount();
  delay(dwell*1000);
}