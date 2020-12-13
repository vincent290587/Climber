/*
 * g_structs.h
 *
 *  Created on: 21 d�c. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_G_STRUCTS_H_
#define SOURCE_G_STRUCTS_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum {
	eFecControlTargetNone,
	eFecControlTargetPower,
	eFecControlSlope,
} eFecControlType;

typedef struct {
	uint16_t target_power_w;
} sControlTargetPower;

typedef struct {
	float slope_ppc;
	float rolling_resistance;
} sControlSlope;

typedef union {
	sControlTargetPower power_control;
	sControlSlope       slope_control;
} uControlPages;

typedef struct {
	eFecControlType type;
	uControlPages   data;
} sFecControl;

typedef struct {
	uint16_t power;
	uint16_t speed;
	uint8_t el_time;
} sFecInfo;


typedef union __attribute__((__packed__)) {
	struct __attribute__((__packed__)) {
		int32_t  calibration;
		uint16_t version;
		uint8_t crc;
	};
	uint8_t flat_user_params[7];
} sUserParameters;


#endif /* SOURCE_G_STRUCTS_H_ */
