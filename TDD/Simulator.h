/*
 * Simulator.h
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#ifndef TDD_SIMULATOR_H_
#define TDD_SIMULATOR_H_

#include "WString.h"


#ifdef __cplusplus
extern "C" {
#endif

void tdd_inject_vl53l1_measurement(int);

int16_t tdd_vnh5019_driver__get_length(void);

void simulator_test(void);

void simulator_init(void);

void simulator_task(void * p_context);

#ifdef __cplusplus
}
#endif


#endif /* TDD_SIMULATOR_H_ */
