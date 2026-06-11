#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Rotary Encoder
#define CLK_PIN 2 //Supports interrupts, used for counting encoder ticks
#define DT_PIN 3 //Supports interrupts, used for counting encoder ticks
#define SW_PIN 4

//SSD1306 OLED (I2C mode)
#define SDA_PIN A4
#define SCL_PIN A5

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//For measuring encoder position
volatile int count = 0;
int currCLK;
int prevCLK;

void setupEncoder();
void readEncoder();
void setupOLED();

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");
  
  delay(500);
  setupEncoder();
  delay(500);
  setupOLED();

  Serial.println("Setup complete");
}

void loop() {
  //Check for encoder rotation
  currCLK = digitalRead(CLK_PIN);
  if (currCLK != prevCLK) readEncoder;
  prevCLK = currCLK;

  //Check for encoder button press
  if(digitalRead(SW_PIN) == LOW) {
    Serial.println("Button pressed");
    delay(200); //for debouncing
  }


}

void setupEncoder() {
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), readEncoder, CHANGE);
  prevCLK = digitalRead(CLK_PIN);
  
  Serial.println("Encoder pins setup complete");
}

void readEncoder() {
  if(digitalRead(CLK_PIN) == digitalRead(DT_PIN)) count++;
  else count--;
  Serial.println("Encoder value changed! Count = " + count);
}

void setupOLED(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  Serial.println("OLED setup complete");
}