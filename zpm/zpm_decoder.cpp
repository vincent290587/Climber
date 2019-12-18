/*
 * decoder.cpp
 *
 *  Created on: 17 d�c. 2019
 *      Author: vgol
 */

#include <stdio.h>
#include "zpm_decoder.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include "ble_api_base.h"
#endif


void zpm_decoder__handle(uint8_t *data, uint16_t length) {

	int32_t data1, data2, data3;

	int res = sscanf((char*)data, "V%dP%dH%d", &data1, &data2, &data3);

	float speed_ms = data1 / 3600000.0f;
	float power    = data2;
	float alti     = data3;

	if (res == 3) {

		data_dispatcher__feed_erg(speed_ms, alti, power);
	} else {

		LOG_ERROR("ZPM data decoding error (%d)", res);
	}

#if defined (BLE_STACK_SUPPORT_REQD)
	static char s_buffer[80];

	snprintf(s_buffer, sizeof(s_buffer), "Received ZPM data: %ld %ld %ld \r\n", data1, data2, data3);

	// log through BLE every second
	ble_nus_log_text(s_buffer);
#endif
}