/*
 * lsm6ds33_wrapper.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "i2c.h"
#include "gpio.h"
#include "boards.h"
#include "helper.h"
#include "lsm6ds33_wrapper.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"

typedef union {
	int16_t i16bit[3];
	uint8_t u8bit[6];
} axis3bit16_t;

/* Private variables ---------------------------------------------------------*/
//static axis3bit16_t data_raw_acceleration;
//static axis3bit16_t data_raw_angular_rate;
static float acceleration_mg[3];
static float angular_rate_mdps[3];


static void _lsm6_readout_cb(void) {

	W_SYSVIEW_RecordEnterISR();

	axis3bit16_t data_raw[2];

	data_dispatcher__feed_acc(acceleration_mg, angular_rate_mdps);

	LOG_DEBUG("LSM6 read %u", millis());

	W_SYSVIEW_RecordExitISR();

}


void lsm6ds33_wrapper__init(void) {

}
