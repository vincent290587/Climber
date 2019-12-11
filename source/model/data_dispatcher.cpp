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
static task_id_t m_task_id = 0;

static sKalmanDescr m_k_lin;

static uint32_t m_update_time = 0;
static uint32_t m_nb_runs = 0;
static float m_distance = 0;
static int32_t m_d_target = 0;
static int32_t m_distance_cal = 300;
static float m_acceleration_mg[3];
static float m_angular_rate_mdps[3];


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

	LOG_INFO("Kalman lin. init !");
}

static float _kalman_run(void) {

	// TODO run kalman
	static sKalmanExtFeed feed;

	feed.dt = 0.001f * (float)(millis() - m_update_time); // in seconds

	feed.matZ.resize(m_k_lin.ker.obs_dim, m_k_lin.ker.obs_dim);
	feed.matZ.set(0, 0, m_distance);

	feed.matU.resize(m_k_lin.ker.ker_dim, 1);
	feed.matU.zeros();
	// TODO set U
	//feed.matU.set(1, 0, model_spd);

	m_k_lin.ker.matA.set(0, 1, feed.dt);
	m_k_lin.ker.matB.set(0, 1, feed.dt);

	kalman_lin_feed(&m_k_lin, &feed);

	LOG_INFO("Kalman run");

	m_nb_runs++;

	return m_k_lin.ker.matX.get(0,0);
}

void data_dispatcher__init(task_id_t _task_id) {

	m_task_id = _task_id;

#ifdef USE_JSCOPE
	jscope.init();
#endif

	_kalman_init();

	vnh5019_driver__init();
}

void data_dispatcher__feed_target_slope(float slope) {

	static const float bike_reach_mm = 1000;
	int32_t front_el = (int32_t)(slope * bike_reach_mm / 100.0f);

	// calculate distance from desired slope
	m_d_target = front_el + m_distance_cal;

	LOG_DEBUG("Target el. dispatched: %d (mm) from %f \%", m_d_target, slope);
}

void data_dispatcher__feed_distance(float distance) {

	if (isnan(distance)) {
		LOG_ERROR("Illegal float");
		return;
	}
	m_distance = distance;
	m_update_time = millis();

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

	while (!m_updated.acc || !m_updated.dist) {
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

	if (m_nb_runs > 10) {
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
