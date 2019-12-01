VL53L1X_API_xxx zip file content:
- 	"core" directory contains the driver files
- 	"platform" directory contains the platform files.
	Note: 	The user has to implement the I2C functions prototyped in the vl53l1_platform.c file accordingly.
			For the STM32 target the user may download the X-CUBE-53L1A1 code on ST.COM and inspire on the implementation of the I2C functions developed
			for stm32 ODE in the vl53l1_platform.c file.
-   VL53L1X API user manual (UM2356)
- 	Release Notes