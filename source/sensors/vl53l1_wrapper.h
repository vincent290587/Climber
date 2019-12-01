/*
 * vl53l1_wrapper.h
 *
 *  Created on: 30 nov. 2019
 *      Author: Vincent
 */

#ifndef SOURCE_SENSORS_VL53L1_WRAPPER_H_
#define SOURCE_SENSORS_VL53L1_WRAPPER_H_


#ifdef	__cplusplus
extern "C" {
#endif

int vl53l1_wrapper__init(void);

int vl53l1_wrapper__measure(void);

#ifdef	__cplusplus
}
#endif

#endif /* SOURCE_SENSORS_VL53L1_WRAPPER_H_ */
