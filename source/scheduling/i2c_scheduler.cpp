/*
 * i2c_scheduler.c
 *
 *  Created on: 10 déc. 2017
 *      Author: Vincent
 */

#include "boards.h"
#include "helper.h"
#include "i2c.h"
#include "gpio.h"
#include "i2c_scheduler.h"
#include "segger_wrapper.h"
#include "parameters.h"
#include "millis.h"
#include "fram.h"
#include "Model.h"


static void _int1_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

	W_SYSVIEW_RecordEnterISR();

	// clear trigger
	//gpio_toggle(LED_1);

    // TODO schedule sensor reading

	W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
static void _i2c_scheduling_sensors_post_init(void) {

	// configure GPIOTE
	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_PULLDOWN;
	in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

	ret_code_t err_code = nrfx_gpiote_in_init(GYR_INT1, &in_config, _int1_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(GYR_INT1, true);
}


/**
 *
 */
static void _i2c_scheduling_sensors_init() {

	// Init sensors configuration

}

/**
 *
 */
void i2c_scheduling_init(void) {

	_i2c_scheduling_sensors_init();

	LOG_WARNING("Sensors initialized");

	// post-init steps
	//_i2c_scheduling_sensors_post_init();

}

/**
 *
 */
void i2c_scheduling_uninit(void) {

	i2c_uninit();

	nrfx_gpiote_in_uninit(GYR_INT1);

}

void i2c_scheduling_tasks(void) {

}
