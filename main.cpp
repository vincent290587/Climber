/*
 * App.cpp
 *
 *  Created on: 8 oct. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardfault.h"
#include "i2c.h"
#include "helper.h"
#include "ant.h"
#include "bsp.h"
#include "i2c.h"
#include "fram.h"
#include "ble_api_base.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_sdm.h"
#include "nrf_strerror.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_clock.h"
#include "task_manager.h"
#include "nrf_bootloader_info.h"
#include "nrfx_wdt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "i2c_scheduler.h"
#include "Model.h"
#include "segger_wrapper.h"
#include "WString.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "nrf_pwr_mgmt.h"
#ifdef __cplusplus
}
#endif


#define APP_DELAY           APP_TIMER_TICKS(APP_TIMEOUT_DELAY_MS)

#define SCHED_MAX_EVENT_DATA_SIZE      APP_TIMER_SCHED_EVENT_DATA_SIZE              /**< Maximum size of scheduler events. */
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE               20                                           /**< Maximum number of events in the scheduler queue. */
#endif

nrfx_wdt_channel_id m_channel_id;

static bsp_event_t m_bsp_evt = BSP_EVENT_NOTHING;


/**
 * @brief Handler for timer events.
 */
void timer_event_handler(void* p_context)
{
	ASSERT(p_context);

}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
extern "C" void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
	assert_info_t assert_info =
	{
			.line_num    = line_num,
			.p_file_name = file_name,
	};
	app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

#ifndef DEBUG_NRF_USER
	LOG_WARNING("System reset");
	LOG_FLUSH();
	NVIC_SystemReset();
#else
	NRF_BREAKPOINT_COND;

	bool loop = true;
	while (loop) ;
#endif // DEBUG

	UNUSED_VARIABLE(assert_info);
}

/**
 *
 * @param id
 * @param pc
 * @param info
 */
extern "C" void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	NRF_LOG_FLUSH();

	m_app_error.err_desc.id = id;
	m_app_error.err_desc.pc = pc;
	m_app_error.err_desc.crc = SYSTEM_DESCR_POS_CRC;

	switch (id)
	{
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
	case NRF_FAULT_ID_SD_ASSERT:
		NRF_LOG_ERROR("SOFTDEVICE: ASSERTION FAILED");
		break;
	case NRF_FAULT_ID_APP_MEMACC:
		NRF_LOG_ERROR("SOFTDEVICE: INVALID MEMORY ACCESS");
		break;
#endif
	case NRF_FAULT_ID_SDK_ASSERT:
	{
		assert_info_t * p_info = (assert_info_t *)info;
		snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
				"ASSERTION FAILED at %s:%u",
				p_info->p_file_name,
				p_info->line_num);
#if USE_SVIEW
		SEGGER_SYSVIEW_Error(m_app_error.err_desc._buffer);
#else
		NRF_LOG_ERROR(m_app_error.err_desc._buffer);
		LOG_ERROR(m_app_error.err_desc._buffer);
#endif
		break;
	}
	case NRF_FAULT_ID_SDK_ERROR:
	{
		error_info_t * p_info = (error_info_t *)info;
		snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
				"ERROR %u [%s] at %s:%u",
				(unsigned int)p_info->err_code,
				nrf_strerror_get(p_info->err_code),
				p_info->p_file_name,
				(uint16_t)p_info->line_num);
#if USE_SVIEW
		SEGGER_SYSVIEW_Error(m_app_error.err_desc._buffer);
#else
		LOG_ERROR(m_app_error.err_desc._buffer);
#endif
		break;
	}
	default:
		NRF_LOG_ERROR("UNKNOWN FAULT at 0x%08X", pc);
		snprintf(m_app_error.err_desc._buffer, sizeof(m_app_error.err_desc._buffer),
				"UNKNOWN FAULT at 0x%08X", (unsigned int)pc);
		break;
	}

	LOG_FLUSH();

#ifdef DEBUG_NRF
	NRF_BREAKPOINT_COND;
	// On assert, the system can only recover with a reset.
#endif

}

extern "C" void HardFault_process(HardFault_stack_t * p_stack)
{
	LOG_ERROR("HardFault: pc=%u", p_stack->pc);

	m_app_error.hf_desc.crc = SYSTEM_DESCR_POS_CRC;
	memcpy(&m_app_error.hf_desc.stck, p_stack, sizeof(HardFault_stack_t));

#ifdef DEBUG_NRF
	NRF_BREAKPOINT_COND;
	// On hardfault, the system can only recover with a reset.

	bool loop = true;
	while (loop) ;
#endif
	// Restart the system by default
	NVIC_SystemReset();
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
	ret_code_t err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);

	NRF_LOG_DEFAULT_BACKENDS_INIT();

	SVIEW_INIT();
}

/**@brief Interrupt function for handling bsp events.
 */
static void bsp_evt_handler(bsp_event_t evt)
{
	m_bsp_evt = evt;
}

/**@brief Function for handling bsp events.
 */
void bsp_tasks(void)
{
	switch (m_bsp_evt)
	{
	case BSP_EVENT_KEY_0:
	{
		LOG_WARNING("Going to DFU !");
		nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
	} break;
	default:
		return; // no implementation needed
	}

	// clear event
	m_bsp_evt = BSP_EVENT_NOTHING;
}

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
	ret_code_t err_code;

	switch (event) {
	case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
	{
		// TODO prepare wakeup source
	}
	// no break
	case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
	{
		LOG_WARNING("Preparing SYSOFF");

		i2c_scheduling_uninit();

		nrf_gpio_pin_clear(HV_EN);
		nrf_gpio_pin_set(N_VCCINT_EN);

#if defined (ANT_STACK_SUPPORT_REQD)
	    ant_setup_stop();
#endif

#if defined (BLE_STACK_SUPPORT_REQD)
	    ble_uninit();
#endif

		// Device ready to enter
		err_code = app_timer_stop_all();
		APP_ERROR_CHECK(err_code);

#ifdef SOFTDEVICE_PRESENT
		nrf_sdh_disable_request();
#endif

	} break;
	case NRF_PWR_MGMT_EVT_PREPARE_DFU:
	{
		nrf_gpio_pin_clear(HV_EN);
		nrf_gpio_pin_set(N_VCCINT_EN);

#if defined (ANT_STACK_SUPPORT_REQD)
	    ant_setup_stop();
#endif

		err_code = app_timer_stop_all();
		APP_ERROR_CHECK(err_code);

		LOG_WARNING("Power management allowed to reset to DFU mode.");

	} break;
	default:
		break;
	}

	return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);


/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
	//NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}


/**@brief Function for initializing buttons and LEDs.
 *
 * @param[out] p_erase_bonds  True if the clear bonds button was pressed to wake the application up.
 */
static void buttons_leds_init(void)
{
	uint32_t err_code = bsp_init(BSP_INIT_BUTTONS | BSP_INIT_LEDS, bsp_evt_handler);
	APP_ERROR_CHECK(err_code);
}

static void pins_init(void)
{
	ret_code_t err_code;

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	// TODO configure pins

	nrf_gpio_cfg_input(GYR_INT1, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_cfg_input(LIS_INT1, NRF_GPIO_PIN_NOPULL);

	nrf_gpio_pin_clear(LIS_INT2);
	nrf_gpio_cfg_output(LIS_INT2);

	nrf_gpio_cfg_input(HV_EN, NRF_GPIO_PIN_PULLUP);
//	nrf_gpio_pin_set(HV_EN);

	nrf_gpio_cfg_output(N_VCCINT_EN);
	nrf_gpio_pin_clear(N_VCCINT_EN);

}

void wdt_reload(void) {
	nrfx_wdt_channel_feed(m_channel_id);
}

void app_shutdown(void) {
	nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
}


#ifdef SOFTDEVICE_PRESENT
/**@brief Function for initializing the softdevice
 *
 * @details Initializes the SoftDevice
 */
static void sdh_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);

	ASSERT(nrf_sdh_is_enabled());

}
#endif


/**
 *
 * @return 0
 */
int main(void)
{
	ret_code_t err_code;

	// Initialize.
	//Configure WDT.
	nrfx_wdt_config_t wdt_config = NRFX_WDT_DEAFULT_CONFIG;
	err_code = nrfx_wdt_init(&wdt_config, wdt_event_handler);
	APP_ERROR_CHECK(err_code);
	err_code = nrfx_wdt_channel_alloc(&m_channel_id);
	APP_ERROR_CHECK(err_code);
	nrfx_wdt_enable();

	log_init();

#ifdef FPU_INTERRUPT_MODE
	// Enable FPU interrupt
	NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOWEST);
	NVIC_ClearPendingIRQ(FPU_IRQn);
	NVIC_EnableIRQ(FPU_IRQn);
#endif

	pins_init();

	err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);

	LOG_INFO("Init start");

	// check for errors
	if (m_app_error.hf_desc.crc == SYSTEM_DESCR_POS_CRC) {
		LOG_ERROR("Hard Fault found");
		String message = "Hardfault happened: pc = 0x";
		message += String(m_app_error.hf_desc.stck.pc);
		message += " in void ";
		message += m_app_error.void_id;
		LOG_ERROR(message.c_str());
		memset(&m_app_error.hf_desc, 0, sizeof(m_app_error.hf_desc));
	}

	err_code = nrf_pwr_mgmt_init();
	APP_ERROR_CHECK(err_code);

#if APP_SCHEDULER_ENABLED
	APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
#endif

	buttons_leds_init();
	nrf_gpio_pin_set(LED_1);

	// drivers
	i2c_init();

	// init BLE + ANT
#ifdef SOFTDEVICE_PRESENT
	sdh_init();
#endif
#if defined (ANT_STACK_SUPPORT_REQD)
	ant_stack_init();
	ant_setup_init();
	ant_setup_start();
#endif
#if defined (BLE_STACK_SUPPORT_REQD)
	ble_init();
#endif
#ifdef FDS_PRESENT
	fram_init_sensor();
	u_settings.checkConfigVersion();
#endif

	// init all I2C devices
	i2c_scheduling_init();

	LOG_INFO("App init done");

	nrf_gpio_pin_clear(LED_1);

	for (;;) {

#if APP_SCHEDULER_ENABLED
		app_sched_execute();
#endif

#if defined (BLE_STACK_SUPPORT_REQD)
		ble_nus_tasks();
#endif

		i2c_scheduling_tasks();

		LOG_DEBUG("App run");

		wdt_reload();

		bsp_tasks();

    	//No more logs to process, go to sleep
		sysview_task_idle();

		nrf_pwr_mgmt_run();
	}

}

