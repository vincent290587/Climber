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
#include "Model.h"
#include "Simulator.h"
#include "segger_wrapper.h"
#include "math_wrapper.h"
#include "assert_wrapper.h"

#include <random>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SIMU_DT_SECONDS              0.5f
#define DIST_STD_DEV_M                0.05

static sKalmanDescr m_k_lin;

static std::default_random_engine generator;
static std::normal_distribution<float> distr_alt(0.0, DIST_STD_DEV_M);
/*******************************************************************************
 * Variables
 ******************************************************************************/


void _init(void) {

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
	m_k_lin.ker.matB.set(0, 1, SIMU_DT_SECONDS);
	m_k_lin.ker.matB.print();

	m_k_lin.ker.matC.set(0, 0, 1);

	// set Q
	m_k_lin.ker.matQ.unity(1 / 20.);

	// set P
	m_k_lin.ker.matP.ones(900);

	// set R
	m_k_lin.ker.matR.unity(DIST_STD_DEV_M * DIST_STD_DEV_M);

	LOG_INFO("Kalman lin. init !");
}

void simulator_test(void) {

	_init();

	for (int i=0; i < 250; i++) {

		static float model_alt = 50;
		static uint32_t model_ind = 0;
		static float model_spd = 0.05;

		if (model_ind++ > 50) {
			model_spd = -model_spd;
			model_ind = 0;
		}

		model_alt += model_spd * SIMU_DT_SECONDS;

		sKalmanExtFeed feed;

		feed.dt = SIMU_DT_SECONDS; // in seconds

		feed.matZ.resize(m_k_lin.ker.obs_dim, m_k_lin.ker.obs_dim);
		feed.matZ.set(0, 0, model_alt);

		feed.matU.resize(m_k_lin.ker.ker_dim, 1);
		feed.matU.zeros();
		feed.matU.set(1, 0, model_spd);

		m_k_lin.ker.matA.set(0, 1, feed.dt);

		kalman_lin_feed(&m_k_lin, &feed);

		printf("%f %f %f %f \r\n",
				model_alt,
				model_spd,
				m_k_lin.ker.matX.get(0,0),
				m_k_lin.ker.matX.get(1,0));

	}
}


void simulator_init(void) {


}

void simulator_tasks(void) {


}
