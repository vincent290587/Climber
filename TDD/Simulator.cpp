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
#include "kalman_ext.h"
#include "Model_tdd.h"
#include "Simulator.h"
#include "segger_wrapper.h"
#include "math_wrapper.h"
#include "assert_wrapper.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static sKalmanDescr m_k_lin;
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

	m_k_lin.ker.ker_dim = 2;
	m_k_lin.ker.obs_dim = 1;

	kalman_lin_init(&m_k_lin);

	m_k_lin.ker.matA.unity();
	m_k_lin.ker.matA.print();

	m_k_lin.ker.matB.zeros();
	m_k_lin.ker.matC.set(1, 1, 1);
	m_k_lin.ker.matB.print();

	m_k_lin.ker.matC.set(0, 0, 1);

	// set Q
	m_k_lin.ker.matQ.unity(1 / 20.);

	// set P
	m_k_lin.ker.matP.ones(900);

	// set R
	m_k_lin.ker.matR.unity(0.1);

	LOG_INFO("Kalman lin. init !");
}

void simulator_tasks(void) {

	if (millis() < 5000) {
		return;
	}

	sKalmanExtFeed feed;
	feed.dt = 0.5; // in seconds
	feed.matZ.resize(1, 1);
	feed.matU.resize(2, 1);

	feed.matU.zeros();

	m_k_lin.ker.matA.set(0, 1, feed.dt);

	// TODO
	kalman_lin_feed(&m_k_lin, &feed);

	LOG_INFO("Kalman lin. run !");

	// make sure we don't run for eternity...
	if (millis() > 1 * 60000) {
		exit(-1);
	}
}
