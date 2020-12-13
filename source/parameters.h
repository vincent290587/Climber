/*
 * parameters.h
 *
 *  Created on: 11 nov. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_PARAMETERS_H_
#define SOURCE_PARAMETERS_H_

#define APP_TIMEOUT_DELAY_MS           5

#define S_TO_MS(X)                     ((X)*1000)

///// CODE FLAGS



//#define FDS_PRESENT

#define KALMAN_FREERUN_NB         20

#define BIKE_REACH_MM             1000

#define SCAN_STOP_TIME_MS         (5 * 60 * 1000)

#define VNH_FULL_SCALE            60

#define DEFAULT_TARGET_DISTANCE   230

#define ZWIFT_SLOPE_FACTOR        2.0f

#define ACTUATOR_MIN_LENGTH       210.f

#endif /* SOURCE_PARAMETERS_H_ */
