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
#include "millis.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_saadc.h"
#include "segger_wrapper.h"
#include "vnh5019_driver.h"

#define SAMPLES_IN_BUFFER 1

static nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(2);
static nrf_saadc_value_t m_buffer_pool[2][SAMPLES_IN_BUFFER];
static int16_t m_adc_value = 0;

static eVNH5019State m_state = eVNH5019StateCoasting;
static int16_t m_speed_mm_s = 0;

void timer_dummy_handler(nrf_timer_event_t event_type, void * p_context) { }

static void _timer_configure(uint16_t freq_hz) {

	uint32_t compare_evt_addr;
	uint32_t gpiote_task_addr;
	nrf_ppi_channel_t ppi_channel;
	ret_code_t err_code;
	nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);

	err_code = nrf_drv_gpiote_out_init(VNH_PWM1, &config);
	APP_ERROR_CHECK(err_code);

	nrf_drv_timer_extended_compare(&m_timer,
			NRF_TIMER_CC_CHANNEL0,
			500 * 1000UL,
			NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
			false);

	err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
	APP_ERROR_CHECK(err_code);

	compare_evt_addr = nrf_drv_timer_event_address_get(&m_timer, NRF_TIMER_EVENT_COMPARE0);
	gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(VNH_PWM1);

	err_code = nrf_drv_ppi_channel_assign(ppi_channel, compare_evt_addr, gpiote_task_addr);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_ppi_channel_enable(ppi_channel);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_out_task_enable(VNH_PWM1);

}

/**
 * Returns the true PWM frequency applied
 */
static uint16_t _pwm_signal_set(uint16_t freq_hz) {

	if (freq_hz > 19500) freq_hz = 19500;

	// start sampling SAADC
	nrf_drv_saadc_sample();

	// Disable timer
	if (nrf_drv_timer_is_enabled(&m_timer))
		nrf_drv_timer_disable(&m_timer);

	if (freq_hz > 3000) {

		uint32_t div = 125000UL / freq_hz / 2;

		LOG_INFO("PWM signal frequency: %u (div = %lu)", freq_hz, div);

		nrf_drv_timer_extended_compare(&m_timer,
					NRF_TIMER_CC_CHANNEL0,
					div,
					NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
					false);

		// Enable timer
		nrf_drv_timer_enable(&m_timer);

		return freq_hz;
	} else {
		nrf_gpio_pin_set(VNH_PWM1);

		LOG_INFO("PWM signal frequency: %u OFF", freq_hz);

		m_state = eVNH5019StateCoasting;

		return 0;
	}

	return 0;
}


static void _saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	W_SYSVIEW_RecordEnterISR();

	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		ret_code_t err_code;

		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);

//		static uint32_t m_adc_evt_counter = 0;
//		LOG_DEBUG("SAADC event number: %d %lu", (int)m_adc_evt_counter, millis());
//		m_adc_evt_counter++;

		m_adc_value = 0;
		for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
		{
			m_adc_value += p_event->data.done.p_buffer[i];
		}

		m_adc_value /= SAMPLES_IN_BUFFER;
		LOG_DEBUG("SAADC value: %d", m_adc_value);
	}

	W_SYSVIEW_RecordExitISR();
}

static void _saadc_init(void)
{

	nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;

	ret_code_t err_code;
	nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_DIFFERENTIAL(5, 6); // TODO set pins

	//Configure SAADC channel
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;                              //Set internal reference of fixed 0.6 volts
	channel_config.gain = NRF_SAADC_GAIN1;                                                //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channel_config.acq_time = NRF_SAADC_ACQTIME_40US;                                     //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
	channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;                                    //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	channel_config.pin_p = NRF_SAADC_INPUT_AIN0;                                          //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
	channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;                                      //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
	channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;                              //Disable pullup resistor on the input pin
	channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;                              //Disable pulldown resistor on the input pin

	err_code = nrf_drv_saadc_init(&saadc_config, _saadc_callback);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_channel_init(0, &channel_config);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
	APP_ERROR_CHECK(err_code);

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
	err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_dummy_handler);
	APP_ERROR_CHECK(err_code);

	_saadc_init();

	_timer_configure(0);
}

int16_t vnh5019_driver__getM1Speed(void) {

	return m_speed_mm_s;
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

	m_state = eVNH5019StateDriving;

	// set PWM signal
	if (!_pwm_signal_set(freq_hz)) {

		// forced coasting
		speed_mm_s = 0;
	}

	// save speed
	m_speed_mm_s = speed_mm_s;

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

	m_state = eVNH5019StateBraking;

	// set PWM signal
	_pwm_signal_set(brake);
}

// Return motor 1 current value in milliamps.
int32_t vnh5019_driver__getM1CurrentMilliamps(void)
{
	if (m_state == eVNH5019StateDriving) {
		// 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
		// Here 1024 counts is 0.6 instead of 0.5: scale
		return m_adc_value * 34 * 6 / 5; // analogRead(VNH_CS1) * 34;
	}

	return 0;
}

// Return error status for motor 1
uint32_t vnh5019_driver__getM1Fault(void)
{
	return !digitalRead(VNH_DIAG1);
}
