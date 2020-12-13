/*
 * data_dispatcher.cpp
 *
 *  Created on: 4 apr. 2019
 *      Author: v.golle
 */

#include <cmath>
#include <string.h>
#include "gpio.h"
#include "utils.h"
#include "millis.h"
#include "arm_pid.h"
#include "kalman_ext.h"
#include "Model.h"
#include "data_dispatcher.h"
#include "vnh5019_driver.h"
#include "math_wrapper.h"
#include "SMA.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble_api_base.h"
#endif

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

#define DIST_STD_DEV_M                6.0f
#define ERG_STD_DEV_M                 0.1f
#define KAL_ERG_SPD_LIMIT             1.5f

typedef struct {
	uint8_t acc;
	uint8_t dist;
	uint8_t erg;
} sUpdateType;

static sUpdateType m_updated;
static task_id_t m_task_id = TASK_ID_INVALID;

static sKalmanDescr m_k_lin;
static sKalmanDescr m_k_erg;

static uint32_t m_update_time = 0;
static uint32_t m_nb_runs = 0;
static float m_distance = 0;
static float m_d_target = 0;
static float m_error_inv = 0;
static int32_t m_distance_cal = DEFAULT_TARGET_DISTANCE;
static float m_acceleration_mg[3];
static float m_angular_rate_mdps[3];
static float m_speed, m_alti, m_power;

static s_pid_config_f32   m_pid_config;
static s_pid_instance_f32 m_cmsis_pid;

static int32_t m_deadzone_center = DEFAULT_TARGET_DISTANCE;
static uint8_t m_deadzone_activ = 1;
static uint8_t m_new_command = 0;

#ifdef TDD
float m_last_est_dist = 0;
float m_last_innov = 0;
float m_last_est_slope = 0;
float m_last_target = 0;
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

	// set Q: kernel noise
	m_k_lin.ker.matQ.unity(1 / 20.f);
	m_k_lin.ker.matQ.set(1, 1, 1 / 1.5f);

	// set P
	m_k_lin.ker.matP.ones(900);

	// set R
	m_k_lin.ker.matR.unity(DIST_STD_DEV_M * DIST_STD_DEV_M);

	// init X
	m_k_lin.ker.matX.set(0, 0, DEFAULT_TARGET_DISTANCE);

	LOG_INFO("Kalman lin. init !");

}

static float _kalman_run(void) {

	// run kalman
	static sKalmanExtFeed feed;

	feed.dt = 0.001f * (float)(millis() - m_update_time); // in seconds
	m_update_time = millis();

	// set measures: Z
	feed.matZ.resize(m_k_lin.ker.obs_dim, 1);
	feed.matZ.zeros();
	feed.matZ.set(0, 0, m_distance);

	// measures mapping
	m_k_lin.ker.matC.set(0, 0, 1);

	// command mapping
	m_k_lin.ker.matB.zeros();
	m_k_lin.ker.matB.set(0, 1, feed.dt);

	// set command: U
	float speed_mm_s = map_duty_to_speed(vnh5019_driver__getM1_duty());
	feed.matU.resize(m_k_lin.ker.ker_dim, 1);
	feed.matU.zeros();
	feed.matU.set(1, 0, speed_mm_s);

	// set core
	m_k_lin.ker.matA.set(0, 1, feed.dt);
	m_k_lin.ker.matA.set(1, 1, 1);

	kalman_lin_feed(&m_k_lin, &feed);

	LOG_DEBUG("Kalman run");

	m_nb_runs++;

#ifdef TDD
	m_last_est_dist = m_k_lin.ker.matX.get(0, 0);
	m_last_innov    = m_k_lin.ker.matKI.get(0, 0);
#endif

	return m_k_lin.ker.matX.get(0,0);
}

#if 0
void _kalman_erg_init(void) {

	// init kalman
	m_k_erg.ker.ker_dim = 6;
	m_k_erg.ker.obs_dim = 3;

	kalman_lin_init(&m_k_erg);

	m_k_erg.ker.matA.unity();
	m_k_erg.ker.matA.print();

	m_k_erg.ker.matB.zeros();
	m_k_erg.ker.matB.print();

	// set Q: kernel noise
	m_k_erg.ker.matQ.unity(1 / 20.);
	m_k_erg.ker.matQ.set(2, 2, 1 / 5.);

	// set P
	m_k_erg.ker.matP.ones(900);

	// set R: measurement noise
	m_k_erg.ker.matR.unity(ERG_STD_DEV_M * ERG_STD_DEV_M);

	// init X
	m_k_erg.ker.matX.set(0, 0, 0);

	LOG_INFO("Kalman ERG init !");

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
	 * | v  |   | 1      dt     0     0    0      0 |
	 * | vp | = | 0       0    -g     0    0  P/m.v |
	 * | sa |   | 0       0     1     0    0      0 | . Xm
	 * | h  |   | 0       0     0     1   dt      0 |
	 * | hp |   | 0       0     v     0    0      0 |
	 * | Pn |   | 0       0     0     0    0      1 |
	 *

		1.000000 0.090000 0.000000 0.000000 0.000000 0.000000
		0.000000 0.000000 -9.810000 0.000000 0.000000 0.001449
		0.000000 0.000000 1.000000 0.000000 0.000000 0.000000
		0.000000 0.000000 0.000000 1.000000 0.090000 0.000000
		0.000000 0.000000 10.000000 0.000000 0.000000 0.000000
		0.000000 0.000000 0.000000 0.000000 0.000000 1.000000

	 *
	 */
}

static float _kalman_erg_run(void) {

//	// run kalman
	static sKalmanExtFeed feed;
	static uint32_t _update_time;
	static uint32_t _nb_runs = 0;

	static float res = 0;
	static float alti_prev = 0;
	static const float time_const = 0.3f;

	// calculate net power
	if (m_speed < KAL_ERG_SPD_LIMIT) {
		_update_time = millis();
		alti_prev = m_alti;
		return res;
	}

	feed.dt = 0.001f * (float)(millis() - _update_time); // in seconds
	_update_time = millis();

//	// set measures: Z
//	feed.matZ.resize(m_k_erg.ker.obs_dim, 1);
//	feed.matZ.zeros();
//	feed.matZ.set(0, 0, m_speed);
//	feed.matZ.set(1, 0, m_alti);
//	feed.matZ.set(2, 0, m_power);
//
//	// measures mapping
//	m_k_erg.ker.matC.set(0, 0, 1);
//	m_k_erg.ker.matC.set(1, 3, 1);
//	m_k_erg.ker.matC.set(2, 5, 1);
//
//	// command mapping
//	m_k_erg.ker.matB.zeros();
//
//	// set command: U
//	feed.matU.zeros();
//
//	// set core
//	m_k_erg.ker.matA.set(0, 0, 1);
//	m_k_erg.ker.matA.set(0, 1, feed.dt);
//	m_k_erg.ker.matA.set(1, 0, 0);
//	m_k_erg.ker.matA.set(1, 1, 0);
//	m_k_erg.ker.matA.set(1, 2, -9.81f);
//	m_k_erg.ker.matA.set(1, 5, vp1);
//	m_k_erg.ker.matA.set(2, 2, 1);
//	m_k_erg.ker.matA.set(3, 3, 1);
//	m_k_erg.ker.matA.set(3, 4, feed.dt);
//	m_k_erg.ker.matA.set(4, 2, m_speed);
//	m_k_erg.ker.matA.set(4, 4, 0);
//
//	//kalman_lin_feed(&m_k_erg, &feed);
//

	if (_nb_runs++ < 10) {
		alti_prev = m_alti;
		return 0;
	}

	res = time_const * res + (1.0f - time_const) * asinf((m_alti - alti_prev) / (feed.dt * m_speed));
	alti_prev = m_alti;

#ifdef TDD
	m_last_est_slope = res;
#endif

	return res;
}
#endif

void data_dispatcher__init(task_id_t _task_id) {

	m_task_id = _task_id;

#ifdef USE_JSCOPE
	jscope.init();
#endif

	_kalman_init();

	//_kalman_erg_init();

	vnh5019_driver__init();

	if (u_settings.isConfigValid()) {
		sUserParameters *params = user_settings_get();

		m_distance_cal = params->calibration;

		LOG_ERROR("FRAM cal: %d (mm) ", m_distance_cal);
	} else
	{

		m_distance_cal = DEFAULT_TARGET_DISTANCE;

		LOG_ERROR("FRAM config not valid: overwriting ");

		u_settings.resetConfig();
	}

	// go to default slope (0%)
	m_d_target = m_distance_cal;

	//Initialize the PID instance structure
	m_pid_config.use_limits = 1;
	m_pid_config.lim_low  = -100.0f;
	m_pid_config.lim_high =  100.0f;
	m_cmsis_pid.Kp = 100.f / 5.f; // PWM = 100 @ error = 5 mm
	m_cmsis_pid.Ki = 0.0f;
	m_cmsis_pid.Kd = 0.3f;
	pid_init_f32(&m_cmsis_pid, 1);
}

void data_dispatcher__offset_calibration(int32_t cal) {

	m_distance_cal += cal;
	m_d_target += (float)cal;

	sUserParameters *params = user_settings_get();
	params->calibration = m_distance_cal;

	m_deadzone_activ = 0;
	m_new_command = 1;

	LOG_INFO("New cal: %d (mm) ", m_distance_cal);

	if (u_settings.writeConfig()) {

		LOG_INFO("FRAM cal saved ");
	} else {

		LOG_ERROR("FRAM cal NOT saved ");
	}
}

void data_dispatcher__feed_target_slope(float slope) {

	if (std::isnan(slope) ||
			fabsf(slope) > 45.0f) {
		LOG_ERROR("Illegal float");
		return;
	}

	float front_el = slope * BIKE_REACH_MM / 100.0f;

	// calculate distance from desired slope
	m_d_target = front_el + (float)m_distance_cal;

	// account for end of course
	if (m_d_target < ACTUATOR_MIN_LENGTH) {

		m_d_target = ACTUATOR_MIN_LENGTH;
	}

	if (m_deadzone_center != (int32_t)m_d_target) {

		m_deadzone_center = (int32_t)m_d_target;
		m_deadzone_activ = 0;
		m_new_command = 1;
	}

	LOG_DEBUG("Target el. dispatched: %d (mm) from %d / 1000", (int)m_d_target, (int)(slope*10.0f));

}

void data_dispatcher__feed_distance(float distance) {

	if (std::isnan(distance)) {
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

	m_updated.erg = 1;

	w_task_delay_cancel(m_task_id);

}

void data_dispatcher__feed_acc(float acceleration_mg[3], float angular_rate_mdps[3]) {

	memcpy(m_acceleration_mg, acceleration_mg, 12);
	memcpy(m_angular_rate_mdps, angular_rate_mdps, 12);

	m_updated.acc = 1;

	w_task_delay_cancel(m_task_id);

}

int16_t _map_error_to_duty(float error) {
	if (error < 0.0f) {
		return (int16_t)regFenLim(error, -7, 0, -2, -2);
	} else {
		return (int16_t)regFenLim(error, 0, 10, 50, 100);
	}
	return 0;
}

void data_dispatcher__run(void) {

#if 0
	if (m_updated.erg) {

		m_updated.erg = 0;

		// run kalman
		float f_slope_rad = _kalman_erg_run();

		float f_slope_pc = 100.0f * tanf(f_slope_rad);

		data_dispatcher__feed_target_slope(f_slope_pc);

	}
#endif

	if (m_updated.dist) {

		m_updated.dist = 0;

		// run kalman
		float f_dist_mm = _kalman_run();

		LOG_INFO("Filtered dist: %ld mm -- target dist %ld", (int32_t)(f_dist_mm), (int32_t)(m_d_target));

		// calculate target speed
		float error = m_d_target - f_dist_mm;

		// new command was received, save that for future calculation
		if (m_new_command) {

			m_error_inv = - error;
		}

		//Process the PID controller
		//float out = pid_f32(&m_cmsis_pid, &m_pid_config, error);
		int16_t i_duty_delta = _map_error_to_duty(error);//(int16_t)out;

		uint32_t fault = vnh5019_driver__getM1Fault();

		if (fault || m_deadzone_activ) {
			i_duty_delta = 0;
		}

#if 1
		float av_val = 0.f;
		static SMA m_average(7, 2);
		m_average.addSample((float)i_duty_delta);
		if (i_duty_delta != 0 && m_average.getValue(av_val)) {

			i_duty_delta = (int16_t)av_val;
		}
#endif

		static uint16_t real_duty = 0;
		int32_t current = vnh5019_driver__getM1CurrentMilliamps();

		// check if we are at the target already
		if (!m_deadzone_activ) {

			// check for target crossing and end of motion
			if ((m_error_inv * error >= 0.f && fabsf(error) < 4.f) ||
					fabsf(error) < 1.f) {

				m_deadzone_activ = 1;
				i_duty_delta = 0;
			}
		}

		// actuator stop detection
		uint8_t force_pwm = 0;
		static uint16_t error_nb = 0;
		if ((i_duty_delta > 30 || i_duty_delta <= -1) &&
				current < 100 &&
				m_nb_runs > KALMAN_FREERUN_NB) {

			error_nb++;

			if (error_nb >= 12) {
				LOG_WARNING("!! ERROR motor stop !!");
				error_nb = 0;
				force_pwm = 1;
				m_deadzone_activ = 2;
				i_duty_delta = 0;
				m_k_lin.ker.matX.set(1, 0, 0);

			} else if (error_nb >= 9) {

				force_pwm = 1;
				i_duty_delta *= 2;
			} else if (error_nb >= 6) {

				force_pwm = 1;
				i_duty_delta /= 2;
			} else if (error_nb >= 3) {

				force_pwm = 1;
			}
		} else {

			error_nb = 0;
		}

		if (m_nb_runs > KALMAN_FREERUN_NB) {

			real_duty = vnh5019_driver__setM1_duty(i_duty_delta, force_pwm);
		}

#ifdef TDD
		const char *format = "Cmd DTY: %d (%u) / e.spd %d / dist %d f=%d / tgt %d (%d) / curr %d / %u %u %u %u \r\n";
#else
		const char *format = "Cmd DTY: %d (%u) / e.spd %ld / dist %ld f=%ld / tgt %ld (%ld) / curr %ld / %lu %u %u %u \r\n";
#endif

		static char s_buffer[200];
		snprintf(s_buffer, sizeof(s_buffer), format,
				i_duty_delta,
				real_duty,
				(int32_t)(m_k_lin.ker.matX.get(1,0) * 10.0f),
				(int32_t)(m_distance),
				(int32_t)(f_dist_mm),
				(int32_t)(m_d_target),
				(int32_t)(error * 10.0f),
				current,
				millis(),
				(uint8_t)fault,
				error_nb,
				m_deadzone_activ);

		m_new_command = 0;

#if defined (BLE_STACK_SUPPORT_REQD)
		// log through BLE every loop
		ble_nus_log_text(s_buffer);
#else
		LOG_INFO("%s", s_buffer);
#endif

	}

#ifdef TDD
	m_last_target = m_d_target;
#endif

#ifdef USE_JSCOPE
	jscope.inputData(f_dist_mm , 0);
	jscope.inputData(m_distance, 4);
	jscope.flush();
#endif

	// handle the PWM tasking there
	vnh5019_driver__tasks();
}
