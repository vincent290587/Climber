/*
 * vnh5019_drivers.h
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#ifndef LIBRARIES_VNH5019_VNH5019_DRIVER_H_
#define LIBRARIES_VNH5019_VNH5019_DRIVER_H_

#include <stdint.h>


#ifdef	__cplusplus
extern "C" {
#endif

void vnh5019_driver__init(void);

void vnh5019_driver__setM1Speed(int16_t speed_mm_s);

void vnh5019_driver__setM1Brake(uint16_t brake);

uint32_t vnh5019_driver__getM1CurrentMilliamps(void);

uint32_t vnh5019_driver__getM1Fault(void);

#ifdef	__cplusplus
}
#endif

#endif /* LIBRARIES_VNH5019_VNH5019_DRIVER_H_ */
