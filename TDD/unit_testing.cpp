/*
 * unit_testing.cpp
 *
 *  Created on: 3 nov. 2018
 *      Author: Vincent
 */

#include <cmath>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Model_tdd.h"
#include "UDMatrix.h"
#include "kalman_ext.h"
#include "sine_fitter.h"
#include "fram.h"
#include "utils.h"

#include "order1_filter.h"

#define TEST_FILTRE_NB    15

#define TEST_ROLLOF_NB    48759

bool test_kalman_ext(void) {

	LOG_INFO("Testing Kalman...");

	int32_t num_periods = 20;
	float sampling_hz = 10;
	float omega = 2 * M_PI; // rad/s
	uint16_t numOfData = 300;

	sKalmanDescr descr;
	sKalmanExtFeed feed;

	descr.ker.ker_dim = 3;
	descr.ker.obs_dim = 1;

	feed.dt = 1 / sampling_hz;

	kalman_ext_init(&descr);

	feed.matZ.resize(1, 1);

	// set A (it must be the jacobian matrix)
	// first val: A1 = dState1 / dState[co]
	descr.ker.matA.unity();

//	descr.ker.matA.set(1, 2, -omega * feed.dt);
//	descr.ker.matA.set(2, 1, omega * feed.dt);

	descr.ker.matA.print();

	// H maps the state vector to the measurements
	// first val: H1 = dMesure1 / dState[co]
	// i.e. expected measurements knowing X
	descr.ker_ext.matH.ones(0.0);

	// set Q
	descr.ker.matQ.unity(1 / 20.);

	// set P
	descr.ker.matP.ones(900);

	// set R
	descr.ker.matR.unity(0.01);

	// set Z
	feed.matZ.ones(0.0);

	float val;
	for(int i=0;i < numOfData;i++)
	{

		val = 14;
		val += 3 * sinf(omega * i * num_periods / (numOfData * sampling_hz));
		val += -8 * cosf(omega * i * num_periods / (numOfData * sampling_hz));

		feed.matZ.set(0, 0, val);

		descr.ker_ext.matH.set(0, 0, 1);
		descr.ker_ext.matH.set(0, 1, sinf(omega * i * num_periods / (numOfData * sampling_hz)));
		descr.ker_ext.matH.set(0, 2, cosf(omega * i * num_periods / (numOfData * sampling_hz)));

		kalman_ext_feed(&descr, &feed);

//		UDMatrix res;
//		res = descr.ker.matX;
//		res.print();
	}

	LOG_INFO("Simulated pos: %f", val);

	return true;
}

bool test_kalman_lin(void) {

	LOG_INFO("Testing Kalman...");

	uint16_t numOfData = 30;
	float omega = 2 * M_PI; // speed in rad / s
	float sampling = 1; // in seconds

	sKalmanDescr descr;
	sKalmanExtFeed feed;

	descr.ker.ker_dim = 2;
	descr.ker.obs_dim = 1;

	feed.dt = sampling;

	kalman_lin_init(&descr);

	feed.matZ.resize(1, 1);

	descr.ker.matA.unity();

	descr.ker.matA.print();

	descr.ker.matC.set(0, 0, 1);

	// set Q
	descr.ker.matQ.unity(1 / 20.);

	// set P
	descr.ker.matP.ones(900);

	// set R
	descr.ker.matR.unity(0.1);

	float val;
	for(int i=0;i < numOfData;i++)
	{
		val = 22 +  5 * i;

		descr.ker.matA.set(0, 1, feed.dt);

		feed.matZ.ones(0.0);
		feed.matZ.set(0, 0, val);

		kalman_lin_feed(&descr, &feed);

//		UDMatrix res;
//		res = descr.ker.matX;
//		res.print();
	}

	return true;
}

bool test_sine_fitting(void) {

	LOG_INFO("Testing sine fitting...");

	uint16_t numOfData = 30;
	float * dataz = new float[numOfData];
	float omega = 2. * M_PI; // speed in rad / s
	float sampling = 0.1; // in seconds

	for(int i=0;i < numOfData;i++)
	{
	  dataz[i] = 125 + 7 * sin(omega * sampling * i + 0.2);
	}

	sSineFitterOuput output;
	sine_fitter_compute(dataz, omega, sampling, numOfData, &output);

	LOG_INFO("Res.: %f %f %f",
			output.alpha,
			output.beta,
			output.phi);

	LOG_INFO("Fitting OK");

	return true;
}

bool test_fram(void) {

	LOG_INFO("Testing FRAM...");

	fram_init_sensor();

	if (!u_settings.isConfigValid())
		return false;

	if (!u_settings.resetConfig())
		return false;

	if (!u_settings.isConfigValid())
		return false;

	LOG_INFO("FRAM OK");

	return true;
}

