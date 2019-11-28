/*
 * data_dispatcher.h
 *
 *  Created on: 4 apr. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_DATA_DISPATCHER_H_
#define SOURCE_MODEL_DATA_DISPATCHER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


void data_dispatcher_feed_gyro(float mdeg_s);

void data_dispatcher_feed_d_ff(float d_ff);

bool data_dispatcher_get_batt_volt(uint32_t *batt_mv);

bool data_dispatcher_get_cadence(uint32_t *cad);


#ifdef __cplusplus
}
#endif

#endif /* SOURCE_MODEL_DATA_DISPATCHER_H_ */
