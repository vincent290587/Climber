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

extern void app_shutdown(void);

void data_dispatcher_feed_gyro(float mdeg_s) {

	if (isnan(mdeg_s)) {
		LOG_ERROR("Illegal float");
		return;
	}


//#ifdef BLE_STACK_SUPPORT_REQD
//	// log cadence through BLE every second
//	static int nb_ind = 25;
//	if (nb_ind-- == 0) {
//		ble_nus_log_cadence(m_cadence, nb_turns);
//		nb_ind = 25;
//	}
//#endif
}

void data_dispatcher_feed_d_ff(float d_ff) {

#ifdef USE_JSCOPE

	// output some results to Segger JSCOPE
	jscope.inputData(d_ff, 0);

	jscope.flush();
#endif
}

//
//bool data_dispatcher_get_batt_volt(uint32_t *batt_mv) {
//	if (batt_mv) *batt_mv = 2200;
//	return true;
//}
//
//bool data_dispatcher_get_cadence(uint32_t *cad) {
//	if (cad) *cad = m_cadence;
//	return true;
//}

bool data_dispatcher_get_batt_volt(uint32_t *batt_mv) {
	if (batt_mv) *batt_mv = 2200;
	return true;
}

bool data_dispatcher_get_cadence(uint32_t *cad) {
	if (cad) *cad = m_cadence;
	return true;
}

