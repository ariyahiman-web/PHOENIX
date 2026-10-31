PHNX — Project Documentation

Team Name: PHNX
Team Members: 
Fatima Habib Muzaffar
Ariyah Iman

1. Project Overview

PHOENIX is an ESP32-based robotic vehicle designed to demonstrate wireless control, embedded systems, and autonomous obstacle avoidance.

The robot combines a microcontroller, motor driver, geared DC motors, an ultrasonic distance sensor, and a servo motor to enable remote operation and obstacle detection.

2. Project Objectives

- Develop a functional ESP32-based robotic vehicle.
- Implement wireless control through a browser-based interface.
- Incorporate obstacle detection using an ultrasonic sensor.
- Demonstrate motor control and embedded programming.
- Integrate mechanical design, electronics, and software into one working system.

3. Hardware Components

* ESP32: Main microcontroller and wireless connectivity
* 30P ESP32 expansion board
* 4 × TT geared 3V DC motors: Locomotion
* L298N motor driver: Controls motor direction and speed
* Ultrasonic sensor: Measures distance to obstacles
* Servo motor: Rotates the ultrasonic sensor
* LDR: Detects ambient light
* LEDs: Visual indicators
* Buzzer: Audio indication
* 2 × 18650 battery holders: Power supply
* Rocker switch: Main power control
* Breadboard and wiring: Electrical connections
* two battery holders for two batteries each
* three LEDs
* three four pin push buttons
* three resistors
* one buzzer
* one LDR
* one resistor connected to the LDR
* jumper wires (male to male, female to female, male to female).

4. Mechanical Design

The chassis was designed using Autodesk Fusion 360 and manufactured through 3D printing.

The design accommodates the motors, electronics, battery holders, front-mounted ultrasonic sensor assembly, and power switch.

The printable components are provided in the `cad` directory.

A four wheeled car driven by four DC motors. 
The chassis of the car is a 3D printed custom design.
The motors and wheels are neatly arranged inside the chassis and held in place via double sided tape/super glue. 
The chassis is held together by custom made chassis holders and screws.
One battery set supplies the four motors whereas another battery set supplies the esp32.
Each battery set contains two 18650 3.7V lithium ion batteries. 
The two battery holders are placed at the rear end of the car and held in place via double sided tape. 
The ESP32 is mounted on top of its expansion board.
The ultrasonic sonar is attached on top of a servo motor at the other end of the 3D printed chassis.
The breadboard is placed adjacent to adjacent to the ESP32 expansion board and behind the servo motor.
Three red LEDs, a buzzer, three resistors, three four pin push buttons and and a light dependent resistor are connected to the breadboard.
All the connections between the components are made using male to female jumper wires, male to male jumper wires and female to female jumper wires.

5. Software and Technologies

- ESP32
- C++
- Arduino framework
- PlatformIO
- Wi-Fi communication
- HTML/CSS/JavaScript-based browser interface

6. Operating Principle

The ESP32 acts as the central controller of the robot.

It receives commands through a wireless browser interface and controls the motors through the L298N motor driver.

The ultrasonic sensor measures the distance to nearby objects. Its servo-mounted arrangement allows the sensor to scan different directions.

Additional electronic components, including the LDR, LEDs, and buzzer, provide supplementary functionality.
 
Features: The car is connected to a web browser via WiFi 
The web browser has buttons which can be clicked/tapped to allow the car to start moving, stop moving, travel left, right, backwards and forwards.
The web browser also contains an automatic obstacle avoiding mode.
When switched on, the obstacle avoiding mode allows the car to move away from obstacles upon detecting them.
The buzzer automatically switches on when an obstacle is detected by the ultrasonic sonar.
The car contains three LEDs which can be manually switched on and off using three four pin push buttons.
The light dependent resistor automatically switches the three LEDs on when it detects low light intensities.
The three LEDs are automatically switched off when the light dependent resistor detects high light intensities.
The three resistors ensure that the circuit is safe and excess current does not flow through it.
A toggle switch is connected to the battery holder that supplies the four motors.
When the toggle switch is turned on, the motor driver turns on and when it is turned off, the motor driver turns off. 

7. Motor Selection Justification

Four TT yellow geared DC motors were selected because they offer a practical balance of affordability, compact size, availability, and torque suitable for a small educational robotic vehicle.

Their geared mechanism reduces rotational speed while increasing usable torque, making them appropriate for driving the robot's wheels.

DC motors: The 6V 100 RPM DC Gear BO Motors operate at 6V, making them ideal for this project. They can easily be powered by batteries. Their speed can be controlled since they can operate with PWM control. They are also lightweight and easy to install using screws and bolts. 

The four-motor arrangement provides traction through all four wheels. The motors can be controlled through the L298N motor driver, allowing the robot to move and turn through differential motor control.

Servo motor: The servo Motor Micro SG90 allows continuous 360 degree rotation which enables the sonar mounted on top of it to detect obstacles at several different directions. The motor's speed and direction can be easily adjusted since it operates with PWM control. It also operates at 5V external power supply which can easily be provided by batteries or microcontrollers. 


8. Team Contributions

Ariyah Iman:

- Designed the robot chassis using Autodesk Fusion 360.
- Coordinated with instructors regarding technical assistance and project requirements.
- Contributed to the assembly and implementation of the robot.
- Assisted in bringing the hardware and software components together.
- Created GitHub Repository
 
Fatima Habib Muzaffar:

- Contributed to the assembly and implementation of the robot.
- Worked on the electronic wiring and software implementation using available instructional resources.
- Assisted in bringing the hardware and software components together.
- Assisted in writing the project documentation


9. Repository Structure

- `cad/` — Fusion 360 design and 3D-printable files.
- `firmware/` — ESP32 source code and PlatformIO configuration.
- `docs/` — Technical documentation.
- `media/` — Project photographs and media.

10. Demonstration

YouTube demonstration video: https://youtube.com/shorts/UpryfAMNAxc11. Connections:

Motor wires and motor driver:
The two red wires of the two motors on the right to output-3 of the motor driver.
The two black wires of the two motors on the right to output-4 of the motor driver.
The two black wires of the two motors on the left to output-1 of the motor driver.
The two red wires of the two motors on the left to output-2 of the motor driver.

Motor driver and ESP32: 
IN1 to GPIO14.
IN2 to GPIO27.
IN3 to GPIO26.
IN4 to GPIO25.
ENA to GPIO33.
ENB to GPIO32.

Ultrasonic sensor and ESP32:
VCC to 5V.
TRIG to GPIO4.
ECHO to GPIO5.
GND terminal to GND pin.

Servo motor and ESP32:
Yellow wire to GPIO15.
Red wire to 5V pin.
Brown wire to GND.

Battery holder and motor driver:
positive wire to 12V IN.
negative wire to GND.

Battery holder and esp32:
positive wire to 5V.
negative wire to GND.

Buzzer and ESP32:
positive terminal to D23.
negative terminal to GND.

Buttons and ESP32:
Button-1 to GPIO16 and GND.
Button-2 to GPIO17 and GND.
Button-3 to GPIO18 and GND. 

LEDs and ESP32:
LED-1 to GPIO22.
LED-2 to GPIO19.
LED-3 to GPIO21.

LDR and ESP32:
one end of LDR to GPIO34.
opposite end of LDR to GND.

12. Setup instructions:

Attach the motors to the chassis using double sided tape or screws and bolts.

Attach the wheels to the motors.

Place the motor driver next to the motors and make the appropriate connections.

Attach the upper chassis to the lower chassis using screws and bolts.

Place the remaining components on top of the upper chassis and make the appropriate connections. 

Copy the code to Platformio extension or arduino IDE and upload it to the esp32.

Connect your device to the esp32's wifi network and enter the IP address in the web browser.

13. Why it follows all the rules:

Mechanical and actuation:
1. Even though it is web operated, it can still move about automatically when the automatic obstacle avoiding mode is switched on. It also has other autonomous features.
2. The DC motors are used to allow the wheels to move. The mini servo motor allows the ultrasonic sensor to rotate and enables it to detect obstacles. 
3. We will design a basic model of the project using fusion and upload it to GitHub.
4. We will make the project as close to the model as possible.

Embedded System+IoT:
1. The code will enable the robot to both be controlled remotely via a web browser and also automatically. Moreover, it automatically switches the LEDs on when the light intensity is low. And it automatically starts a buzzer when it encounters an obstacle. The servo motor also rotates automatically. 
2. The car will have its own power source.
3. Ultrasonic sensors will detect obstacles whereas an LDR detects light intensity.
4. The car has three buttons which can be used to turn the LEDs on and off. 
5. The car will be powered by batteries.
6. A buzzer starts the moment the car detects an obstacle and LEDs turn on the moment the light intensity detected by the LDR is low.
7. The entire car can be controlled via a web browser.

Rules and bans:
1. This project does not contain any sharp objects/weapons.
2. The power source has a maximum voltage of 7.4 V.



