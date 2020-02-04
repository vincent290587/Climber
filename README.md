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
- The slope is sniffed in Zwift TCP packets directly (preferred, but only works with Zwift...),  
- or it can be obtained from the game ANT+ packets (only works in simulation mode and not in ERG...).

So... pick your poison !


## Incline achievement

The current bike front wheel hub's height is computed using a ToF infrared laser sensor: the VL53L1x.  
These measurement are input in a Kalman filter to get a better dynamic estimate of the bike's incline.  
A VNH5019 is then PWMed to drive the linear actuator.

# Hardware

- MCU: nRF52840 (BLE and ANT+), in the form of the PCA10059 (USB dongle) from Nordic
- Incline detection: ToF sensor VL53L1x (Pololu)
- Motor controller: VNH5019

# How to use it ?

## Hardware connections

The pinut that is used is all defined in the file named `custom_board_v1.h`.

## Pre-requisites

You need a ready installed Nordic SDK V15.3 with an equally installed S340 (findable at thisisant.com).
You also need to place the correct ANT / ANT+ keys in your freshly installed SDK ;-)

## Full simulation

Yes, the complete system has its own simulator where you can modify settings and look at amazing plots !

!! You need to be using Linux (or WSL) !!

### Building the simulator

- Go to the TDD folder, and type `cmake ..`
- Then build using `make`
- Run the program `./Climber`

The results are output in a file called `simu.txt`, which you can easily use in a plotting software. (KST plot is warmly recommended !!)

## Building the software

- Go to the compil/s340 folder
- Create a `Makefile.local` file with the path to your SDK and the port used for uploading software to the dongle (You can get inspiration from Makefile.local.sample)
- By default the software will get slope information from a second dongle using BLE, but you can also choose to use the ANT+ packets, and to do so you will have to uncomment the line `#CFLAGS += -DANT_STACK_SUPPORT_REQD`. If you are not interested in getting the logs, you might even as well no use BLE at all, and to do so, comment `CFLAGS += -DBLE_STACK_SUPPORT_REQD`.
- Run your favorite `make` command
- Plug your dongle in, and press the little side button to put it in bootloader mode
- Find the name of the dongle port, and make sure it matches the port name given in your `Makefile.local`
- Type `make dfu_softdevice`, which will load the S340 softdevice
- Then type `make dfu`, which will upload the software

The dongle will reboot automagically and it should be running !
