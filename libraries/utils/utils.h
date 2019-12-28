/* 
 * File:   utils.h
 * Author: vincent
 *
 * Created on October 27, 2015, 10:55 AM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>
#include "math_wrapper.h"

#ifndef M_TWOPI
#define M_TWOPI         (M_PI * 2.0f)
#endif

#ifdef	__cplusplus
extern "C" {
#endif


float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f) __attribute__ ((pure));

float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f) __attribute__ ((pure));

static inline float toRadians(float angle) __attribute__ ((pure));
static inline float toRadians(float angle) {
  return M_PI * angle / 180.0f;
}

static inline float toDegrees(float angle) __attribute__ ((pure));
static inline float toDegrees(float angle) {
  return 180.0f * angle / M_PI;
}

static inline float map_duty_to_speed(int16_t duty) __attribute__ ((pure));
static inline float map_duty_to_speed(int16_t duty) {
	if (duty <= -1) {
		return regFenLim((float)duty, -100, 0, -14.2, 0);
	} else if (duty >= 1) {
		return regFenLim((float)duty, 0, 100, 0, 12.8);
	}
	return 0;
}

static inline int16_t map_speed_to_duty(float speed) __attribute__ ((pure));
static inline int16_t map_speed_to_duty(float speed) {
	if (speed < 0.0f) {
		return (int16_t)regFenLim(speed, -14.2, 0, -100, 0);
	} else {
		return (int16_t)regFenLim(speed, 0, 12.8, 0, 100);
	}
	return 0;
}

void calculePos (const char *nom, float *lat, float *lon);

long unsigned int toBase10 (char *entree);

extern void loggerMsg(const char *msg_);

double radians(double value) __attribute__ ((pure));

double degrees(double value) __attribute__ ((pure));

double sq(double value) __attribute__ ((pure));

uint32_t get_sec_jour(uint8_t hour_, uint8_t min_, uint8_t sec_) __attribute__ ((pure));

float compute2Complement(uint8_t msb, uint8_t lsb) __attribute__ ((pure));

float percentageBatt(float tensionValue, float current) __attribute__ ((pure));

void encode_uint16 (uint8_t* dest, uint16_t input);

void encode_uint32 (uint8_t* dest, uint32_t input);

uint16_t decode_uint16 (uint8_t* dest);

uint32_t decode_uint32 (uint8_t* dest);

void const_char_to_buffer(const char *str_, uint8_t *buff_, uint16_t max_size);

float simpLinReg(float* x, float* y, float* lrCoef, int n);

uint8_t calculate_crc(uint8_t input_a[], uint16_t length);

int floorSqrt(int x);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

