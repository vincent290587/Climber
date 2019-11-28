/*
 * lsm6ds33_wrapper.c
 *
 *  Created on: 28 nov. 2019
 *      Author: Vincent
 */

#include <stdint.h>
#include "lsm6ds3_reg.h"


void lsm6ds33_wrapper__init(void) {
  /*
   * Initialize mems driver interface
   */
  stmdev_ctx_t dev_ctx;
  lsm6ds3_int1_route_t int_1_reg;

  /*
   * Uncomment if interrupt generation on DRDY INT2 pin.
   */
  //lsm6ds3_int2_route_t int_2_reg;

  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hi2c1;

  /*
   * Initialize platform specific hardware
   */
  platform_init();

  /*
   * Check device ID
   */
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

}
