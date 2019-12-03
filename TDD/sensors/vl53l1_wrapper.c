/*
 * vl53l1_wrapper.c
 *
 *  Created on: 30 nov. 2019
 *      Author: Vincent
 */


#include "gpio.h"
#include "boards.h"
#include "millis.h"
#include "helper.h"
#include "vl53l1_wrapper.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"

#define F(X)                     (const char*)(X)

// Timing budget set through VL53L1_SetMeasurementTimingBudgetMicroSeconds().
#define MEASUREMENT_BUDGET_MS			50

// Interval between measurements, set through
// VL53L1_SetInterMeasurementPeriodMilliSeconds(). According to the API user
// manual (rev 2), "the minimum inter-measurement period must be longer than the
// timing budget + 4 ms." The STM32Cube example from ST uses 500 ms, but we
// reduce this to 55 ms to allow faster readings.
#define INTER_MEASUREMENT_PERIOD_MS		80

static task_id_t m_vl_task_id = 0;
static uint32_t nb_error = 0;



static void _printRangingData(int status)
{

	if(!status)
	{
		nb_error = 0;

		//data_dispatcher__feed_distance((float)RangingData.RangeMilliMeter);

	} else {
		LOG_INFO("Measurement error %d", status);

		nb_error++;
	}
}

int vl53l1_wrapper__init(void) {

	m_vl_task_id = w_task_id_get();

	return 0;
}

#define VL53L1_RANGE_COMPLETION_POLLING_TIMEOUT_MS   2000

int vl53l1_wrapper__measure(void) {

	return 0;
}

