/*
 * custom_board_v1.h
 *
 *  Created on: 30 nov. 2018
 *      Author: Vincent
 */

#ifndef CUSTOM_BOARD_V1_H_
#define CUSTOM_BOARD_V1_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LED definitions for PCA10059
// Each LED color is considered a separate LED
#define LEDS_NUMBER    4

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define LED_1          LED1_G
#define LED_2          LED2_R
#define LED_3          LED2_G
#define LED_4          LED2_B

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

// There is only one button for the application
// as the second button is used for a RESET.
#define BUTTONS_NUMBER 1

#define BUTTON_1       NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }

#define BSP_BUTTON_0   BUTTON_1

#define BSP_SELF_PINRESET_PIN NRF_GPIO_PIN_MAP(0,19)

//#define HAS_LSM6

#define SDA_PIN_NUMBER   NRF_GPIO_PIN_MAP(0, 17)
#define SCL_PIN_NUMBER   NRF_GPIO_PIN_MAP(0, 15)

#define LSM6_INT1        NRF_GPIO_PIN_MAP(0, 23)
#define LSM6_INT2        NRF_GPIO_PIN_MAP(0, 26)

#define VL53_INT         NRF_GPIO_PIN_MAP(0, 13)

#define VNH_INA1         NRF_GPIO_PIN_MAP(1, 13)
#define VNH_INB1         NRF_GPIO_PIN_MAP(1, 15)
#define VNH_PWM1         NRF_GPIO_PIN_MAP(1, 10)
#define VNH_CS1          NRF_GPIO_PIN_MAP(0, 29)
#define VNH_DIAG1        NRF_GPIO_PIN_MAP(0, 31)

#define HWFC           true

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_BOARD_V1_H_ */
