# Arduino VT100 Radar

A radar visualization system built using Arduino UNO.

## Features

- HC-SR04 ultrasonic distance measurement
- Servo based scanning
- Polar coordinate visualization
- Cartesian coordinate visualization
- VT100 terminal graphics
- 16x2 I2C LCD output
- Buzzer proximity alert


## Hardware

- Arduino UNO
- HC-SR04 ultrasonic sensor
- SG90 servo motor
- 16x2 I2C LCD
- Buzzer


## Libraries Used

- Servo
- LiquidCrystal_I2C
- VT100

VT100 library:
https://github.com/YOUR_USERNAME/VT100-Arduino


## Working Principle

The ultrasonic sensor measures distance:

distance = time × speed of sound / 2


The servo scans from 0° to 180°.

The measured polar coordinates:

(r, θ)

are converted into Cartesian coordinates:

x = r cos(θ)

y = r sin(θ)


The results are displayed on:
- Serial VT100 terminal
- LCD