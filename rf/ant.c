

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
#include "ant_bsc.h"
#include "ant_interface.h"

#include "Model.h"
#include "segger_wrapper.h"

#include "bsc.h"


/**< Application's ANT observer priority. You shouldn't need to modify this value. */
#define APP_ANT_OBSERVER_PRIO       1



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

	case BSC_CHANNEL_NUM:
		ant_evt_bsc (p_ant_evt);
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

	bsc_init();
}

/**
 *
 */
void ant_setup_init(void) {

	// CAD
	bsc_profile_setup();
}

/**
 *
 */
void ant_setup_start(void)
{
	// Open the ANT channels
	bsc_profile_start();

	LOG_INFO("ANT started");
}

/**
 *
 */
void ant_setup_stop(void)
{
	// Open the ANT channels
	bsc_profile_stop();

	LOG_INFO("ANT started");
}

/**
 *
 */
void ant_tasks(void) {

}


#endif
