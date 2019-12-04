/*
 * I2C.h
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#ifndef DRIVERS_I2C_H_
#define DRIVERS_I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include "parameters.h"

#ifndef TDD
#include "nrf_twi_mngr.h"


#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
    NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
    NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)

#define I2C_WRITE_CONT(addr, p_data, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, NRF_TWI_MNGR_NO_STOP)
#endif


#ifdef __cplusplus
extern "C" {
#endif


void i2c_init(void);

void i2c_uninit(void);

void i2c_scan(void);


#ifndef TDD
void i2c_schedule(nrf_twi_mngr_transaction_t const * p_transaction);

uint32_t i2c_perform(nrf_drv_twi_config_t const *    p_config,
        nrf_twi_mngr_transfer_t const * p_transfers,
        uint8_t                         number_of_transfers,
        void                            (* user_function)(void));
#endif

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_I2C_H_ */
