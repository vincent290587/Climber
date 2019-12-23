/*
 * decoder.cpp
 *
 *  Created on: 17 déc. 2019
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

	int data0 = 0;

	int res = sscanf((char*)data, "S%d", &data0);

	if (res == 1) {

		LOG_INFO("ZPM decoded ! (%d)", res);

		float slope = (float)data0 / 100.0f - 200.0f;

		data_dispatcher__feed_target_slope(slope);
	} else {

		LOG_ERROR("ZPM data decoding error (%d)", res);
	}

#if defined (BLE_STACK_SUPPORT_REQD)
	static char s_buffer[80];

	snprintf(s_buffer, sizeof(s_buffer), "Received ZPM data: %d \r\n", data0);

	// log through BLE every second
	ble_nus_log_text(s_buffer);
#endif
}
