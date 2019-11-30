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
#include "vl53l1_api.h"
#include "vl53l1_wrapper.h"
#include "segger_wrapper.h"

#define F(X)                     (const char*)(X)

// Timing budget set through VL53L1_SetMeasurementTimingBudgetMicroSeconds().
#define MEASUREMENT_BUDGET_MS			50

// Interval between measurements, set through
// VL53L1_SetInterMeasurementPeriodMilliSeconds(). According to the API user
// manual (rev 2), "the minimum inter-measurement period must be longer than the
// timing budget + 4 ms." The STM32Cube example from ST uses 500 ms, but we
// reduce this to 55 ms to allow faster readings.
#define INTER_MEASUREMENT_PERIOD_MS		75

static VL53L1_Dev_t                   dev;
static VL53L1_DEV                     Dev = &dev;

static uint8_t m_is_data_ready = 0;
static VL53L1_RangingMeasurementData_t RangingData;
static task_id_t m_vl_task_id = 0;

static void _printRangingData()
{

	VL53L1_Error status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
	if(!status)
	{
		LOG_INFO("RangeStatus           : %u ", RangingData.RangeStatus);
		LOG_INFO("RangeMilliMeter       : %u ", RangingData.RangeMilliMeter);
		LOG_INFO("SignalRateRtnMegaCps  : %f ", RangingData.SignalRateRtnMegaCps/65536.0);
		LOG_INFO("RAmbientRateRtnMegaCps: %f ", RangingData.AmbientRateRtnMegaCps/65336.0);
	} else {
		LOG_INFO("Measurement error");
	}
}

static void _int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	W_SYSVIEW_RecordEnterISR();

	// cancel task delay
	m_is_data_ready = 0xFFu;
	w_task_delay_cancel(m_vl_task_id);

	W_SYSVIEW_RecordEnterISR();
}

int vl53l1_wrapper__init(void) {

	uint8_t byteData;
	uint16_t wordData;

	m_vl_task_id = task_id_get();

	// This is the default 8-bit slave address (including R/W as the least
	// significant bit) as expected by the API. Note that the Arduino Wire library
	// uses a 7-bit address without the R/W bit instead (0x29 or 0b0101001).
	Dev->I2cDevAddr = 0x52;

	VL53L1_software_reset(Dev);

	VL53L1_RdByte(Dev, 0x010F, &byteData);
	LOG_INFO(F("VL53L1X Model_ID: 0x%02X"), byteData);

	VL53L1_RdByte(Dev, 0x0110, &byteData);
	LOG_INFO(F("VL53L1X Module_Type: 0x%02X"), byteData);

	VL53L1_RdWord(Dev, 0x010F, &wordData);
	LOG_INFO(F("VL53L1X: 0x%02X"), wordData);

	VL53L1_Error status;
	LOG_INFO(F("Autonomous Ranging Test"));
	status = VL53L1_WaitDeviceBooted(Dev);
	status = VL53L1_DataInit(Dev);
	status = VL53L1_StaticInit(Dev);
	status = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_SHORT);
	status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, MEASUREMENT_BUDGET_MS * 1000);
	status = VL53L1_SetInterMeasurementPeriodMilliSeconds(Dev, INTER_MEASUREMENT_PERIOD_MS);
	status = VL53L1_StartMeasurement(Dev);

	LOG_INFO(F("Status: %d"), status);

	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_PULLUP;
	in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;

	ret_code_t err_code = nrfx_gpiote_in_init(VL53_INT, &in_config, _int_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(VL53_INT, true);

	return status;
}


int vl53l1_wrapper__measure(void) {

	while (!m_is_data_ready) {
		w_task_delay(25);
	}
	m_is_data_ready = 0;

	VL53L1_Error status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
	_printRangingData();

	return status;
}

