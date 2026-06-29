# Cycle Tester (Rotational Motion)
This project implements a rotational motion mechanism to test the cleaning life of opthalmic devices using an Arduino microcontroller. It controls the speed, dwell, and rotational angle of a stepper motor connected to a gearbox and displays the number of cycles on an OLED screen as it runs.

## Features
- Configurable speed, dwell, and rotation angle.
- Information displayed on OLED and navigation is controlled by rotary encoder (I2C interface).
- Hardware interrupts used to stop motor at any point in testing.

## Hardware
- Arduino R4 Minima
- Analog Devices QSH5718 Bipolar Stepper Motor (57mm, 2.8A, 1.26Nm)
- DFRobot DRI0043 Stepper Controller/Driver
- Adafruit 326 OLED
- Adafruit I2C QT Rotary Encoder
- Mean Well MPM-30-5ST AC/DC Converter (5V, 30W)
- Canaduino 26558 DC/DC Converter (5 to 30Vin, 1 to 30Vout, 300W)
- Sanyo Denki 109R0805F401 Axial Fan (80x25mm)
- Adafruit 3368 Axial Fan (30x8mm)

## Software
This project is built using https://platformio.org/ with the Arduino framework.

### Environment Configuration
- **Platform:** renesas-ra
- **Board:** Arduino UNO R4 Minima
- **Framework:** Arduino
- **Serial Monitor Baud Rate:** 9600

### Dependencies
The following libraries are automatically installed via PlatformIO:

- Adafruit SSD1306 (OLED display driver)
- Adafruit SH110X (OLED display support)
- Adafruit Seesaw Library (I2C peripheral driver)

### PlatformIO Setup
1. Install PlatformIO:
   - VS Code Extension: *PlatformIO IDE* (recommended)
   - Or CLI:  
     ```bash
     pip install platformio
     ```

2. Clone the repository:
   ```bash
   git clone https://github.com/maxaviar/Cycle-Tester_Rotational-Motion.git
   cd Cycle-Tester_Rotational-Motion

## Wiring
|     Component    | MCU Pin |         Other Pins         |
|------------------|---------|----------------------------|
|     OLED VCC     |   3.3V  |----------------------------|
|     OLED GND     |   GND   |----------------------------|
|     OLED SDA     |   A4    |----------------------------|
|     OLED SCK     |   A5    |----------------------------|
|    Encoder Vin   |   5V    |----------------------------|
|    Encoder GND   |   GND   |----------------------------|
|    Encoder SDA   |   A4    |----------------------------|
|    Encoder SCL   |   A5    |----------------------------|
|    Encoder INT   |   D2    |----------------------------|
|     Stepper A    |---------|         DRI0043 A+         |
|     Stepper A-   |---------|         DRI0043 A-         |
|     Stepper B    |---------|         DRI0043 B+         |
|     Stepper B-   |---------|         DRI0043 B-         |
|    DRI0043 DIR+  |   5V    |----------------------------|
|    DRI0043 PUL+  |   5V    |----------------------------|
|    DRI0043 DIR-  |   D11   |----------------------------|
|    DRI0043 DIR+  |   D12   |----------------------------|
|    DRI0043 VCC   |---------|      DC/DC +Vout(12V)      |
|    DRI0043 GND   |   GND   |----------------------------|
|    AC/DC +Vout   |---------|         DC/DC +Vin         |
|    AC/DC -Vout   |   GND   |----------------------------|
|    DC/DC +Vin    |---------|         AC/DC +Vout        |
|    DC/DC -Vin    |   GND   |----------------------------|
| DC/DC +Vout(12V) |   Vin   |         DRI0043 VCC        |
|    DC/DC -Vout   |   GND   |----------------------------|
|   Both Fans +5V  |   +5V   |----------------------------|
|   Both Fans GND  |   GND   |----------------------------|
