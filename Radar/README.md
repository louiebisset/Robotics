# Ultrasonic Radar (Arduino Uno)

A servo sweeps an HC-SR04 ultrasonic sensor through 150 degrees. The Arduino sends angle and distance readings to a laptop, where a Python script draws a live radar-style map.

<!-- Add a photo or GIF here: ![Radar demo](docs/demo.gif) -->

## Hardware

- Arduino Uno R3 (Elegoo)
- HC-SR04 ultrasonic sensor
- SG90 servo
- Jumper wires, breadboard

## Wiring

| Part | Pin | Uno pin |
|---|---|---|
| HC-SR04 | VCC | 5V |
| HC-SR04 | GND | GND |
| HC-SR04 | Trig | D9 |
| HC-SR04 | Echo | D10 |
| SG90 | Red (power) | 5V |
| SG90 | Brown (ground) | GND |
| SG90 | Orange (signal) | D6 |

## How it works

1. The servo moves in 2 degree steps between 15 and 165 degrees.
2. At each step the sensor sends a pulse and times the echo. Distance = echo time x 0.0343 cm/us / 2.
3. The median of three readings is sent as `angle,distance`.
4. The Python script plots each reading on a polar chart and replaces old points as the sweep repeats.

## Next steps

- [e.g. swap the servo for the stepper motor for a wider sweep]
- [e.g. smooth readings with a moving average]
