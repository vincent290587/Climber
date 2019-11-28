/*
 * bsc.c
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "Model.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"

#include "ant.h"
#include "nordic_common.h"
#include "app_error.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "app_timer.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_key_manager.h"
#include "ant_search_config.h"
#include "ant_bsc.h"
#include "ant_bsc_simulator.h"
#include "ant_interface.h"

#include "Model.h"
#include "segger_wrapper.h"

#include "bsc.h"

#define ANT_DELAY                       APP_TIMER_TICKS(30000)


static void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event);


BSC_SENS_CHANNEL_CONFIG_DEF(m_ant_bsc,
                            BSC_CHANNEL_NUM,
                            CHAN_ID_TRANS_TYPE,
                            SENSOR_TYPE,
                            CHAN_ID_DEV_NUM,
                            ANTPLUS_NETWORK_NUM);
BSC_SENS_PROFILE_CONFIG_DEF(m_ant_bsc,
                            true,
                            true,
							ANT_BSC_COMB_PAGE_0,
                            ant_bsc_evt_handler);

static ant_bsc_profile_t  m_ant_bsc;    /**< Simulator used to simulate profile data. */

/** @snippet [ANT BSC TX Instance] */

static ant_bsc_simulator_t  m_ant_bsc_simulator;    /**< Simulator used to simulate profile data. */

/**
 *
 */
static void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event)
{
	LOG_DEBUG("BSC event %u", event);

	W_SYSVIEW_RecordEnterISR();

	switch (event)
	{
	case ANT_BSC_PAGE_0_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_1_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_2_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_3_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_4_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_5_UPDATED:
	{
		LOG_INFO("BSC sending page 5");
		/* Log computed value */
	} break;

	case ANT_BSC_COMB_PAGE_0_UPDATED:
	{
		LOG_INFO("BSC sending combined page 0");

		m_ant_bsc_simulator._cb.auto_change = false;

		// speed
		m_ant_bsc_simulator._cb.sensorsim_s_state.current_val = 0;

		// cadence
		uint32_t cadence = 0;
		data_dispatcher_get_cadence(&cadence);
		m_ant_bsc_simulator._cb.sensorsim_c_state.current_val = cadence;

	    // battery voltage
		uint32_t batt_mv = 0;
		data_dispatcher_get_batt_volt(&batt_mv);
		m_ant_bsc_simulator.p_profile->BSC_PROFILE_coarse_bat_volt = batt_mv / 1000;
	    m_ant_bsc_simulator.p_profile->BSC_PROFILE_fract_bat_volt  = batt_mv % 1000;
	    m_ant_bsc_simulator.p_profile->BSC_PROFILE_bat_status      = BSC_BAT_STATUS_GOOD;

		ant_bsc_simulator_one_iteration(&m_ant_bsc_simulator);
	} break;

	default:
		break;
	}

	W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
void ant_evt_bsc (ant_evt_t * p_ant_evt)
{
	switch (p_ant_evt->event)
	{
	case EVENT_TX:
		ant_bsc_sens_evt_handler(p_ant_evt, &m_ant_bsc);
		break;

	default:
		break;
	}

}

/**@brief Function for initializing the timer module.
 */
void bsc_init(void)
{
	/** @snippet [ANT BSC simulator init] */
    const ant_bsc_simulator_cfg_t simulator_cfg =
    {
        .p_profile      = &m_ant_bsc,
        .device_type    = SENSOR_TYPE,
    };
    /** @snippet [ANT BSC simulator init] */

    ant_bsc_simulator_init(&m_ant_bsc_simulator, &simulator_cfg, true);
}

/**
 *
 */
void bsc_profile_setup(void) {

	ret_code_t err_code;

    err_code = ant_bsc_sens_init(&m_ant_bsc,
                                 BSC_SENS_CHANNEL_CONFIG(m_ant_bsc),
                                 BSC_SENS_PROFILE_CONFIG(m_ant_bsc));
    APP_ERROR_CHECK(err_code);

    m_ant_bsc.BSC_PROFILE_manuf_id     = BSC_MF_ID;
    m_ant_bsc.BSC_PROFILE_serial_num   = BSC_SERIAL_NUMBER;
    m_ant_bsc.BSC_PROFILE_hw_version   = BSC_HW_VERSION;
    m_ant_bsc.BSC_PROFILE_sw_version   = BSC_SW_VERSION;
    m_ant_bsc.BSC_PROFILE_model_num    = BSC_MODEL_NUMBER;

}

/**
 *
 */
void bsc_profile_start(void) {

	ret_code_t err_code;

    err_code = ant_bsc_sens_open(&m_ant_bsc);
    APP_ERROR_CHECK(err_code);
}

/**
 *
 */
void bsc_profile_stop(void) {

	ret_code_t err_code;

    err_code = sd_ant_channel_close(m_ant_bsc.channel_number);
    APP_ERROR_CHECK(err_code);
}


#endif
