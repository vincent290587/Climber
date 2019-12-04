/*
 * vnh5019_driver.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "gpio.h"
#include "boards.h"
#include "helper.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
#include "segger_wrapper.h"
#include "vnh5019_driver.h"


static nrf_drv_timer_t timer = NRF_DRV_TIMER_INSTANCE(2);

void timer_dummy_handler(nrf_timer_event_t event_type, void * p_context) { }

static void _timer_configure(uint16_t freq_hz) {

    uint32_t compare_evt_addr;
    uint32_t gpiote_task_addr;
    nrf_ppi_channel_t ppi_channel;
    ret_code_t err_code;
    nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);

    err_code = nrf_drv_gpiote_out_init(VNH_PWM1, &config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(&timer, (nrf_timer_cc_channel_t)0, 200 * 1000UL, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);

    compare_evt_addr = nrf_drv_timer_event_address_get(&timer, NRF_TIMER_EVENT_COMPARE0);
    gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(VNH_PWM1);

    err_code = nrf_drv_ppi_channel_assign(ppi_channel, compare_evt_addr, gpiote_task_addr);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_task_enable(VNH_PWM1);

    // Enable timer
    nrf_drv_timer_enable(&timer);
}

static void _pwm_signal_set(uint16_t freq_hz) {

	if (freq_hz > 19500) freq_hz = 19500;

	uint32_t div = 125000UL / freq_hz / 2;

    // Enable timer
    nrf_drv_timer_disable(&timer);

    LOG_INFO("PWM signal frequency: %u (div = %lu)", freq_hz, div);

    if (freq_hz > 5600) {
    	nrf_drv_timer_extended_compare(&timer, (nrf_timer_cc_channel_t)0, div, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

    	// Enable timer
    	nrf_drv_timer_enable(&timer);
    } else {
    	nrf_gpio_pin_set(VNH_PWM1);
    }
}

void vnh5019_driver__init(void) {

	nrf_gpio_cfg_output(VNH_INA1);
	nrf_gpio_pin_clear(VNH_INA1);

	nrf_gpio_cfg_output(VNH_INB1);
	nrf_gpio_pin_clear(VNH_INB1);

	nrf_gpio_cfg_input(VNH_DIAG1, NRF_GPIO_PIN_PULLUP);

	nrf_gpio_cfg_output(VNH_PWM1);
	nrf_gpio_pin_set(VNH_PWM1);

    int err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&timer, &timer_cfg, timer_dummy_handler);
    APP_ERROR_CHECK(err_code);

	_timer_configure(0);
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
