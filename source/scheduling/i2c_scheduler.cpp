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


/**
 *
 */
static void _i2c_scheduling_sensors_post_init(void) {


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
	_i2c_scheduling_sensors_post_init();

}

/**
 *
 */
void i2c_scheduling_uninit(void) {

	i2c_uninit();

}

void i2c_scheduling_tasks(void) {

}
