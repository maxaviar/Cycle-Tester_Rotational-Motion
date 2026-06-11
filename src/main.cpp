#include <Arduino.h>
#include <HardwareSerial.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

//Rotary Encoder
#define CLK_PIN 2 //Supports interrupts, used for counting encoder ticks
#define DT_PIN 3 //Supports interrupts, used for counting encoder ticks
#define SW_PIN 4

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

//For measuring encoder position
volatile int count = 0;
volatile int alt_count = 0;

bool count_changed = false;
bool alt_counter = false;

void setupEncoder();
void readEncoder();

void setupOLED();
void displayCount();

void setupStepper();
void moveCW();
void moveCCW();

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
  //Check for encoder rotation
  if (count_changed) {
    Serial.print("Encoder value changed! Count = ");
    Serial.println(count);
    displayCount();
    count_changed = false;
  }

  //Check for encoder button press
  if(digitalRead(SW_PIN) == LOW) {
    Serial.println("Button pressed");
    alt_counter = !alt_counter;
    delay(200); //for debouncing
  }
  
  /* Just for testing out rotating the stepper back and forth
  for(int i=0;i<10;i++) moveCW();
  for(int j=0;j<10;j++) moveCCW();
  */
}

void setupEncoder() {
  pinMode(CLK_PIN, INPUT_PULLUP); //find out what input vs. input_pullup actually does
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), readEncoder, CHANGE);
  
  Serial.println("Encoder pins setup complete");
}

void readEncoder() {
  if (!alt_counter) {
    if(digitalRead(CLK_PIN) == digitalRead(DT_PIN)) count++;
    else count--;
  }
  if (alt_counter) {
    if(digitalRead(CLK_PIN) == digitalRead(DT_PIN)) alt_count++;
    else alt_count--;
  }
  count_changed = true;
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
  display.setCursor(5,20);
  display.print("Count = ");
  display.println(count);

  display.setCursor(5,30);
  display.print("Alt. Count = ");
  display.println(alt_count);

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
  digitalWrite(9, LOW);  //ENABLE CH A
  digitalWrite(8, HIGH); //DISABLE CH B

  digitalWrite(12, HIGH);   //Sets direction of CH A
  analogWrite(3, 255);   //Moves CH A
  Serial.println("Moving CH A pt1");

  delay(30);
  
  digitalWrite(9, HIGH);  //DISABLE CH A
  digitalWrite(8, LOW); //ENABLE CH B

  digitalWrite(13, LOW);   //Sets direction of CH B
  analogWrite(11, 255);   //Moves CH B
  Serial.println("Moving CH B pt2");

  delay(30);
  
  digitalWrite(9, LOW);  //ENABLE CH A
  digitalWrite(8, HIGH); //DISABLE CH B

  digitalWrite(12, LOW);   //Sets direction of CH A
  analogWrite(3, 255);   //Moves CH A
  Serial.println("Moving CH A pt2");

  delay(30);
    
  digitalWrite(9, HIGH);  //DISABLE CH A
  digitalWrite(8, LOW); //ENABLE CH B

  digitalWrite(13, HIGH);   //Sets direction of CH B
  analogWrite(11, 255);   //Moves CH B
  Serial.println("Moving CH B pt2");
  
  delay(30);
}

void moveCCW() {
  digitalWrite(9, LOW);  //ENABLE CH A
  digitalWrite(8, HIGH); //DISABLE CH B

  digitalWrite(12, HIGH);   //Sets direction of CH A
  analogWrite(3, 255);   //Moves CH A
  
  delay(30);
  
  digitalWrite(9, HIGH);  //DISABLE CH A
  digitalWrite(8, LOW); //ENABLE CH B

  digitalWrite(13, HIGH);   //Sets direction of CH B
  analogWrite(11, 255);   //Moves CH B
  
  delay(30);
  
  digitalWrite(9, LOW);  //ENABLE CH A
  digitalWrite(8, HIGH); //DISABLE CH B

  digitalWrite(12, LOW);   //Sets direction of CH A
  analogWrite(3, 255);   //Moves CH A
  
  delay(30);
    
  digitalWrite(9, HIGH);  //DISABLE CH A
  digitalWrite(8, LOW); //ENABLE CH B

  digitalWrite(13, LOW);   //Sets direction of CH B
  analogWrite(11, 255);   //Moves CH B
  
  delay(30);
}