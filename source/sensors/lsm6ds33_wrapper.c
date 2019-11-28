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
static bool m_is_updated = false;

static int32_t _lsm6_i2c_read(void *handle, uint8_t reg_addr, uint8_t *p_data, uint16_t length) {

	nrf_twi_mngr_transfer_t const xfer[] =
	{
			I2C_READ_REG(LSM6DS3_I2C_ADD_L, &reg_addr, p_data, length)
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
			I2C_WRITE(LSM6DS3_I2C_ADD_L, p_data, length+1)
	};

	return i2c_perform(NULL, xfer, sizeof(xfer) / sizeof(xfer[0]), NULL);
}

static void _lsm6_readout_cb(ret_code_t result, void * p_user_data) {

	W_SYSVIEW_RecordEnterISR();

	axis3bit16_t *data_raw = (axis3bit16_t*)p_user_data;

	ASSERT(p_user_data);

	if (result) {
		LOG_WARNING("LSM6 read error");
		return;
	}

	acceleration_mg[0] = lsm6ds3_from_fs2g_to_mg(data_raw[0].i16bit[0]);
	acceleration_mg[1] = lsm6ds3_from_fs2g_to_mg(data_raw[0].i16bit[1]);
	acceleration_mg[2] = lsm6ds3_from_fs2g_to_mg(data_raw[0].i16bit[2]);

	angular_rate_mdps[0] = lsm6ds3_from_fs2000dps_to_mdps(data_raw[1].i16bit[0]);
	angular_rate_mdps[1] = lsm6ds3_from_fs2000dps_to_mdps(data_raw[1].i16bit[1]);
	angular_rate_mdps[2] = lsm6ds3_from_fs2000dps_to_mdps(data_raw[1].i16bit[2]);

	m_is_updated = true;

	LOG_DEBUG("LSM6 read");

	W_SYSVIEW_RecordExitISR();

}

static void _lsm6_read_sensor(void) {

	// read all
	static axis3bit16_t data_raw[2];

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND readout_reg[] = { LSM6DS3_OUTX_L_XL };

	static nrf_twi_mngr_transfer_t const lsm6_readout_transfer[] =
	{
			I2C_READ_REG(LSM6DS3_I2C_ADD_L, readout_reg, data_raw , 12)
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

	// clear trigger
	gpio_clear(LIS_INT2);

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
	lsm6ds3_gy_full_scale_set(&dev_ctx, LSM6DS3_2000dps);

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
	lsm6ds3_pin_int1_route_set(&dev_ctx, &int_1_reg);

	nrfx_gpiote_in_config_t in_config;
	in_config.is_watcher = true;
	in_config.hi_accuracy = true;
	in_config.skip_gpio_setup = false;
	in_config.pull = NRF_GPIO_PIN_PULLDOWN;
	in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;

	ret_code_t err_code = nrfx_gpiote_in_init(LIS_INT1, &in_config, _int1_handler);
	APP_ERROR_CHECK(err_code);

	nrfx_gpiote_in_event_enable(LIS_INT1, true);

	LOG_ERROR("LSM6 Init success");

}
