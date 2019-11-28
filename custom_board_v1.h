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

// LEDs definitions
#define LEDS_NUMBER    1

//#define LED_START      14
#define LED_1            NRF_GPIO_PIN_MAP(0, 26)
//#define LED_STOP       14

#define LEDS_ACTIVE_STATE 1

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 1

//#define BUTTON_START   17
#define BUTTON_1       NRF_GPIO_PIN_MAP(0, 25)
//#define BUTTON_STOP    19
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }

#define BSP_BUTTON_0   BUTTON_1

#define SDA_PIN_NUMBER   NRF_GPIO_PIN_MAP(0, 13)
#define SCL_PIN_NUMBER   NRF_GPIO_PIN_MAP(0, 11)

#define GYR_INT1         NRF_GPIO_PIN_MAP(0, 30)
#define GYR_INT2         NRF_GPIO_PIN_MAP(0, 31)

#define LIS_INT1         NRF_GPIO_PIN_MAP(0, 15)
#define LIS_INT2         NRF_GPIO_PIN_MAP(0, 17)

#define HV_EN            NRF_GPIO_PIN_MAP(0, 20)

#define N_VCCINT_EN      NRF_GPIO_PIN_MAP(0, 19)

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_BOARD_V1_H_ */
