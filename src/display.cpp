#include "common.h"
#include "display.h"

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

  if (settings.page_number == 0){
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.print("Speed: ");
    display.print(settings.speed);
    display.print(" step/sec");
    if (settings.position == SPEED_POS) display.println(" *");

    display.setCursor(0,10);
    display.print("Dwell: ");
    display.print(settings.dwell);
    display.print(" sec");
    if (settings.position == DWELL_POS) display.println(" *");

    display.setCursor(0,20);
    display.print("Angle: ");
    display.print(settings.rotation_angle);
    display.print(" deg");
    if (settings.position == ANGLE_POS) display.println(" *");

    display.setCursor(0,30);
    display.print("Count = ");
    display.print(settings.count);
  
    display.setCursor(0,55);
    if (!settings.run){
      display.print("Page (1/2)");
      if (settings.position == PAGE_POS) display.println(" *");
    }
    else {
      display.println("Press button to stop");
    }
  }

  else {
    display.setCursor(0,20);
    display.print("Reset?");
    if (!settings.rst_count)
      display.print(" No");
    else
      display.print(" Yes");
    if (settings.position == COUNT_POS)
      display.println(" *");

    display.setCursor(0,30);
    display.print("Start? ");
    if (settings.start)
      display.print("Yes");
    else
      display.print("No");
    
    if (settings.position == START_POS)
      display.println(" *");

    display.setCursor(0,55);
    display.print("Page (2/2)");
    if (settings.position == PAGE_POS) display.println(" *");
  } 
  display.display();
}
