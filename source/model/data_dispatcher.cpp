/*
 * data_dispatcher.cpp
 *
 *  Created on: 4 apr. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "gpio.h"
#include "millis.h"
#include "sine_fitter.h"
#include "Model.h"
#include "data_dispatcher.h"
#include "math_wrapper.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble_api_base.h"
#endif

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

typedef struct {
	uint8_t acc;
	uint8_t dist;
} sUpdateType;

static sUpdateType m_updated;
static task_id_t m_task_id = 0;

static float m_distance = 0;
static float m_acceleration_mg[3];
static float m_angular_rate_mdps[3];


void data_dispatcher__init(task_id_t _task_id) {

	m_task_id = _task_id;

	// TODO init kalman

}

void data_dispatcher__run(void) {

	while (!m_updated.acc || !m_updated.dist) {
		w_task_delay(20);
		return;
	}
	m_updated.acc = 0;
	m_updated.dist = 0;

	// TODO run kalman

	LOG_INFO("Kalman run");

#ifdef BLE_STACK_SUPPORT_REQD
	static char s_buffer[60];

	snprintf(s_buffer, sizeof(s_buffer), "hi :-) %lu", millis());

	// log through BLE every second
	ble_nus_log_text(s_buffer);
#endif
}

void data_dispatcher__feed_distance(float distance) {

	if (isnan(distance)) {
		LOG_ERROR("Illegal float");
		return;
	}

	m_updated.acc = 1;
	m_distance = distance;

	w_task_delay_cancel(m_task_id);

}

void data_dispatcher__feed_acc(float acceleration_mg[3], float angular_rate_mdps[3]) {

	m_updated.dist = 1;

	memcpy(m_acceleration_mg, acceleration_mg, 12);
	memcpy(m_angular_rate_mdps, angular_rate_mdps, 12);

	w_task_delay_cancel(m_task_id);

}
