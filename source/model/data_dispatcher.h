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
#include "segger_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

void data_dispatcher__init(task_id_t _task_id);

void data_dispatcher__run(void);

void data_dispatcher__offset_calibration(int32_t cal);

void data_dispatcher__feed_distance(float distance);

/**
 *
 * @param slope Slope in 100 * %
 */
void data_dispatcher__feed_target_slope(float slope);

void data_dispatcher__feed_erg(float speed, float alti, float power);

void data_dispatcher__feed_acc(float acceleration_mg[3], float angular_rate_mdps[3]);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_MODEL_DATA_DISPATCHER_H_ */
