[![CircleCI](https://circleci.com/gh/vincent290587/Climber/tree/master.svg?style=svg)](https://circleci.com/gh/vincent290587/Climber/tree/master)

# Project

This is another project of an indoor-trainer grade simulator.  
It is meant to be used with Zwift, but could be used with any other similar program or game.  
The concept was inspired by the Wahoo Kickr Climb, but with different strategy / parts !

- [x] Sensors readout
- [x] ANT+ FE-C Slope sniffing
- [x] Kalman filtering
- [x] Actuator command
- [x] User params flash saving
- [x] Zwift TCP packets sniffing for ERG (or any) mode slope sniffing.

# Concept

The ANT+ target incline is sniffed using the ANT+ protocol, it is then compared to the filtered measured front wheel hub height.  
Two 3D printed parts allow to mount securely the actuator to the bike's fork, and to the ground.  
You know the rest ;-)

## Slope detection

The software can work in 2 modes:  
- The slope is sniffed in Zwift TCP packets directly (preferred !),  
- or it can be obtained from the game ANT+ packets (not in ERG mode).


## Incline achievement

The current bike front wheel hub's height is computed using a ToF infrared laser sensor: the VL53L1x.  
These measurement are input in a Kalman filter to get a better dynamic estimate of the bike's incline.  
A VNH5019 is then PWMed to drive the linear actuator.

# Hardware

- MCU: nRF52832 or nRF52840 (BLE and ANT+)
- Incline detection: ToF sensor VL53L1x (Pololu)
- Motor controller: VNH5019