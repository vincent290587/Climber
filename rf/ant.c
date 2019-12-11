

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
#include "data_dispatcher.h"
#include "segger_wrapper.h"



/**< Application's ANT observer priority. You shouldn't need to modify this value. */
#define APP_ANT_OBSERVER_PRIO       1

static ant_fec_profile_t m_fec_profile;


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

        ant_fec_disp_evt_handler(p_ant_evt, &m_fec_profile);

        const ant_fec_message_layout_t * p_fec_message_payload = (ant_fec_message_layout_t *)p_ant_evt->message.ANT_MESSAGE_aucPayload;
        if (p_fec_message_payload->page_number == 51) {

            LOG_DEBUG("FEC rx page: 0x%02X %u", p_ant_evt->message.ANT_MESSAGE_ucMesgID, p_fec_message_payload->page_number);

        	uint16_t grade_slope = m_fec_profile.page_51.grade_slope;
        	float f_grade = (float)((int32_t)grade_slope * ANT_FEC_PAGE51_SLOPE_LSB - 200);
        	data_dispatcher__feed_target_slope(f_grade);
        }


	} break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		break;
	case EVENT_RX_SEARCH_TIMEOUT:
		break;
	case EVENT_CHANNEL_CLOSED:
	case EVENT_CHANNEL_COLLISION:
		break;

	default:

		LOG_WARNING("BS evt %d", p_ant_evt->event);
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

	case CT_CHANNEL_NUMBER:
		ant_evt_bs (p_ant_evt);
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


/**@brief initialize application
 */
static void continuous_scan_init()
{
    uint32_t err_code;

    // Set library config to report RSSI and Device ID
    err_code = sd_ant_lib_config_set(
        ANT_LIB_CONFIG_MESG_OUT_INC_RSSI | ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    APP_ERROR_CHECK(err_code);

    // Configure channel 0 for scanning mode
    ant_channel_config_t channel_config =
    {
        .channel_number    = CT_CHANNEL_NUMBER,
        .channel_type      = CHANNEL_TYPE_SLAVE,
        .ext_assign        = 0x00,
        .rf_freq           = FEC_ANTPLUS_RF_FREQ,
        .transmission_type = 0x05u,
        .device_type       = FEC_DEVICE_TYPE,
        .device_number     = TACX_DEVICE_NUMBER,
        .channel_period    = FEC_MSG_PERIOD,          // Not used, since this is going to be scanning
        .network_number    = ANTPLUS_NETWORK_NUMBER,
    };

    err_code = ant_channel_init(&channel_config);
    APP_ERROR_CHECK(err_code);

    // Activate message reception from the slave as well
    // This function starts receive scanning mode feature. Channel 0 must be assigned.  All other channels must be closed.
    err_code = sd_ant_rx_scan_mode_start(0);
    APP_ERROR_CHECK(err_code);

}


//static void bs_scan_init(void) {
//
//    uint32_t err_code;
//    // Set library config to report RSSI and Device ID
//    err_code = sd_ant_lib_config_set(
//        ANT_LIB_CONFIG_MESG_OUT_INC_RSSI | ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
//    APP_ERROR_CHECK(err_code);
//
//	// BS
//	const ant_search_config_t bs_search_config =
//	{
//			.channel_number        = BS_CHANNEL_NUMBER,
//			.low_priority_timeout  = ANT_LOW_PRIORITY_TIMEOUT_DISABLE,
//			.high_priority_timeout = ANT_HIGH_PRIORITY_TIMEOUT_DISABLE,
//			.search_sharing_cycles = ANT_SEARCH_SHARING_CYCLES_DISABLE,
//			.search_priority       = ANT_SEARCH_PRIORITY_DEFAULT,
//			.waveform              = ANT_WAVEFORM_DEFAULT,
//	};
//
//	// Background search
//	const ant_channel_config_t bs_channel_config =
//	{
//			.channel_number    = BS_CHANNEL_NUMBER,
//			.channel_type      = CHANNEL_TYPE_SLAVE,
//			.ext_assign        = 0,
//			.rf_freq           = FEC_ANTPLUS_RF_FREQ,              	// ANT+ frequency
//			.transmission_type = 0x05,
//			.device_type       = 17u,
//			.device_number     = TACX_DEVICE_NUMBER,
//			.channel_period    = 8192u,
//			.network_number    = ANTPLUS_NETWORK_NUMBER,
//	};
//
//	err_code = ant_channel_init(&bs_channel_config);
//	APP_ERROR_CHECK(err_code);
//
//    err_code = sd_ant_rx_scan_mode_start(0);
//
//	err_code = ant_search_init(&bs_search_config);
//	APP_ERROR_CHECK(err_code);
//
//    err_code = sd_ant_channel_open(BS_CHANNEL_NUMBER);
//    APP_ERROR_CHECK(err_code);
//}

/**
 *
 */
void ant_setup_init(void) {


}

/**
 *
 */
void ant_setup_start(void)
{
//	bs_scan_init();

	continuous_scan_init();

	LOG_INFO("ANT started");
}

void ant_setup_stop(void) {

}

/**
 *
 */
void ant_tasks(void) {


}


#endif
