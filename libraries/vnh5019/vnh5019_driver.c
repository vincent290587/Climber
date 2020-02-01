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
#include "parameters.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_pwm.h"
#include "segger_wrapper.h"
#include "vnh5019_driver.h"

#define SAMPLES_IN_BUFFER 1

#define PWM_FREQUENCY_HZ     10000UL

static nrf_drv_pwm_t m_pwm1 = NRF_DRV_PWM_INSTANCE(1);

static nrf_saadc_value_t m_buffer_pool[2][SAMPLES_IN_BUFFER];
static int16_t m_adc_value = 0;

static eVNH5019State m_state = eVNH5019StateCoasting;
static int16_t m_duty_cycle = 0;
static int16_t m_new_pwm_data = 0;

static volatile bool ready_flag;            // A flag indicating PWM status.

static void _on_pwm_event(nrf_drv_pwm_evt_type_t event_type)
{
    if (event_type == NRF_DRV_PWM_EVT_FINISHED)
    {
		ready_flag = true;
    }
}

static void _pwm_configure(void) {

	nrf_gpio_pin_clear(VNH_PWM1);

	nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            VNH_PWM1, //  | NRF_DRV_PWM_PIN_INVERTED
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 100,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };

    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm1, &config0, _on_pwm_event));
}

/**
 * Returns the true PWM frequency applied
 */
static uint16_t _pwm_signal_set(uint16_t duty_cycle, uint8_t force) {

	static uint16_t duty_cycle_prev = 0;

	if (duty_cycle > 100) {
		duty_cycle = 100;
	}

	if (duty_cycle > 6 &&
			(duty_cycle != duty_cycle_prev || force)) {

		duty_cycle &= ~0b1111;
		duty_cycle |= 0b10000;

		if (duty_cycle > 100) {
			duty_cycle = 100;
		}

		if (duty_cycle == 100 && duty_cycle_prev == 0) {
			duty_cycle = 70;
		}

		m_new_pwm_data = 1;

		LOG_INFO("PWM signal duty cycle: %lu", duty_cycle);

	} else if (duty_cycle <= 6) {

		m_new_pwm_data = 1;

		LOG_INFO("PWM signal frequency: %u OFF", duty_cycle);

        duty_cycle = 0;

	} else if (duty_cycle == duty_cycle_prev) {


	}

	duty_cycle_prev = duty_cycle;

	return duty_cycle;
}


static void _saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	W_SYSVIEW_RecordEnterISR();

	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		ret_code_t err_code;

		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);

		m_adc_value = 0;
		for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
		{
			m_adc_value += (int16_t)p_event->data.done.p_buffer[i];
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
	nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(VNH_CS1);

	//Configure SAADC channel
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;                              //Set internal reference of fixed 0.6 volts
	channel_config.gain = NRF_SAADC_GAIN1_6;                                                //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channel_config.acq_time = NRF_SAADC_ACQTIME_40US;                                     //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
	channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;                                    //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	channel_config.pin_p = VNH_CS1;                                                       //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
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
	nrf_gpio_pin_clear(VNH_PWM1);

	int err_code = nrf_drv_ppi_init();
	APP_ERROR_CHECK(err_code);

	_saadc_init();

	_pwm_configure();
}

int16_t vnh5019_driver__getM1_duty(void) {

	return m_duty_cycle;
}

uint16_t vnh5019_driver__setM1_duty(int16_t s_duty_cycle, uint8_t force)
{
	unsigned char reverse = 0;
	static unsigned char reverse_prev = 0;
	uint16_t duty_cycle = 0;

	if (s_duty_cycle < 0)
	{
		s_duty_cycle = -s_duty_cycle;
		reverse = 1;  // Preserve the direction
	}

	if (s_duty_cycle > 100)
	{
		duty_cycle = 100;
	} else {
		duty_cycle = s_duty_cycle;
	}

	m_state = eVNH5019StateDriving;

	// set PWM signal
	duty_cycle = _pwm_signal_set(duty_cycle, reverse == reverse_prev ? force : 1);
	if (!duty_cycle) {

		// forced coastin
		m_state = eVNH5019StateCoasting;
	}

	gpio_set(LED_2);
	gpio_set(LED_3);
	gpio_set(LED_4);

	reverse_prev = reverse;

	if (duty_cycle == 0)
	{
		digitalWrite(VNH_INA1, LOW);   // Make the motor coast no
		digitalWrite(VNH_INB1, LOW);   // matter which direction it is spinning.

		// save speed
		m_duty_cycle = 0;

		gpio_clear(LED_2);
	}
	else if (reverse)
	{
		digitalWrite(VNH_INA1, LOW);
		digitalWrite(VNH_INB1, HIGH);

		// save speed
		m_duty_cycle = -duty_cycle;

		gpio_clear(LED_3);
	}
	else
	{
		digitalWrite(VNH_INA1, HIGH);
		digitalWrite(VNH_INB1, LOW);

		// save speed
		m_duty_cycle =  duty_cycle;

		gpio_clear(LED_4);
	}

	return duty_cycle;
}

void vnh5019_driver__setM1Brake(uint16_t brake)
{
	digitalWrite(VNH_INA1, LOW);
	digitalWrite(VNH_INB1, LOW);

	m_state = eVNH5019StateBraking;

	// set PWM signal
	_pwm_signal_set(brake, 1);
}

// Return motor 1 current value in milliamps.
int32_t vnh5019_driver__getM1CurrentMilliamps(void)
{
	// 144 mV per 1000mA
	// for 10 bit SAADC with gain 1/6
	int32_t voltage_mv = ((int32_t)m_adc_value * 6 * 1000) / (1 << 10);
	return (voltage_mv * 1000) / 144;

}

// Return error status for motor 1
uint32_t vnh5019_driver__getM1Fault(void)
{
	return !digitalRead(VNH_DIAG1);
}

void vnh5019_driver__tasks(void) {

	static uint32_t last_up = 0;

	// handle timeout
	if (m_new_pwm_data &&
		!ready_flag &&
		millis() - last_up > 700) {

		LOG_WARNING("PWM timeout");
		ready_flag = true;
	}

	// handle APP_PWM driver
	if (ready_flag &&
		m_new_pwm_data) {

        ready_flag = false;

		// This array cannot be allocated on stack (hence "static") and it must
		// be in RAM (hence no "const", though its content is not changed).
		static nrf_pwm_values_common_t /*const*/ seq1_values[1] = { 0 };
		nrf_pwm_sequence_t const seq1 =
		{
			.values.p_common = seq1_values,
			.length          = 1,
			.repeats         = 0,
			.end_delay       = 0,
		};

		seq1_values[0] = m_duty_cycle | 0x8000;

		uint32_t error = nrf_drv_pwm_simple_playback(&m_pwm1, &seq1, 1, NRF_DRV_PWM_FLAG_LOOP);
		APP_ERROR_CHECK(error);

		LOG_WARNING("PWM set");

		// clear flags
		m_new_pwm_data = 0;
		last_up = millis();
	} else {

		// start sampling SAADC
		nrf_drv_saadc_sample();
	}

}
