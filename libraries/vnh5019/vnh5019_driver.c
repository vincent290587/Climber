/*
 * vnh5019_driver.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "gpio.h"
#include "boards.h"
#include "app_pwm.h"
#include "helper.h"
#include "segger_wrapper.h"
#include "vnh5019_driver.h"


APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag = false;            // A flag indicating PWM status.


static void _pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

static void _pwm_signal_set(uint16_t freq_hz) {

	if (freq_hz > 20000) freq_hz = 20000;

    app_pwm_disable(&PWM1);

	// uninit PWM
	app_pwm_uninit(&PWM1);

	// no frequency to generate
	if (!freq_hz) {
		nrf_gpio_pin_clear(VNH_PWM1);
		return;
	}

    /* 1-channel PWM, 0Hz, output on DK LED pins. */
	uint32_t period_50us = 1000000 / (50 * freq_hz);
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(period_50us * 50u, VNH_PWM1);

    /* Switch the polarity of the channel. */
    //pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    ready_flag = false;

    /* Initialize and enable PWM. */
    ret_code_t err_code;
    do {
    	err_code = app_pwm_init(&PWM1, &pwm1_cfg, _pwm_ready_callback);
    	APP_ERROR_CHECK(err_code);
    } while (err_code);

    app_pwm_enable(&PWM1);
}

void vnh5019_driver__init(void) {

	nrf_gpio_cfg_output(VNH_INA1);
	nrf_gpio_pin_clear(VNH_INA1);

	nrf_gpio_cfg_output(VNH_INB1);
	nrf_gpio_pin_clear(VNH_INB1);

	nrf_gpio_cfg_input(VNH_DIAG1, NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(VNH_PWM1);
	nrf_gpio_pin_clear(VNH_PWM1);
}

void vnh5019_driver__setM1Speed(int16_t speed_mm_s)
{
	unsigned char reverse = 0;
	uint16_t freq_hz = 0;

	if (speed_mm_s < 0)
	{
		speed_mm_s = -speed_mm_s;
		reverse = 1;  // Preserve the direction
	}

	if (speed_mm_s < 16)
	{
		freq_hz = (speed_mm_s * 1000 * 20) / 16;
	} else {
		freq_hz = 20000;
	}

	// set PWM signal
	_pwm_signal_set(freq_hz);

	if (speed_mm_s == 0)
	{
		digitalWrite(VNH_INA1, LOW);   // Make the motor coast no
		digitalWrite(VNH_INB1, LOW);   // matter which direction it is spinning.
	}
	else if (reverse)
	{
		digitalWrite(VNH_INA1, LOW);
		digitalWrite(VNH_INB1, HIGH);
	}
	else
	{
		digitalWrite(VNH_INA1, HIGH);
		digitalWrite(VNH_INB1, LOW);
	}
}

void vnh5019_driver__setM1Brake(uint16_t brake)
{
	digitalWrite(VNH_INA1, LOW);
	digitalWrite(VNH_INB1, LOW);

	// set PWM signal
	_pwm_signal_set(brake);
}

// Return motor 1 current value in milliamps.
uint32_t vnh5019_driver__getM1CurrentMilliamps(void)
{
	// 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
	return 0; // analogRead(VNH_CS1) * 34;
}

// Return error status for motor 1
uint32_t vnh5019_driver__getM1Fault(void)
{
	return !digitalRead(VNH_DIAG1);
}
