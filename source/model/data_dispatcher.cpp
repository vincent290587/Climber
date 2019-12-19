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
#define KAL_ERG_SPD_LIMIT             1.5f

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
static float m_d_target = 0;
static int32_t m_distance_cal = DEFAULT_TARGET_DISTANCE;
static float m_acceleration_mg[3];
static float m_angular_rate_mdps[3];
static float m_speed, m_alti, m_power;

#ifdef TDD
float m_last_est_dist = 0;
float m_last_innov = 0;
float m_last_est_slope = 0;
#endif

void _kalman_init(void) {

	// init kalman
	m_k_lin.ker.ker_dim = 8;
	m_k_lin.ker.obs_dim = 4;

	kalman_lin_init(&m_k_lin);

	m_k_lin.ker.matA.unity();
	m_k_lin.ker.matA.print();

	m_k_lin.ker.matB.zeros();
	m_k_lin.ker.matB.print();

	// set Q: kernel noise
	m_k_lin.ker.matQ.unity(1 / 20.);
	m_k_lin.ker.matQ.set(1, 1, 1 / 5.);
	m_k_lin.ker.matQ.set(4, 4, 1 / 80.);

	// set P
	m_k_lin.ker.matP.ones(900);

	// set R
	m_k_lin.ker.matR.unity(DIST_STD_DEV_M * DIST_STD_DEV_M);

	// init X
	m_k_lin.ker.matX.set(0, 0, DEFAULT_TARGET_DISTANCE);

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
	 * | d  |
	 * | dd |
	 * | v  |   | 1      dt     0     0    0      0 |
	 * | vp | = | 0       0    -g     0    0  P/m.v |
	 * | sa |   | 0       0     1     0    0      0 | . Xm
	 * | h  |   | 0       0     0     1   dt      0 |
	 * | hp |   | 0       0     v     0    0      0 |
	 * | Pn |   | 0       0     0     0    0      1 |
	 *
1.000000 0.150000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000

0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000

0.000000 0.000000 1.000000 0.150000 0.000000 0.000000 0.000000 0.000000

0.000000 0.000000 0.000000 0.000000 -9.810000 0.000000 0.000000 0.001449

0.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000 0.000000

0.000000 0.000000 0.000000 0.000000 0.000000 1.000000 0.150000 0.000000

0.000000 0.000000 0.000000 0.000000 10.000000 0.000000 0.000000 0.000000

0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 1.000000
	 *
	 */
}

static float _kalman_run(void) {

	// run kalman
	static sKalmanExtFeed feed;

	// calculate net power
	float vp1 = 10.0f / 69.0f;
	if (m_speed > KAL_ERG_SPD_LIMIT) {
		float speed28 = powf(m_speed, 2.8f);
		m_power -= (0.0102f * speed28) + 9.428f;
		vp1 = 1 / (69.0f * m_speed);
	} else {
		m_speed = KAL_ERG_SPD_LIMIT;
	}

	feed.dt = 0.001f * (float)(millis() - m_update_time); // in seconds
	m_update_time = millis();

	// set measures: Z
	feed.matZ.resize(m_k_lin.ker.obs_dim, 1);
	feed.matZ.zeros();
	feed.matZ.set(0, 0, m_distance);
	feed.matZ.set(1, 0, m_speed);
	feed.matZ.set(2, 0, m_alti);
	feed.matZ.set(3, 0, m_power);

	// measures mapping
	m_k_lin.ker.matC.set(0, 0, 1);
	m_k_lin.ker.matC.set(1, 2, 1);
	m_k_lin.ker.matC.set(2, 5, 1);
	m_k_lin.ker.matC.set(3, 7, 1);

	// command mapping
	m_k_lin.ker.matB.zeros();
	m_k_lin.ker.matB.set(0, 1, feed.dt);
	m_k_lin.ker.matB.set(3, 3, 1);

	// set command: U
	int16_t speed_mm_s = regFenLim(vnh5019_driver__getM1_duty(), -100.0f, 100.0f, -16.0f, 16.0f);
	feed.matU.resize(m_k_lin.ker.ker_dim, 1);
	feed.matU.zeros();
	feed.matU.set(1, 0, (float)speed_mm_s / 3.0f);

	// set core
	m_k_lin.ker.matA.set(0, 1, feed.dt);
	m_k_lin.ker.matA.set(1, 1, 1);
	m_k_lin.ker.matA.set(2, 2, 1);
	m_k_lin.ker.matA.set(2, 3, feed.dt);
	m_k_lin.ker.matA.set(3, 2, 0);
	m_k_lin.ker.matA.set(3, 3, 0);
	m_k_lin.ker.matA.set(3, 4, -9.81f);
	m_k_lin.ker.matA.set(3, 7, vp1);
	m_k_lin.ker.matA.set(4, 4, 1);
	m_k_lin.ker.matA.set(5, 5, 1);
	m_k_lin.ker.matA.set(5, 6, feed.dt);
	m_k_lin.ker.matA.set(6, 4, m_speed);
	m_k_lin.ker.matA.set(6, 6, 0);

	//m_k_lin.ker.matA.print();exit(0);

	kalman_lin_feed(&m_k_lin, &feed);

	LOG_INFO("Kalman run");

	m_nb_runs++;

#ifdef TDD
	m_last_est_dist = m_k_lin.ker.matX.get(0, 0);
	m_last_innov    = m_k_lin.ker.matKI.get(0, 0);

	m_last_est_slope = m_k_lin.ker.matX.get(4, 0);

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
		m_d_target = m_distance_cal;

		LOG_INFO("FRAM cal: %d (mm) ", m_distance_cal);
	} else {

		m_distance_cal = DEFAULT_TARGET_DISTANCE;
		m_d_target = DEFAULT_TARGET_DISTANCE;

		LOG_ERROR("FRAM config not valid ");
	}
}

void data_dispatcher__offset_calibration(int32_t cal) {

	m_distance_cal += cal;
	m_d_target += (float)cal;

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

	float front_el = slope * BIKE_REACH_MM / 100.0f;

	// calculate distance from desired slope
	m_d_target = front_el + (float)m_distance_cal;

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

void data_dispatcher__feed_erg(float speed, float alti, float power) {

	m_alti  = alti;
	m_speed = speed;
	m_power = power;

}

void data_dispatcher__feed_acc(float acceleration_mg[3], float angular_rate_mdps[3]) {

	memcpy(m_acceleration_mg, acceleration_mg, 12);
	memcpy(m_angular_rate_mdps, angular_rate_mdps, 12);

	m_updated.acc = 1;

	w_task_delay_cancel(m_task_id);

}

void data_dispatcher__run(void) {

	while (!m_updated.dist) {
		w_task_delay(20);
		return;
	}
	m_updated.acc = 0;
	m_updated.dist = 0;

	// run kalman
	float f_dist_mm = _kalman_run();

	LOG_INFO("Filtered dist: %ld mm -- target dist %ld", (int32_t)(f_dist_mm), (int32_t)(m_d_target));

	// calculate target speed
	float duty_target = regFenLim(m_d_target - f_dist_mm, -12.0f, 12.0f, -VNH_FULL_SCALE, VNH_FULL_SCALE) ;

	int16_t i_duty_delta = (int16_t)duty_target;

	static int16_t duty_delta_filt = 0;
	duty_delta_filt = 5 * i_duty_delta  + 5 * duty_delta_filt;
	duty_delta_filt /= 10;

	LOG_INFO("Target duty: %d  f: %d ", i_duty_delta, duty_delta_filt);

	if (m_nb_runs > KALMAN_FREERUN_NB) {
		vnh5019_driver__setM1_duty(duty_delta_filt);
	}

	if (vnh5019_driver__getM1Fault()) {

	}

#ifdef USE_JSCOPE
	jscope.inputData(f_dist_mm , 0);
	jscope.inputData(m_distance, 4);
	jscope.flush();
#endif

#if defined (BLE_STACK_SUPPORT_REQD)
	static char s_buffer[80];

	snprintf(s_buffer, sizeof(s_buffer), "Commanded speed: %d mm/s dist: %ld\r\n", speed_mm_s, (int32_t)(f_dist_mm));

	// log through BLE every second
	ble_nus_log_text(s_buffer);
#endif
}
