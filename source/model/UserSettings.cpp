/*
 * UserSettings.cpp
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "utils.h"
#include "fram.h"
#include "boards.h"
#include "Model.h"
#include "parameters.h"
#include "UserSettings.h"
#include "segger_wrapper.h"

static sUserParameters s_m_params;


sUserParameters *user_settings_get(void) {
	return &s_m_params;
}


UserSettings::UserSettings() : m_params(s_m_params) {
	m_is_init = false;
}

bool UserSettings::isConfigValid(void) {

	if (!m_is_init) {

		if (!fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters)))
			return false;

		m_is_init = true;
	}

	uint8_t crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	if (crc != m_params.crc) {
		LOG_ERROR("User parameters: wrong CRC, found 0x%02X", crc);
		return false;
	}

	if (FRAM_SETTINGS_VERSION != m_params.version) {
		LOG_ERROR("User parameters: wrong version, found %u", m_params.version);
	} else {
		LOG_WARNING("User parameters V%u read correctly", m_params.version);
	}

	return true;
}

bool UserSettings::writeConfig(void) {

	m_params.version = FRAM_SETTINGS_VERSION;
	m_params.crc     = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	LOG_WARNING("User params written");

	return fram_write_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters));
}

void UserSettings::resetConfig(void) {

	memset(&m_params, 0, sizeof(sUserParameters));

	m_params.calibration = 300;

	LOG_WARNING("User params factory reset");

	return;
}

void UserSettings::checkConfigVersion(void) {

	if (fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters))) {

		uint8_t crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

		if (crc != m_params.crc) {
			this->resetConfig();

			LOG_WARNING("Basic user params set");
		} else {
			m_is_init = true;
		}
	} else {

#ifdef FDS_PRESENT
		// can be here if flash page not found
		// reset config
		this->resetConfig();
		if (!this->writeConfig()) {

			LOG_ERROR("Write params error");
		}
#endif
	}
}
