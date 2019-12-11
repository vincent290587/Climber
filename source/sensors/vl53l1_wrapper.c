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

static VL53L1_Dev_t                   dev;
static VL53L1_DEV                     Dev = &dev;

static uint8_t m_is_data_ready = 0;
static VL53L1_RangingMeasurementData_t RangingData;
static task_id_t m_vl_task_id = TASK_ID_INVALID;
static uint32_t nb_error = 0;

//#define I2C_TEST
#ifdef I2C_TEST
int rd_write_verification( VL53L1_Dev_t *dev, uint16_t addr, uint32_t expected_value)
{
	VL53L1_Error Status  = VL53L1_ERROR_NONE;
	uint8_t bytes[4],mbytes[4];
	uint16_t words[2];
	uint32_t dword;
	int i;
	VL53L1_ReadMulti(dev, addr, mbytes, 4);
	for (i=0; i<4; i++){ VL53L1_RdByte(dev, addr+i, &bytes[i]); }
	for (i=0; i<2; i++){ VL53L1_RdWord(dev, addr+i*2, &words[i]); }
	Status = VL53L1_RdDWord(dev, addr, &dword);

	LOG_INFO("expected   = %8x,",expected_value);
	LOG_INFO("read_multi = %2x, %2x, %2x, %2x", mbytes[0],mbytes[1],mbytes[2],mbytes[3]);
	LOG_INFO("read_bytes = %2x, %2x, %2x, %2x", bytes[0],bytes[1],bytes[2],bytes[3]);
	LOG_INFO("read words = %4x, %4x",words[0],words[1]);
	LOG_INFO("read dword = %8x",dword);

	w_task_delay(5);

	if((mbytes[0]<<24 | mbytes[1]<<16 | mbytes[2]<<8 | mbytes[3]) != expected_value) return (-1);
	if((bytes[0]<<24 | bytes[1]<<16 | bytes[2]<<8 | bytes[3]) != expected_value) return (-1);
	if((words[0]<<16 | words[1]) != expected_value) return (-1);
	if(dword != expected_value) return(-1);
	return Status;

}
#define REG 0x3A
#define WRITE_CHECK   135
void i2c_test(VL53L1_Dev_t *dev)
{
	VL53L1_Error Status  = VL53L1_ERROR_NONE;
	int err_count = 0;
	uint8_t buff[4] = {0x11,0x22,0x33,0x44};
	uint8_t long_out[135] ={0x29, 0x02, 0x10, 0x00, 0x22, 0xBC, 0xCC, 0x81, 0x80, 0x07, 0x16, 0x00, 0xFF, 0xFD,
							0xF7, 0xDE, 0xFF, 0x0F, 0x00, 0x15, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x44, 0x00, 0x2C, 0x00, 0x11, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x11, 0x02, 0x00, 0x02, 0x08, 0x00, 0x08, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF,
							0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0B, 0x00, 0x00, 0x02, 0x14, 0x21, 0x00, 0x00,
							0x02, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x38, 0xFF, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00,
							0x9D, 0x07, 0x00, 0xD2, 0x05, 0x01, 0x68, 0x00, 0xC0, 0x08, 0x38, 0x00, 0x00, 0x00, 0x00, 0x03,
							0xB6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x05, 0x06, 0x06, 0x01, 0x00, 0x02,
							0xC7, 0xFF, 0x8B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x40};
	uint8_t long_in[135]= {0xff};
  	int i=0;

	//Status = rd_write_verification(dev, 0x10f, 0xeacc10ff);			// verify the Chip ID works

	w_task_delay(4000);

	Status += VL53L1_WriteMulti(dev, 0x01,  long_out, WRITE_CHECK);			// check if WriteMulti can write 135 bytes
	Status += VL53L1_ReadMulti(dev, 0x01,  long_in, WRITE_CHECK);			// check if WriteMulti can read 135 bytes

	if (Status) LOG_INFO("i2c test1 failed - please check it. Status = %d\n", Status);
	Status = 0;

	w_task_delay(5);

	err_count = 0;
	for (i=0; i<WRITE_CHECK; i++) if(long_in[i] != long_out[i])err_count++;
	if (err_count > 10) Status++;

	LOG_INFO("i2c test1 - err_count = %d\n", err_count);
	err_count = 0;

	Status += VL53L1_WriteMulti(dev, REG,  buff, 4);				// check WriteMulti
	if (rd_write_verification(dev, REG, 0x11223344) <0) err_count++;

	if (Status) LOG_INFO("i2c test2 failed - please check it. Status = %d\n", Status);
	Status = 0;
	LOG_INFO("i2c test2 - err_count = %d\n", err_count);
	err_count = 0;

	w_task_delay(5);

	Status += VL53L1_WrDWord(dev, REG, 0xffeeddcc);				// check WrDWord
	if (rd_write_verification(dev, REG, 0xffeeddcc) <0) err_count++;

	if (Status) LOG_INFO("i2c test3 failed - please check it. Status = %d\n", Status);
	Status = 0;
	LOG_INFO("i2c test3 - err_count = %d\n", err_count);
	err_count = 0;

	w_task_delay(5);

	Status += VL53L1_WrWord(dev, REG, 0x5566);					// check WrWord
	Status += VL53L1_WrWord(dev, REG+2, 0x7788);
	if (rd_write_verification(dev, REG, 0x55667788) <0) err_count++;

	if (Status) LOG_INFO("i2c test4 failed - please check it. Status = %d\n", Status);
	Status = 0;
	LOG_INFO("i2c test4 - err_count = %d\n", err_count);
	err_count = 0;

	w_task_delay(5);

	for (i=0; i<4; i++){ VL53L1_WrByte (dev, REG+i, buff[i]); }
	if (rd_write_verification(dev, REG,0x11223344) <0) Status++;
	if (Status > 0)
	{
		LOG_INFO("i2c test failed - please check it. Status = %d\n", Status);
	}

	w_task_delay(4000);
}

#endif


static void _sensor_init(void) {

	uint8_t byteData;
	uint16_t wordData;

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

	//i2c_test(Dev);

	VL53L1_Error status;
	LOG_INFO(F("Autonomous Ranging Test"));
	status = VL53L1_WaitDeviceBooted(Dev);
	status = VL53L1_DataInit(Dev);
	status = VL53L1_StaticInit(Dev);
	status = VL53L1_set_interrupt_polarity(Dev, VL53L1_DEVICEINTERRUPTPOLARITY_ACTIVE_LOW);
	status = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_LONG);
	status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, MEASUREMENT_BUDGET_MS * 1000);
	status = VL53L1_SetInterMeasurementPeriodMilliSeconds(Dev, INTER_MEASUREMENT_PERIOD_MS);

	w_task_delay(10);

	status = VL53L1_StartMeasurement(Dev);
	LOG_INFO(F("Status: %d"), status);

	w_task_delay(10);

	uint32_t pMeasurementTimingBudgetMicroSeconds = 0;
	status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(Dev,
			&pMeasurementTimingBudgetMicroSeconds);
	LOG_INFO(F("Budget: %lu"), pMeasurementTimingBudgetMicroSeconds);

	uint32_t InterMeasurementPeriodMilliSeconds = 0;
	status = VL53L1_GetInterMeasurementPeriodMilliSeconds(Dev,
			&InterMeasurementPeriodMilliSeconds);
	LOG_INFO(F("Inter meas: %lu"), InterMeasurementPeriodMilliSeconds);

	w_task_delay(10);
}

static void _printRangingData(VL53L1_Error status)
{

	if(!status)
	{
		nb_error = 0;

		data_dispatcher__feed_distance((float)RangingData.RangeMilliMeter);

		LOG_INFO("RangeStatus           : %u ", RangingData.RangeStatus);
		LOG_INFO("RangeMilliMeter       : %u ", RangingData.RangeMilliMeter);
		LOG_DEBUG("SignalRateRtnMegaCps  : %d ", (int)(RangingData.SignalRateRtnMegaCps/65536.0));
		LOG_DEBUG("RAmbientRateRtnMegaCps: %d ", (int)(RangingData.AmbientRateRtnMegaCps/65336.0));
	} else {
		LOG_INFO("Measurement error %d", status);
		nb_error++;

		if (nb_error > 10) {
			nb_error = 0;
			_sensor_init();
		}
	}
}

static void _int_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	W_SYSVIEW_RecordEnterISR();

	LOG_DEBUG("vl53l1 int %u", millis());

	// cancel task delay
	m_is_data_ready = 0xFFu;
	w_task_delay_cancel(m_vl_task_id);

	W_SYSVIEW_RecordEnterISR();
}

int vl53l1_wrapper__init(void) {

	m_vl_task_id = w_task_id_get();

	_sensor_init();

	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_NOPULL;
	in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;

	ret_code_t err_code = nrfx_gpiote_in_init(VL53_INT, &in_config, _int_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(VL53_INT, true);

	nrf_gpio_cfg_sense_input(VL53_INT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);

	return 0;
}

#define VL53L1_RANGE_COMPLETION_POLLING_TIMEOUT_MS   2000

int vl53l1_wrapper__measure(void) {

	VL53L1_Error status;
	static uint32_t startMs = 0;

	while (!m_is_data_ready) {

		if (millis() - startMs > VL53L1_RANGE_COMPLETION_POLLING_TIMEOUT_MS) {

			startMs = millis();
			status = VL53L1_ClearInterruptAndStartMeasurement(Dev);
			LOG_INFO(F("clearing int %d"), status);
		}

		w_task_delay(20);
	}

	startMs = millis();

	// non-blocking check for data ready
//	uint8_t isReady = 0;
//	status = VL53L1_GetMeasurementDataReady(Dev, &isReady);
//
//	LOG_INFO(F("vl53l1_wrapper__measure %u %u"), isReady, millis());
//
//	if (!isReady) {
//
//		if (millis() - startMs > VL53L1_RANGE_COMPLETION_POLLING_TIMEOUT_MS) {
//
//			startMs = millis();
//			status = VL53L1_ClearInterruptAndStartMeasurement(Dev);
//			LOG_INFO(F("clearing int %d"), status);
//		}
//
//		return 1;
//	}

	m_is_data_ready = 0;

	status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
	_printRangingData(status);

	VL53L1_ClearInterruptAndStartMeasurement(Dev);

	return status;
}

