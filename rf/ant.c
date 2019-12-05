

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ant.h"
#include "nordic_common.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_sdh.h"
#include "app_timer.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_key_manager.h"
#include "ant_search_config.h"
#include "ant_fec.h"
#include "ant_interface.h"

#include "Model.h"
#include "segger_wrapper.h"

#include "fec.h"


/**< Application's ANT observer priority. You shouldn't need to modify this value. */
#define APP_ANT_OBSERVER_PRIO       1


/**
 * Event handler for background search
 */
static void ant_evt_bs (ant_evt_t * p_ant_evt)
{
	ret_code_t err_code = NRF_SUCCESS;

	switch (p_ant_evt->event)
	{
	case EVENT_RX:
	{
		uint16_t m_last_device_id;
		uint8_t m_last_rssi = 0;

        m_last_rssi = p_ant_evt->message.ANT_MESSAGE_aucExtData[5];
        m_last_device_id = uint16_decode(p_ant_evt->message.ANT_MESSAGE_aucExtData);

        if (m_last_device_id)
        {
        	m_last_device_id = uint16_decode(p_ant_evt->message.ANT_MESSAGE_aucExtData);

    		LOG_WARNING("Dev. ID 0x%04X %d", m_last_device_id, (int8_t)m_last_rssi);

        }

	} break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		break;
	case EVENT_RX_SEARCH_TIMEOUT:
		break;
	case EVENT_CHANNEL_CLOSED:
		break;
	}

	APP_ERROR_CHECK(err_code);
}


/**@brief Function for dispatching a ANT stack event to all modules with a ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after a ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
void ant_evt_handler(ant_evt_t * p_ant_evt, void * p_context)
{
	W_SYSVIEW_RecordEnterISR();

	switch(p_ant_evt->channel) {

	case FEC_CHANNEL_NUMBER:
		ant_evt_fec (p_ant_evt);
		break;

	case BS_CHANNEL_NUMBER:
		ant_evt_bs (p_ant_evt);
		break;

	default:
		break;
	}

    W_SYSVIEW_RecordExitISR();
}

NRF_SDH_ANT_OBSERVER(m_ant_observer, APP_ANT_OBSERVER_PRIO, ant_evt_handler, 0);

/**@brief Function for initializing the timer module.
 */
void ant_timers_init(void)
{
	fec_init();
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ant_stack_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_ant_enable();
	APP_ERROR_CHECK(err_code);

	err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUMBER);
	APP_ERROR_CHECK(err_code);

}

/**@brief Function for HRM profile initialization.
 *
 * @details Initializes the HRM profile and open ANT channel.
 */
static void ant_profile_setup(void)
{

	// FEC
	fec_profile_setup();

    // BS
	const ant_search_config_t bs_search_config =
	{
			.channel_number        = BS_CHANNEL_NUMBER,
			.low_priority_timeout  = ANT_LOW_PRIORITY_TIMEOUT_DISABLE,
			.high_priority_timeout = 80,
			.search_sharing_cycles = ANT_SEARCH_SHARING_CYCLES_DISABLE,
			.search_priority       = ANT_SEARCH_PRIORITY_DEFAULT,
			.waveform              = ANT_WAVEFORM_DEFAULT,
	};

    // Background search
    const ant_channel_config_t bs_channel_config =
    {
        .channel_number    = BS_CHANNEL_NUMBER,
        .channel_type      = CHANNEL_TYPE_SLAVE,
        .ext_assign        = EXT_PARAM_ALWAYS_SEARCH,
        .rf_freq           = 0x39u,              	// ANT+ frequency
        .transmission_type = WILDCARD_TRANSMISSION_TYPE,
        .device_type       = 0x00,   // Wild card
        .device_number     = 0x00,              // Wild card
        .channel_period    = 0x00,              // This is not taken into account.
        .network_number    = ANTPLUS_NETWORK_NUMBER,
    };

	ret_code_t err_code;
    err_code = ant_channel_init(&bs_channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = ant_search_init(&bs_search_config);
    APP_ERROR_CHECK(err_code);

    //    uint8_t pucANTLibConfig;
    //    err_code = sd_ant_lib_config_get (&pucANTLibConfig);
    //    APP_ERROR_CHECK(err_code);
    //
    //    err_code = sd_ant_lib_config_set (pucANTLibConfig | ANT_LIB_CONFIG_MESG_OUT_INC_RSSI);
    //    APP_ERROR_CHECK(err_code);
}


void ant_search_start(eAntPairingSensorType search_type) {

	ret_code_t err_code;

    err_code = sd_ant_channel_open(BS_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);

	switch (search_type) {
	case eAntPairingSensorTypeNone:
		break;
	case eAntPairingSensorTypeHRM:
		break;
	case eAntPairingSensorTypeBSC:
		break;
	case eAntPairingSensorTypeFEC:
		break;
	default:
		break;
	}

}

void ant_search_end(eAntPairingSensorType search_type, uint16_t dev_id) {

	ret_code_t err_code;

    err_code = sd_ant_channel_close(BS_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);

//    err_code = sd_ant_channel_unassign(BS_CHANNEL_NUMBER);
//    APP_ERROR_CHECK(err_code);

	if (!dev_id) return;

	switch (search_type) {
	case eAntPairingSensorTypeNone:
	    break;

	case eAntPairingSensorTypeFEC:
	{
		// Set the new device ID.
		ret_code_t err_code = sd_ant_channel_id_set(FEC_CHANNEL_NUMBER,
				dev_id,
				FEC_DEVICE_TYPE,
				WILDCARD_TRANSMISSION_TYPE);
		APP_ERROR_CHECK(err_code);
	} break;
	default:
		break;
	}
}

/**
 *
 */
void ant_setup_init(void) {

	ant_profile_setup();

}

/**
 *
 */
void ant_setup_start(void)
{
	ret_code_t err_code;

	uint16_t fec_id = TACX_DEVICE_NUMBER;

	// Set the new device ID.
	err_code = sd_ant_channel_id_set(FEC_CHANNEL_NUMBER,
			fec_id,
			FEC_DEVICE_TYPE,
			WILDCARD_TRANSMISSION_TYPE);
	APP_ERROR_CHECK(err_code);

	// Open the ANT channels
	fec_profile_start();

//	glasses_profile_start();

	LOG_INFO("ANT started");
}

void ant_setup_stop(void) {

}

/**
 *
 */
void ant_tasks(void) {

	// check all channels are open
//	for (eAntSensorsChannelNumber channel = eAntSensorsChannelHRM; channel < eAntSensorsChannelBS; channel++) {
//
//		uint8_t status = 0;
//		sd_ant_channel_status_get((uint8_t)channel, &status);
//
//		if (STATUS_SEARCHING_CHANNEL != status &&
//				STATUS_TRACKING_CHANNEL != status) {
//
//			// channel is not searching and not tracking...
//			// TODO smth
//			LOG_WARNING("Channel %u status is weird...", (uint8_t)channel);
//		}
//
//	}

}


#endif
