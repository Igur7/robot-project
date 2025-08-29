# robot-project
https://wokwi.com/projects/439749393075718145
Arduino Nano Robot Arm Project
📖 Description

This project is a DIY robotic arm with a gripper, designed in Autodesk Inventor and controlled by an Arduino Nano.
It features:
a 3D-printed base with a hexagonal ventilation/grid pattern,
multiple servo motors including a functional gripper,
joystick and potentiometer control,
buttons for recording and saving movments and reseting arm to its basic positon,
an external power supply for stable servo operation.
The project is intended for learning mechanics, electronics, and embedded programming through hands-on building.

🔩 Parts
Mechanical
3D-printed base plate with hexagonal cutouts (Inventor .ipt file included),
Arm links and servo holders,
Gripper mechanism (servo-driven),
Screws, spacers, and fasteners.
Electronic
Arduino Nano (ATmega328P),
External 5V power supply,
Servo motors (SG90 / MG996R, including one for the gripper),
2× Joystick module,
Push buttons,
potentiometer,
Breadboard and jumper wires.

⚡ Wiring / Connections
Arduino Nano Pins:
Joystick X → A0
Joystick Y → A1
Potentiometer → A2
int buttonRec = 3;   // guzik nagrywania
int buttonPlay = 4;  // guzik odtwarzania
int buttonReset = 5; // guzik resetu pozycji
Servo Base → D9
Servo Arm Joint 1 → D10
Servo Arm Joint 2 → D11
Servo Gripper → D12

Power:

Arduino Nano powered by USB or VIN (7–12V)
Servos powered by external 5V supply (⚠️ common GND with Arduino is required)
🛠️ Assembly Instructions
3D Print all mechanical parts (base, arm links, gripper, servo holders).
Mount servo motors in their holders and connect them to the arm joints.
Attach the gripper servo to the gripper mechanism.
Assemble the arm on the base plate with screws and spacers.
Wire up the joysticks, buttons, potentiometer, and servos according to the wiring scheme.
pload the Arduino sketch from the /code folder.
Test each control input (joystick axes, potentiometer, buttons) to confirm proper movement of arm joints and gripper.

🚀 Future Improvements

Add more degrees of freedom to the arm,

Enable wireless control (Bluetooth / Wi-Fi),


