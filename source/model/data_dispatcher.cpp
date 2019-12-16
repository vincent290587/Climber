/*
 * data_dispatcher.cpp
 *
 *  Created on: 4 apr. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "gpio.h"
#include "utils.h"
#include "millis.h"
#include "kalman_ext.h"
#include "Model.h"
#include "data_dispatcher.h"
#include "vnh5019_driver.h"
#include "math_wrapper.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble_api_base.h"
#endif

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

#define DIST_STD_DEV_M                0.8f

typedef struct {
	uint8_t acc;
	uint8_t dist;
} sUpdateType;

static sUpdateType m_updated;
static task_id_t m_task_id = TASK_ID_INVALID;

static sKalmanDescr m_k_lin;

static uint32_t m_update_time = 0;
static uint32_t m_nb_runs = 0;
static float m_distance = 0;
static int32_t m_d_target = 320;
static int32_t m_distance_cal = 300;
static float m_acceleration_mg[3];
static float m_angular_rate_mdps[3];

#ifdef TDD
float m_last_est_dist = 0;
float m_last_innov = 0;
#endif

void _kalman_init(void) {

	// init kalman
	m_k_lin.ker.ker_dim = 2;
	m_k_lin.ker.obs_dim = 1;

	kalman_lin_init(&m_k_lin);

	m_k_lin.ker.matA.unity();
	m_k_lin.ker.matA.print();

	m_k_lin.ker.matB.zeros();
	m_k_lin.ker.matB.print();

	m_k_lin.ker.matC.set(0, 0, 1);

	// set Q
	m_k_lin.ker.matQ.unity(1 / 20.);

	// set P
	m_k_lin.ker.matP.ones(900);

	// set R
	m_k_lin.ker.matR.unity(DIST_STD_DEV_M * DIST_STD_DEV_M);

	// init X
	m_k_lin.ker.matX.set(0, 0, 325.0f);

	LOG_INFO("Kalman lin. init !");

	/*
	 * Slope can also be estimated using the following equations:
	 * (if only I had in-game slope for ERG mode...)
	 *
	 * m.v_dot = m.gp + P_n/v
	 *
	 * gp = g.sin(a)
	 *
	 * sl = tan(a)
	 *
	 * hp = v.sin(a)
	 *
	 * | v  |   | 1      dt     0     0    0 |
	 * | vp | = | P_n/m.v 0  -1/g     0    0 |
	 * | sa |   | 0       0     1     0    0 | . Xm
	 * | h  |   | 0       0     0     1   dt |
	 * | hp |   | 0       0     v     0    1 |
	 *
	 */
}

static float _kalman_run(void) {

	// run kalman
	static sKalmanExtFeed feed;

	feed.dt = 0.001f * (float)(millis() - m_update_time); // in seconds
	m_update_time = millis();

	feed.matZ.resize(m_k_lin.ker.obs_dim, m_k_lin.ker.obs_dim);
	feed.matZ.set(0, 0, m_distance);

	m_k_lin.ker.matB.zeros();

	// set U
	int16_t speed_mm_s = vnh5019_driver__getM1Speed();
	feed.matU.resize(m_k_lin.ker.ker_dim, 1);
	feed.matU.zeros();
	feed.matU.set(1, 0, (float)speed_mm_s);

	m_k_lin.ker.matA.set(0, 1, feed.dt);
	m_k_lin.ker.matB.set(0, 1, feed.dt);

	kalman_lin_feed(&m_k_lin, &feed);

	LOG_INFO("Kalman run");

	m_nb_runs++;

#ifdef TDD
	m_last_est_dist = m_k_lin.ker.matX.get(0,0);
	m_last_innov = m_k_lin.ker.matKI.get(0,0);
#endif

	return m_k_lin.ker.matX.get(0,0);
}

void data_dispatcher__init(task_id_t _task_id) {

	m_task_id = _task_id;

#ifdef USE_JSCOPE
	jscope.init();
#endif

	_kalman_init();

	vnh5019_driver__init();

	if (u_settings.isConfigValid()) {
		sUserParameters *params = user_settings_get();
		m_distance_cal = params->calibration;

		LOG_INFO("FRAM cal: %d (mm) ", m_distance_cal);
	} else {

		LOG_ERROR("FRAM config not valid ");
	}
}

void data_dispatcher__offset_calibration(int32_t cal) {

	m_distance_cal += cal;

	sUserParameters *params = user_settings_get();
	params->calibration = m_distance_cal;

	LOG_DEBUG("New cal: %d (mm) ", m_distance_cal);

	if (u_settings.writeConfig()) {

		LOG_INFO("FRAM cal saved ");
	} else {

		LOG_ERROR("FRAM cal NOT saved ");
	}
}

void data_dispatcher__feed_target_slope(float slope) {

	int32_t front_el = (int32_t)(slope * BIKE_REACH_MM / 100.0f);

	// calculate distance from desired slope
	m_d_target = front_el + m_distance_cal;

	LOG_WARNING("Target el. dispatched: %d (mm) from %d / 1000", m_d_target, (int)(slope*10.0f));


#if defined (BLE_STACK_SUPPORT_REQD)
	// BLE disabling for maxing ANT+ antenna time
	static int is_scanning = 1;
	if (is_scanning) {
		is_scanning = 0;
		ble_uninit();
	}
#endif
}

void data_dispatcher__feed_distance(float distance) {

	if (isnan(distance)) {
		LOG_ERROR("Illegal float");
		return;
	}
	m_distance = distance;

	m_updated.dist = 1;

	w_task_delay_cancel(m_task_id);

}

void data_dispatcher__feed_acc(float acceleration_mg[3], float angular_rate_mdps[3]) {

	memcpy(m_acceleration_mg, acceleration_mg, 12);
	memcpy(m_angular_rate_mdps, angular_rate_mdps, 12);

	m_updated.acc = 1;

	w_task_delay_cancel(m_task_id);

}

void data_dispatcher__run(void) {

	while (!m_updated.acc || !m_updated.dist || m_d_target < 100) {
		w_task_delay(20);
		return;
	}
	m_updated.acc = 0;
	m_updated.dist = 0;

	// run kalman
	float f_dist_mm = _kalman_run();

	LOG_INFO("Filtered dist: %ld mm", (int32_t)(f_dist_mm));

	// calculate target speed
	float delta_target = regFenLim(m_d_target - f_dist_mm, -17.0f, 17.0f, -17.0f, 17.0f) ;

	int16_t i_delta_mm = (int16_t)delta_target;
	int16_t speed_mm_s = (int16_t)i_delta_mm;

	LOG_INFO("Target speed: %d mm/s", speed_mm_s);

	if (m_nb_runs > KALMAN_FREERUN_NB) {
		vnh5019_driver__setM1Speed(speed_mm_s);
	}

#ifdef USE_JSCOPE
	jscope.inputData(f_dist_mm , 0);
	jscope.inputData(m_distance, 4);
	jscope.flush();
#endif

#ifdef BLE_STACK_SUPPORT_REQD
	static char s_buffer[60];

	snprintf(s_buffer, sizeof(s_buffer), "hi :-) %lu", millis());

	// log through BLE every second
	ble_nus_log_text(s_buffer);
#endif
}
