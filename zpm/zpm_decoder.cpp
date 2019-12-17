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


void zpm_decoder__handle(uint8_t *data, uint16_t length) {

	float speed=0, alti=0, power=0;
	int32_t data1, data2, data3;

	int res = sscanf((char*)data, "V%dP%dH%d", &data1, &data2, &data3);

	if (res == 3) {

		data_dispatcher__feed_erg(speed, alti, power);
	} else {

		LOG_ERROR("ZPM data decoding error (%d)", res);
	}
}
