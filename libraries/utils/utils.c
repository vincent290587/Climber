/* 
 * File:   utils.c
 * Author: vincent
 *
 * Created on October 27, 2015, 10:55 AM
 */


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "utils.h"
#include "segger_wrapper.h"


#define BATT_INT_RES                   0.155

#define FACTOR 100000.


float min(float val1, float val2) {
  if (val1 <= val2) return val1;
  else return val2;
}

float max(float val1, float val2) {
  if (val1 <= val2) return val2;
  else return val1;
}

double radians(double value) {
	return value * (double)M_PI / (double)180.;
}

double degrees(double value) {
	return value * (double)180. / (double)M_PI;
}

double sq(double value) {
	return value * value;
}

float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {

  if (b1_f == b1_i) return b2_i;
  
  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  return res;
}

float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {

  if (b1_f == b1_i) return b2_i;

  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  if (res < min(b2_i,b2_f)) res = min(b2_i,b2_f);
  if (res > max(b2_i,b2_f)) res = max(b2_i,b2_f);
  return res;
}


void calculePos (const char *nom, float *lat, float *lon) {

    static char tab[15];
    int iLat;
    int iLon;
    
    if (!nom) {
      return;
    }

    strncpy(tab, nom, 5);
    tab[5] = '\0';
    iLat = toBase10(tab);
    if (lat) {
        *lat = (float) iLat / FACTOR - 90.;
    }

    strncpy(tab, nom + 6, 2);
    strncpy(tab + 2, nom + 9, 3);
    tab[5] = '\0';
    iLon = toBase10(tab);
    if (lon) {
        *lon = (float) iLon / FACTOR - 180.;
    }

    return;
}


long unsigned int toBase10 (char *entree) {

    static char tab[15];

    if (!entree) {
        return 0;
    }
    
    if (!strstr(entree, ".")) {
        strncpy(tab, entree, 5);
        tab[5] = '\0';
    } else {
        strncpy(tab, entree, 2);
        strncpy(tab + 2, entree + 3, 3);
        tab[5] = '\0';
    }

    return strtoul(tab, NULL, 36);

}

uint32_t get_sec_jour(uint8_t hour_, uint8_t min_, uint8_t sec_)
{
  unsigned long res = 0;

  res = 3600 * hour_ + 60 * min_ + sec_;

  return res;
}


float compute2Complement(uint8_t msb, uint8_t lsb) {
	uint16_t t;
	uint16_t val;
	uint8_t tl=lsb, th=msb;
	float ret;

	if (th & 0b00100000) {
		t = th << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		val -= 1;
		val = ~(val | 0b1110000000000000);
		//LOG_INFO("Raw 2c1: %u\r\n", val);
		ret = (float)val;
	} else {
		t = (th & 0xFF) << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		//LOG_INFO("Raw 2c2: %u\r\n", val);
		ret = (float)-val;
	}

	return ret;
}

/**
 *
 * @param tensionValue in Volts
 * @param current
 * @return Percentage between 0 and 100
 */
float percentageBatt(float tensionValue, float current) {

    float fp_ = 0.f;

    tensionValue += current * BATT_INT_RES / 1000.;

    if (tensionValue > 4.2f) {
			fp_ = 100.f;
    } else if (tensionValue > 3.78f) {
        fp_ = 536.24f * tensionValue * tensionValue * tensionValue;
		fp_ -= 6723.8f * tensionValue * tensionValue;
        fp_ += 28186.f * tensionValue - 39402.f;

		if (fp_ > 100.f) fp_ = 100.f;

    } else if (tensionValue > 3.2f) {
        fp_ = powf(10.f, -11.4f) * powf(tensionValue, 22.315f);
    } else {
        fp_ = -1.f;
    }

    return fp_;
}


void encode_uint16 (uint8_t* dest, uint16_t input) {
	dest[0] = (uint8_t) (input & 0xFF);
	dest[1] = (uint8_t) ((input & 0xFF00) >> 8);
}

void encode_uint32 (uint8_t* dest, uint32_t input) {
	dest[0] = (uint8_t) (input & 0xFF);
	dest[1] = (uint8_t) ((input & 0xFF00) >> 8);
	dest[2] = (uint8_t) ((input & 0xFF0000) >> 16);
	dest[3] = (uint8_t) ((input & 0xFF000000) >> 24);
}

uint16_t decode_uint16 (uint8_t* dest) {
	uint16_t res = dest[0];
	res |=  dest[1] << 8;
	return res;
}

uint32_t decode_uint32 (uint8_t* dest) {
	uint32_t res = dest[0];
	res |=  dest[1] << 8;
	res |=  dest[2] << 16;
	res |=  dest[3] << 24;
	return res;
}

void const_char_to_buffer(const char *str_, uint8_t *buff_, uint16_t max_size) {

	memset(buff_, 0, max_size);

	for (uint8_t i=0; i < strlen(str_); i++) {

		if (i==max_size) {
			return;
		}

		buff_[i] = str_[i];

	}

}

/**
 * http://en.wikipedia.org/wiki/Simple_linear_regression
 *
 * @param x input arrau horizontal
 * @param y input arrau vertical
 * @param lrCoef slope=lrCoef[0] and intercept=lrCoef[1]
 * @param n length of the x and y arrays.
 *
 * @return Square of the correlation
 */
float simpLinReg(float* x, float* y, float* lrCoef, int n) {
	// pass x and y arrays (pointers), lrCoef pointer, and n.  The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1].  n is length of the x and y arrays.
	// http://en.wikipedia.org/wiki/Simple_linear_regression

	// initialize variables
	float xbar = 0;
	float ybar = 0;
	float xybar = 0;
	float xsqbar = 0;
	float ysqbar = 0;

	// calculations required for linear regression
	for (int i = 0; i < n; i++) {
		xbar = xbar + x[i];
		ybar = ybar + y[i];
		xybar = xybar + x[i] * y[i];
		xsqbar = xsqbar + x[i] * x[i];
		ysqbar = ysqbar + y[i] * y[i];
	}
	xbar = xbar / n;
	ybar = ybar / n;
	xybar = xybar / n;
	xsqbar = xsqbar / n;
	ysqbar = ysqbar / n;

	float corr = xybar - xbar*ybar;
	corr *= corr;
	float corr_denomsq = (xsqbar - xbar*xbar)*(ysqbar - ybar*ybar);

	// simple linear regression algorithm
	lrCoef[0] = (xybar - xbar * ybar) / (xsqbar - xbar * xbar);
	lrCoef[1] = ybar - lrCoef[0] * xbar;

	if (corr_denomsq == 0.0f) {
		corr = 1.0f;
	} else {
		corr /= corr_denomsq;
	}

	return corr;
}

/**
 * Calculates the XOR checksum of an array
 */
uint8_t calculate_crc(uint8_t input_a[], uint16_t length) {
	int sum = 0;
	for (int i = 1; i < length; i++) {
		sum ^= (uint8_t) input_a[i];
	}
	return sum;
}


/**
 * Returns floor of square root of x
 */
int floorSqrt(int x)
{
	// Base cases
	if (x == 0 || x == 1)
		return x;

	// Staring from 1, try all numbers until
	// i*i is greater than or equal to x.
	int i = 1, result = 1;
	while (result <= x)
	{
		i++;
		result = i * i;
	}
	return i - 1;
}
