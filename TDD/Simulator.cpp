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
#include "tdd_logger.h"
#include "Simulator.h"
#include "data_dispatcher.h"
#include "zpm_decoder.h"
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

	const char * zpm_data = "V40000000P340H1567";
	zpm_decoder__handle((uint8_t*)zpm_data, strlen(zpm_data));

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

#define SIM_DT_MS                      50
#define SIM_SLOPE_PERIOD_MS            20000
#define BIKE_HUB_DIST_MM               315


enum {
	TDD_LOGGING_TGT_SLOPE = TDD_LOGGING_USER_START,
	TDD_LOGGING_MEAS_DIST,
	TDD_LOGGING_EST_INNOV,
	TDD_LOGGING_EST_DIST,
	TDD_LOGGING_EST_ERROR,
	TDD_LOGGING_MOTOR_SPEED,
	TDD_LOGGING_ACT_POS,
	TDD_LOGGING_SIM_SLOPE,
	TDD_LOGGING_USER_END,
};

void simulator_init(void) {

	tdd_logger_init("simu.txt");

	tdd_logger_log_name(TDD_LOGGING_TIME		, "millis");

	tdd_logger_log_name(TDD_LOGGING_TGT_SLOPE	, "target_slope");
	tdd_logger_log_name(TDD_LOGGING_MEAS_DIST	, "target_dist");
	tdd_logger_log_name(TDD_LOGGING_EST_INNOV	, "kal_innov");
	tdd_logger_log_name(TDD_LOGGING_EST_DIST	, "est_dist");
	tdd_logger_log_name(TDD_LOGGING_EST_ERROR	, "est_error");
	tdd_logger_log_name(TDD_LOGGING_MOTOR_SPEED	, "motor_speed");
	tdd_logger_log_name(TDD_LOGGING_ACT_POS		, "act_pos");
	tdd_logger_log_name(TDD_LOGGING_SIM_SLOPE	, "sim_slope");

	data_dispatcher__offset_calibration(15);
}

void simulator_task(void * p_context) {

	simulator_init();

	float sim_phase = 0;
	float tgt_slope = 0;

	LOG_INFO("Simu %u ", millis());

	for (;;) {

		if (millis() < 5000) {

			if (millis() > 1500) {

				tdd_logger_start();
			}

		} else if (millis() < 15000) {

			sim_phase += SIM_DT_MS * 2.0f * M_PI / SIM_SLOPE_PERIOD_MS;

			tgt_slope = 4.5f * sinf(sim_phase);

		} else if (millis() < 30000) {

			static int nb_ind = 0;

			if (!nb_ind) {

				tgt_slope = 4.0f;
			}

			if (nb_ind++ > 3000 / SIM_DT_MS && millis() < 19000) {

				tgt_slope = -14;
				nb_ind = 1;
			}

		} else {

			LOG_ERROR("Exiting simulation");
			exit(0);
		}

		data_dispatcher__feed_target_slope(tgt_slope);

		// calculate distance from desired slope
		int32_t front_el = (int32_t)(tgt_slope * BIKE_REACH_MM / 100.0f);
		int target_dist = front_el + BIKE_HUB_DIST_MM;

		// simulate actuator
		int16_t vnh_dist_mm = tdd_vnh5019_driver__get_length();

		// inject sim dist
		tdd_inject_vl53l1_measurement(vnh_dist_mm);

#if 1
		// changing slope with constant speed
		const float slope = 0.1f + 0.2f * sinf(sim_phase);

		// inject sim erg
		float m_speed = 10.0f;
		float speed28 = powf(m_speed, 2.8f);
		float v_speed = m_speed * sin(slope);
		float power = v_speed * 69 * 9.81 + (0.0102f * speed28) + 9.428f;
		static float alti = 200;
		float dh = v_speed * SIM_DT_MS / 1000.0f;
		alti += dh;
#elif 0
		// constant slope with changing power and speed
		const float slope = 0.1f;
		const float inc_speed = 0.02f;

		// inject sim erg
		static float m_speed = 1.0f;
		m_speed += inc_speed;
		float speed28 = powf(m_speed, 2.8f);
		float v_speed = m_speed * sin(slope);
		float power = v_speed * 69 * 9.81 + (0.0102f * speed28) + 9.428f + inc_speed * 69.0f * 1000.0f / (SIM_DT_MS / m_speed);
		static float alti = 200;
		float dh = v_speed * SIM_DT_MS / 1000.0f;
		alti += dh;
#else
		// constant net power=0, slope and speed auto
		const float alti = 110.0f + 10.0f * sinf(sim_phase);
		const float power = 0;
		const float slope = 1000.0f * 2.0f * M_PI * cosf(sim_phase) / SIM_SLOPE_PERIOD_MS;
		const float inc_speed = 0.02f;

		// inject sim erg
		const float m_speed = 20.0f - sqrtf( 2.0f * 9.81f * (alti - 100.0f));
#endif

//		data_dispatcher__feed_erg(m_speed, alti, power);
		data_dispatcher__feed_erg(0, 0, 0);

		// logging
		extern int16_t	m_vnh_speed_mm_s;
		extern float	m_last_innov;
		extern float	m_last_est_dist;
		extern float	m_last_est_slope;

		float sim_slope = ((float)vnh_dist_mm - BIKE_HUB_DIST_MM) * 100.0f / BIKE_REACH_MM;

		tdd_logger_log_int(TDD_LOGGING_TIME, millis());

		tdd_logger_log_float(TDD_LOGGING_TGT_SLOPE	, tgt_slope);
		tdd_logger_log_int(TDD_LOGGING_MEAS_DIST	, target_dist);
		tdd_logger_log_float(TDD_LOGGING_EST_INNOV	, m_last_innov);
		tdd_logger_log_float(TDD_LOGGING_EST_DIST	, m_last_est_dist);
		tdd_logger_log_float(TDD_LOGGING_EST_ERROR	, 100.0f * (m_last_est_dist- (float)vnh_dist_mm) / (float)vnh_dist_mm);
		tdd_logger_log_sint(TDD_LOGGING_MOTOR_SPEED	, m_vnh_speed_mm_s);
		tdd_logger_log_int(TDD_LOGGING_ACT_POS		, vnh_dist_mm);
		tdd_logger_log_float(TDD_LOGGING_SIM_SLOPE	, sim_slope);

		tdd_logger_flush();

		w_task_delay(SIM_DT_MS);
	}

}
