#include "common.h"
#include "encoder.h"

volatile bool interruptFlag = false;
int32_t encoder_position = 0;
bool lastButtonState = false;

Adafruit_seesaw ss;

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
  interruptFlag = true;
}

void readDial(bool clockwiseTurn) {
  if (settings.position == SPEED_POS) {
    if((clockwiseTurn) && (settings.speed < MAX_SPEED)) settings.speed++;
    else if ((!clockwiseTurn) && (settings.speed > MIN_SPEED)) settings.speed--;
  }
  else if (settings.position == DWELL_POS) { 
    if((clockwiseTurn) && (settings.dwell < MAX_DWELL)) settings.dwell+=DWELL_INCREMENT;
    else if ((!clockwiseTurn) && (settings.dwell > MIN_DWELL)) settings.dwell-=DWELL_INCREMENT;
  }
  else if (settings.position == ANGLE_POS){
    if((clockwiseTurn) && (settings.rotation_angle < MAX_ANGLE)) settings.rotation_angle += ANGLE_INCREMENT;
    else if ((!clockwiseTurn) && (settings.rotation_angle > MIN_ANGLE)) settings.rotation_angle -= ANGLE_INCREMENT;
  }
  else if (settings.position == PAGE_POS) {
    if(clockwiseTurn) settings.page_number = 1;
    else if (!clockwiseTurn) settings.page_number = 0;
  }
  else if (settings.position == COUNT_POS) {
    if(clockwiseTurn) settings.rst_count = true;
    else if (!clockwiseTurn) settings.rst_count = false;
  }
  else if (settings.position == START_POS){
    if(clockwiseTurn) settings.start = true;
    else if (!clockwiseTurn) settings.start = false;
  }
}

void readButton() {
  if (settings.run) {
      settings.run = false;
      settings.start = false;
      settings.position = 0;
    }
  else if ((settings.position == COUNT_POS) && settings.rst_count) {
    settings.rst_count = false;
    settings.count = 0;
    settings.position = 0;
    settings.page_number = 0;
  }
  else if ((settings.position == START_POS) && settings.start) {
    settings.run = true;
    settings.page_number = 0;
    delay(50);
  }
  else if (settings.page_number == 0) {
    settings.position = (settings.position+1) % 4;
  }
  else {
    settings.position = (settings.position+1) % 6;
    if (settings.position == 0) {
        settings.position = PAGE_POS;
    }
  }
}
