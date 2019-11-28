/*
 * data_dispatcher.cpp
 *
 *  Created on: 4 apr. 2019
 *      Author: v.golle
 */


#include "gpio.h"
#include "millis.h"
#include "sine_fitter.h"
#include "Model.h"
#include "data_dispatcher.h"
#include "math_wrapper.h"
#include "segger_wrapper.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble_api_base.h"
#endif

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

typedef struct {
	float m_angle[50];
	uint16_t nb_val;
} sCadenceBuffer;

static float m_angle = 0;
static uint32_t m_cadence = 0;

static uint32_t m_static_nb = 0;

static sCadenceBuffer m_cad_buffer;

static void _check_is_moving(float mdeg_s) {

	if (m_cadence < 5 &&
			m_static_nb++ > 360 * 25) {
		LOG_ERROR("Inactivity timeout: going to shutdown");

#ifdef BLE_STACK_SUPPORT_REQD
		ble_nus_log_text("Inactivity timeout: going to shutdown");
		ble_nus_tasks();
		delay_ms(10);
#endif

		m_static_nb = 0;
		app_shutdown();
	} else if (m_cadence) {
		m_static_nb = 0;
	}

}

static void _cadence_compute(float mdeg_s) {

	if (m_cad_buffer.nb_val == sizeof(m_cad_buffer.m_angle) / sizeof(m_cad_buffer.m_angle[0])) {

		float omega = 0.0F;
		// determine max and min of angular speed
		{
			float min = m_cad_buffer.m_angle[0];
			float max = m_cad_buffer.m_angle[0];

			for (unsigned i=1; i< sizeof(m_cad_buffer.m_angle) / sizeof(m_cad_buffer.m_angle[0]); i++) {

				min = MIN(min, m_cad_buffer.m_angle[i]);
				max = MAX(max, m_cad_buffer.m_angle[i]);

			}

			omega = 0.5 * (min + max);
		}

		m_cadence = (uint32_t)(fabsf(omega / 1000.) * 60. / 360. + 0.5);

		LOG_INFO("Cadence: %f %u", omega, m_cadence);

		m_cad_buffer.nb_val = 0;
	} else {
		// fill buffer
		m_cad_buffer.m_angle[m_cad_buffer.nb_val++] = mdeg_s;
	}

}

void data_dispatcher_feed_gyro(float mdeg_s) {

	if (isnan(mdeg_s)) {
		LOG_ERROR("Illegal float");
		return;
	}

	_check_is_moving(mdeg_s);

	_cadence_compute(mdeg_s);

	// integrate angular speed over time (25Hz)
	float val = fabsf(mdeg_s / 1000.) / 25.;
	m_angle += val;

	static uint32_t nb_turns = 0;

	if (m_angle > 360.) {
		m_angle = 0.0;

		LOG_WARNING("Full turn !");

		nb_turns++;

		gpio_set(LED_1);

	} else {

		gpio_clear(LED_1);

	}

#ifdef BLE_STACK_SUPPORT_REQD
	// log cadence through BLE every second
	static int nb_ind = 25;
	if (nb_ind-- == 0) {
		ble_nus_log_cadence(m_cadence, nb_turns);
		nb_ind = 25;
	}
#endif
}

void data_dispatcher_feed_d_ff(float d_ff) {

#ifdef USE_JSCOPE

	// output some results to Segger JSCOPE
	jscope.inputData(d_ff, 0);

	jscope.flush();
#endif
}


bool data_dispatcher_get_batt_volt(uint32_t *batt_mv) {
	if (batt_mv) *batt_mv = 2200;
	return true;
}

bool data_dispatcher_get_cadence(uint32_t *cad) {
	if (cad) *cad = m_cadence;
	return true;
}

