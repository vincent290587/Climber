/*
 * sine_fitter.h
 *
 *  Created on: 27 mars 2019
 *      Author: Vincent
 */

#ifndef LIBRARIES_KALMAN_SINE_FITTER_H_
#define LIBRARIES_KALMAN_SINE_FITTER_H_

#include <stdint.h>


typedef struct {
	float alpha;
	float beta;
	float phi;
} sSineFitterOuput;


void sine_fitter_compute(float *dataz, float omega, float sampling, uint16_t numOfData, sSineFitterOuput *output);


#endif /* LIBRARIES_KALMAN_SINE_FITTER_H_ */
