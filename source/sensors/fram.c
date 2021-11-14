/*
 * fram.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "fram.h"
#include "utils.h"
#include "boards.h"
#include "parameters.h"
#include "nordic_common.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#if defined(FRAM_PRESENT)

#include "i2c.h"
#include "nrf_twi_mngr.h"

#define FRAM_TWI_ADDRESS       0b10100000

#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fram_init_sensor() {

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	uint8_t twi_address = FRAM_TWI_ADDRESS;
	twi_address |= (block_addr & 0x100) >> 7;

	uint8_t address = block_addr & 0x0FF;

	{
		nrf_twi_mngr_transfer_t const fram_xfer[] =
		{
				I2C_READ_REG_REP_STOP(twi_address, &address, readout, length)
		};

		i2c_perform(NULL, fram_xfer, sizeof(fram_xfer) / sizeof(fram_xfer[0]), NULL);
	}

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint8_t twi_address = FRAM_TWI_ADDRESS;
	twi_address |= (block_addr & 0x100) >> 7;

	if (length > 254) return false;

	uint8_t p_data[256] = {0};
	p_data[0] = block_addr & 0x0FF;

	memcpy(&p_data[1], writeout, length);
	length++;

	{
		nrf_twi_mngr_transfer_t const fram_xfer[] =
		{
				I2C_WRITE(twi_address, p_data, length)
		};

		i2c_perform(NULL, fram_xfer, sizeof(fram_xfer) / sizeof(fram_xfer[0]), NULL);
	}

	return true;
}

#elif defined( FDS_PRESENT ) && !defined(TDD)


#include "fds.h"
#include "Model.h"
#include "UserSettings.h"

#define FDS_SUCCESS     NRF_SUCCESS

/* File ID and Key used for the configuration record. */

#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)

static task_id_t m_fram_task_id = TASK_ID_INVALID;

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
static bool volatile m_fds_wr_pending;


static void fds_evt_handler(fds_evt_t const * p_evt)
{
	switch (p_evt->id)
	{
	case FDS_EVT_INIT:
		if (p_evt->result == FDS_SUCCESS)
		{
			m_fds_initialized = true;
			LOG_WARNING("!! FDS init done !!");
		}
		break;

	case FDS_EVT_WRITE:
	{
		if (p_evt->result != FDS_SUCCESS)
		{
			LOG_ERROR("Record write error");
		} else {
			LOG_INFO("!! Record writen !! ");
		}
		m_fds_wr_pending = false;
	} break;

	case FDS_EVT_UPDATE:
	{
		if (p_evt->result != FDS_SUCCESS)
		{
			LOG_ERROR("Record update error");
		} else {
			LOG_INFO("!! Record updated !! ");
		}
		m_fds_wr_pending = false;
	} break;

	default:
		break;
	}

	w_task_delay_cancel(m_fram_task_id);
}

void fram_init_sensor() {

	ret_code_t rc;

	m_fram_task_id = w_task_id_get();

	/* Register first to receive an event when initialization is complete. */
	rc = fds_register(fds_evt_handler);
	APP_ERROR_CHECK(rc);

	LOG_INFO("FRAM init pending...");
	LOG_FLUSH();

	m_fds_initialized = false;

	rc = fds_init();
	APP_ERROR_CHECK(rc);

	w_task_delay(200);

	if (!m_fds_initialized) {

		// timeout
		LOG_ERROR("FRAM init timeout");
		return;
	}

	LOG_WARNING("FRAM init done");

	fds_gc();

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	fds_record_desc_t desc = {0};
	fds_find_token_t  tok  = {0};

	ASSERT(m_fds_initialized);

	m_fram_task_id = w_task_id_get();

	LOG_DEBUG("Reading config file...");

	ret_code_t rc = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);

	if (rc == FDS_SUCCESS)
	{
		/* A config file is in flash. Let's update it. */
		fds_flash_record_t config = {0};

		/* Open the record and read its contents. */
		rc = fds_record_open(&desc, &config);
		APP_ERROR_CHECK(rc);

		if (rc) return false;

		/* Copy the configuration from flash into readout. */
		length = MIN(sizeof(sUserParameters), length);
		memcpy(readout, config.p_data, length);

		/* Close the record when done reading. */
		rc = fds_record_close(&desc);
		APP_ERROR_CHECK(rc);

		return true;
	}

	LOG_ERROR("FDS record not found");

	return false;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

	ASSERT(m_fds_initialized);

	m_fram_task_id = w_task_id_get();

	LOG_WARNING("FDS write start");

	fds_record_t _record =
	{
			.file_id           = CONFIG_FILE,
			.key               = CONFIG_REC_KEY,
			.data.p_data       = writeout,
			/* The length of a record is always expressed in 4-byte units (words). */
			.data.length_words = (length + 3) / sizeof(uint32_t),
	};

	m_fds_wr_pending = true;

	ret_code_t rc = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);
	if (rc == FDS_SUCCESS) {

        /* Write the updated record to flash. */
        rc = fds_record_update(&desc, &_record);
        APP_ERROR_CHECK(rc);
	} else {

		/* System config not found; write a new one. */
		rc = fds_record_write(&desc, &_record);
		APP_ERROR_CHECK(rc);
	}

	uint16_t timeout = 0;
	while (m_fds_wr_pending) {

		w_task_delay(20);

		if (timeout++ > 100) {
			return false;
		}
	}

	fds_gc();

	return true;
}

#else

#include "g_structs.h"
#include "UserSettings.h"

static sUserParameters m_params;


void fram_init_sensor() {

	m_params.version = 1U;

	m_params.crc = calculate_crc(m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(sUserParameters), length);

	LOG_INFO("FRAM reading %u bytes", res_len);

	// copy as much as we can
	memcpy(readout, m_params.flat_user_params, res_len);

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(sUserParameters), length);

	LOG_INFO("FRAM writing %u bytes", res_len);

	// copy as much as we can
	memcpy(m_params.flat_user_params, writeout, res_len);

	return true;
}

#endif
