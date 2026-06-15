#include <Arduino.h>
#include <HardwareSerial.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//Rotary Encoder
#define CLK_PIN 2 //Supports interrupts, used for counting encoder ticks
#define DT_PIN 4 
#define SW_PIN 3 //Supports interrupts, used for counting encoder ticks

//SSD1306 OLED (I2C mode)
#define SDA_PIN A4
#define SCL_PIN A5

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
                         // e.g. the one with GM12864-77 written on it
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define STEP_DEGREES 1.8

#define MIN_SPEED 20
#define MAX_SPEED 40
#define MIN_DWELL 0.0
#define MAX_DWELL 7.5
#define MIN_ANGLE 90
#define MAX_ANGLE 270

// For measuring encoder position
// Volatile bc they get adjusted via interrupts
// C/Arduino optimizes and stores variables a certain way to help
// with memory management, but this could mess up data adjusted during
// an interrupt function since they can run at any point during the loop().
// Making a variable "volatile" prevents that optimization.
volatile int speed = 20; // in steps/sec. | Ranges from 20 to 40
volatile float dwell = 3; // in sec | Ranges from 0 to 7.5
volatile int rotation_angle = 90; // 90 to 270
volatile bool start = false;
volatile bool run = false;

int step_delay = 50;
int step_number = 0;

int counter_position = 0;

void setupEncoder();
void readDial();
void readButton();

void setupOLED();
void displayCount();

void setupStepper();
void moveCW();
void moveCCW();
void adjustSpeed();
void moveByAngle();

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");
  
  delay(500);
  setupEncoder();
  delay(500);
  setupOLED();
  delay(500);
  setupStepper();

  Serial.println("Setup complete");
}

void loop() {
  //Will have to change this later to update count with every rotation
  displayCount();

  while(run) moveByAngle();
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
  if (counter_position == 0) {
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (speed < MAX_SPEED)) speed++;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (speed > MIN_SPEED)) speed--;
  }
  else if (counter_position == 1) { //Figure out why only this one goes negative
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (dwell < MAX_DWELL)) dwell+=0.1;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (dwell > MIN_DWELL)) dwell-=0.1;
  }
  else if (counter_position == 2){
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN)) && (rotation_angle < MAX_ANGLE)) rotation_angle+=5;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN)) && (rotation_angle > MIN_ANGLE)) rotation_angle-=5;
  }
  else {
    if((digitalRead(CLK_PIN) == digitalRead(DT_PIN))) start = !start;
    else if ((digitalRead(CLK_PIN) != digitalRead(DT_PIN))) start = !start;
  }
}

void readButton() {
  Serial.println("Button pressed");
  if (run) {
    run = false;
    start = false;
    Serial.println("Test stopped");
  }
  else if ((counter_position == 3) && start) {
    run = true;
    Serial.println("Test starting");
  }
  
  else counter_position = (counter_position+1) % 4;
  
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

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(5,10);
  display.print("Speed = ");
  display.print(speed);
  if (counter_position == 0) display.println(" <-");

  display.setCursor(5,20);
  display.print("Dwell = ");
  display.print(dwell);
  if (counter_position == 1) display.println(" <-");

  display.setCursor(5,30);
  display.print("Angle = ");
  display.print(rotation_angle);
  if (counter_position == 2) display.println(" <-");

  display.setCursor(5,50);
  display.print("Start? ");
  if (start) display.println("Yes");
  else display.println("No");

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

void adjustSpeed() {
  step_delay = 1000/speed;
}

void moveByAngle() {
  for (int i=0; (i<100) && run; i++) moveCW();
  delay(dwell*1000);

  for (int j=100; (j>0) && run; j--) moveCCW();
  delay(dwell*1000);
}