[![CircleCI](https://circleci.com/gh/vincent290587/Climber/tree/master.svg?style=svg)](https://circleci.com/gh/vincent290587/Climber/tree/master)

# Project

This is yet another project of an indoor-trainer grade simulator.  
The concept was inspired from OpenGradeSim, but with different strategy / parts !

- [x] Sensors readout
- [x] ANT FE-C Slope sniffing
- [x] Kalman filtering
- [x] Actuator command
- [x] User params flash saving
- [ ] ANT FE-C Power / speed to slope conversion

# Concept

The ANT+ target incline is sniffed using the ANT+ protocol, it is then compared to the filtered measured front wheel hub height.  
Two 3D printed parts allow to mount securely the actuator to the bike's fork, and to the ground.
You know the rest ;-)

## Slope detection

The current in-game slope is sniffed using an ANT+ continuous scanner directly with the nRF52 radio, which makes it available only the in non-workout mode of Zwift.  


## Incline achievement

The current bike front wheel hub's height is computed using a Tof infrared laser sensor: the VL53L1x.  
These measurement are input in a Kalman filter to get a better dynamic estimate of the bike's incline.  
A VNH5019 is then commanded to drive the linear actuator.

# Hardware

- MCU: nRF52832 or nRF52840 (BLE and ANT+)
- Incline detection: ToF sensor VL53L1x (Pololu)
- Motor controller: VNH5019
- Optional IMU: LSM6DS33 (optional because the VNH5019 is precise almost to the mm and hence sufficient !)