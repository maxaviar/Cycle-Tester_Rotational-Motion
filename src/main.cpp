#include "common.h"
#include "display.h"
#include "encoder.h"
#include "stepper.h"

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");
  Wire.end();
  delay(500);
  
  // Manually unclog i2c bus
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
  displayCount();
  adjustSpeedAndAngle();

  if(interruptFlag) {
    interruptFlag = false;

    int32_t new_position = ss.getEncoderPosition();
    bool currButtonState = !ss.digitalRead(SS_SWITCH);
    bool clockwiseTurn = (new_position > encoder_position);

    if (currButtonState && !lastButtonState) {
      readButton();
      delay(200);
    }

    // did we move around
    if (encoder_position != new_position) {
      encoder_position = new_position;      // and save for next round
      readDial(clockwiseTurn);
    }
    lastButtonState = currButtonState;
  }
  
  while(settings.run && !interruptFlag) {
    moveByAngle();
    //Turn off stepper
    if (interruptFlag) {
      analogWrite(3, 0);
      analogWrite(11, 0); 
    }
  }

  // don't overwhelm serial port
  delay(50);
}