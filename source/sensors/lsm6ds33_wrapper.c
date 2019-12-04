/*
 * lsm6ds33_wrapper.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "i2c.h"
#include "gpio.h"
#include "boards.h"
#include "helper.h"
#include "lsm6ds3_reg.h"
#include "lsm6ds33_wrapper.h"
#include "data_dispatcher.h"
#include "nrf_twi_mngr.h"
#include "segger_wrapper.h"

#undef I2C_READ_REG
#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr >> 1, p_buffer, byte_cnt, 0)

#undef I2C_READ_REG_REP_STOP
#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_reg_addr, 1, 0), \
		NRF_TWI_MNGR_READ (addr >> 1, p_buffer, byte_cnt, 0)

#undef I2C_WRITE
#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr >> 1, p_data, byte_cnt, 0)

typedef union {
	int16_t i16bit[3];
	uint8_t u8bit[6];
} axis3bit16_t;

/* Private variables ---------------------------------------------------------*/
//static axis3bit16_t data_raw_acceleration;
//static axis3bit16_t data_raw_angular_rate;
static float acceleration_mg[3];
static float angular_rate_mdps[3];

static int32_t _lsm6_i2c_read(void *handle, uint8_t reg_addr, uint8_t *p_data, uint16_t length) {

	nrf_twi_mngr_transfer_t const xfer[] =
	{
			I2C_READ_REG(LSM6DS3_I2C_ADD_H, &reg_addr, p_data, length)
	};

	return i2c_perform(NULL, xfer, sizeof(xfer) / sizeof(xfer[0]), NULL);
}

static int32_t _lsm6_i2c_write(void *handle, uint8_t reg_addr, uint8_t *data, uint16_t length) {

	uint8_t p_data[30];

	ASSERT(length < 30 - 1);

	p_data[0] = reg_addr;
	for (int i=0; i< length; i++) {
		p_data[i+1] = data[i];
	}

	nrf_twi_mngr_transfer_t const xfer[] =
	{
			I2C_WRITE(LSM6DS3_I2C_ADD_H, p_data, length+1)
	};

	return i2c_perform(NULL, xfer, sizeof(xfer) / sizeof(xfer[0]), NULL);
}

static void _lsm6_readout_cb(ret_code_t result, void * p_user_data) {

	W_SYSVIEW_RecordEnterISR();

	axis3bit16_t data_raw[2];
	memcpy(&data_raw[0], p_user_data, 12);

	ASSERT(p_user_data);

	if (result) {
		LOG_WARNING("LSM6 read error");
		return;
	}

	acceleration_mg[0] = (float)(((int32_t)data_raw[0].i16bit[0] * 61) / 1000);
	acceleration_mg[1] = (float)(((int32_t)data_raw[0].i16bit[1] * 61) / 1000);
	acceleration_mg[2] = (float)(((int32_t)data_raw[0].i16bit[2] * 61) / 1000);

	angular_rate_mdps[0] = (float)(((int32_t)data_raw[1].i16bit[0] * 4375) / 1000);
	angular_rate_mdps[1] = (float)(((int32_t)data_raw[1].i16bit[1] * 4375) / 1000);
	angular_rate_mdps[2] = (float)(((int32_t)data_raw[1].i16bit[2] * 4375) / 1000);

	LOG_DEBUG("LSM6 read     : %d %d %d \r\n\t\t  %d %d %d ",
			data_raw[0].i16bit[0],
			data_raw[0].i16bit[1],
			data_raw[0].i16bit[2],
			data_raw[1].i16bit[0],
			data_raw[1].i16bit[1],
			data_raw[1].i16bit[2]);

	data_dispatcher__feed_acc(acceleration_mg, angular_rate_mdps);

	LOG_DEBUG("LSM6 read %u", millis());

	W_SYSVIEW_RecordExitISR();

}

void _lsm6_read_sensor(void) {

	// read all
	static uint8_t data_raw[15];

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND readout_reg[] = {
			LSM6DS3_OUTX_L_XL ,
			LSM6DS3_OUTX_L_G,
			LSM6DS3_STATUS_REG };

	static nrf_twi_mngr_transfer_t const lsm6_readout_transfer[] =
	{
			I2C_READ_REG(LSM6DS3_I2C_ADD_H, readout_reg  , data_raw	  , 6),
			I2C_READ_REG(LSM6DS3_I2C_ADD_H, readout_reg+1, data_raw+6 , 6),
			I2C_READ_REG(LSM6DS3_I2C_ADD_H, readout_reg+2, data_raw+12, 1),
	};

	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
	{
			.callback            = _lsm6_readout_cb,
			.p_user_data         = data_raw,
			.p_transfers         = lsm6_readout_transfer,
			.number_of_transfers = sizeof(lsm6_readout_transfer) / sizeof(lsm6_readout_transfer[0])
	};

	i2c_schedule(&transaction);

}

static void _int1_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	W_SYSVIEW_RecordEnterISR();

	// schedule sensor reading
	_lsm6_read_sensor();

	W_SYSVIEW_RecordEnterISR();
}

void lsm6ds33_wrapper__init(void) {
	/*
	 * Initialize mems driver interface
	 */
	stmdev_ctx_t dev_ctx;

	dev_ctx.write_reg	= _lsm6_i2c_write;
	dev_ctx.read_reg	= _lsm6_i2c_read;
	dev_ctx.handle		= NULL;

	/*
	 * Uncomment if interrupt generation on DRDY INT2 pin.
	 */
	lsm6ds3_int1_route_t int_1_reg;
	//lsm6ds3_int2_route_t int_2_reg;

	/*
	 * Check device ID
	 */
	uint8_t whoamI, rst;
	lsm6ds3_device_id_get(&dev_ctx, &whoamI);
	if (whoamI != LSM6DS3_ID)
		while(1)
		{
			/* manage here device not found */
			LOG_ERROR("whoamI wrong (0x%02X)", whoamI);
		}

	/*
	 * Restore default configuration
	 */
	lsm6ds3_reset_set(&dev_ctx, PROPERTY_ENABLE);
	do {
		lsm6ds3_reset_get(&dev_ctx, &rst);
	} while (rst);

	/*
	 * Enable Block Data Update
	 */
	lsm6ds3_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	/*
	 * Set full scale
	 */
	lsm6ds3_xl_full_scale_set(&dev_ctx, LSM6DS3_2g);
	lsm6ds3_gy_full_scale_set(&dev_ctx, LSM6DS3_125dps);

	/*
	 * Set Output Data Rate
	 */
	lsm6ds3_xl_data_rate_set(&dev_ctx, LSM6DS3_XL_ODR_12Hz5);
	lsm6ds3_gy_data_rate_set(&dev_ctx, LSM6DS3_GY_ODR_12Hz5);

	/*
	 * Enable interrupt generation on DRDY INT1 pin
	 */
	lsm6ds3_pin_int1_route_get(&dev_ctx, &int_1_reg);
	int_1_reg.int1_drdy_g = PROPERTY_ENABLE;
	int_1_reg.int1_drdy_xl = PROPERTY_ENABLE;
	int_1_reg.drdy_on_int1 = PROPERTY_ENABLE;
	lsm6ds3_pin_int1_route_set(&dev_ctx, &int_1_reg);

	LOG_INFO("LSM6 int %u", int_1_reg);

	lsm6ds3_lir_t int_type = LSM6DS3_INT_PULSED;
	lsm6ds3_int_notification_set(&dev_ctx, int_type);

	lsm6ds3_pin_pol_t pin_pol = LSM6DS3_ACTIVE_HIGH;
	lsm6ds3_pin_polarity_set(&dev_ctx, pin_pol);

	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_PULLDOWN;
	in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

	ret_code_t err_code = nrfx_gpiote_in_init(LSM6_INT1, &in_config, _int1_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(LSM6_INT1, true);

	nrf_gpio_cfg_sense_input(LSM6_INT1, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH);

	 _lsm6_read_sensor();
}
