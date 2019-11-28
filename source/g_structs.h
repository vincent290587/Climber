/*
 * g_structs.h
 *
 *  Created on: 21 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_G_STRUCTS_H_
#define SOURCE_G_STRUCTS_H_

#include <stdint.h>
#include <stdbool.h>


typedef union {
	struct {
		uint16_t version;
		uint16_t crc;
	};
	uint8_t flat_user_params;
} sUserParameters;


#endif /* SOURCE_G_STRUCTS_H_ */
