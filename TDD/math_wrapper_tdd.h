/*
 * math_wrapper_tdd.h
 *
 *  Created on: 19 nov. 2018
 *      Author: Vincent
 */

#ifndef TDD_MATH_WRAPPER_TDD_H_
#define TDD_MATH_WRAPPER_TDD_H_


#include <math.h>
#include "assert_wrapper.h"


#ifndef _USE_MATH_DEFINES

#define M_E		2.7182818284590452354f
#define M_LOG2E		1.4426950408889634074f
#define M_LOG10E	0.43429448190325182765f
#define M_LN2		_M_LN2
#define M_LN10		2.30258509299404568402f
#define M_PI		3.14159265358979323846f
#define M_TWOPI         (M_PI * 2.0f)
#define M_PI_2		1.57079632679489661923f
#define M_PI_4		0.78539816339744830962f
#define M_3PI_4		2.3561944901923448370E0f
#define M_SQRTPI        1.77245385090551602792981f
#define M_1_PI		0.31830988618379067154f
#define M_2_PI		0.63661977236758134308f
#define M_2_SQRTPI	1.12837916709551257390f
#define M_SQRT2		1.41421356237309504880f
#define M_SQRT1_2	0.70710678118654752440f
#define M_LN2LO         1.9082149292705877000E-10f
#define M_LN2HI         6.9314718036912381649E-1f
#define M_SQRT3	1.73205080756887719000f
#define M_IVLN10        0.43429448190325182765f /* 1 / log(10) */
#define M_LOG2_E        _M_LN2
#define M_INVLN2        1.4426950408889633870E0f  /* 1 / log(2) */

#endif



static inline float my_sqrtf(float val) {
	ASSERT(val >= 0.);
	return sqrtf(val);
}

static inline float my_cos(float val) {
	return cosf(val);
}

static inline float my_sin(float val) {
	return sinf(val);
}





#endif /* TDD_MATH_WRAPPER_TDD_H_ */
