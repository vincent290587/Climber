/*
 * Simulator.cpp
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "millis.h"
#include "Model_tdd.h"
#include "Simulator.h"
#include "segger_wrapper.h"
#include "math_wrapper.h"
#include "assert_wrapper.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/


void simulator_init(void) {

	m_app_error.hf_desc.crc = SYSTEM_DESCR_POS_CRC;
	m_app_error.hf_desc.stck.pc = 0x567896;

	m_app_error.special = SYSTEM_DESCR_POS_CRC;

	m_app_error.err_desc.crc = SYSTEM_DESCR_POS_CRC;
	snprintf(m_app_error.err_desc._buffer,
			sizeof(m_app_error.err_desc._buffer),
			"Error 0x123456 in file /mnt/e/Nordic/Projects/Perso/stravaV10/TDD/Simulator.cpp:48");

}

void simulator_tasks(void) {

	if (millis() < 5000) {
		return;
	}

	if (millis() > 90000) {
		data_dispatcher_feed_gyro(500.0F);
		return;
	}

	float sampling_hz = 25;
	float omega = 2 * M_PI; // rad/s

	float val;
	static int i = 0;

	val = omega * 1000 * 180 / M_PI;
	val += 3000 * sinf(omega * i / sampling_hz);
	i++;

	data_dispatcher_feed_gyro(val);

	// make sure we don't run for eternity...
	if (millis() > 5 * 60000) {
		exit(-1);
	}
}
