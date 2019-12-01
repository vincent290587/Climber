/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "Model.h"
#include "helper.h"
#include "boards.h"
#include "sdk_config.h"
#include "app_scheduler.h"
#include "vl53l1_wrapper.h"
#include "lsm6ds33_wrapper.h"
#include "segger_wrapper.h"

#include "i2c_scheduler.h"

#if defined (BLE_STACK_SUPPORT_REQD)
#include "ble_api_base.h"
#endif

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant.h"
#endif


UserSettings   u_settings;

sTasksIDs     m_tasks_id;

sAppErrorDescr m_app_error __attribute__ ((section(".noinit")));


/**
 *
 */
void model_dispatch_sensors_update(void) {

}

/**
 *
 */
void perform_system_tasks(void) {

#if APP_SCHEDULER_ENABLED
	app_sched_execute();
#endif

}

/**
 *
 */
void perform_system_tasks_light(void) {

#if APP_SCHEDULER_ENABLED
	app_sched_execute();
#endif

	if (NRF_LOG_PROCESS() == false)
	{
		pwr_mgmt_run();
	}
}

void sensing_task(void * p_context)
{

	int res;
	do {
		res = vl53l1_wrapper__init();

		w_task_delay(200);
	} while (res);

	for(;;)
	{

		res = vl53l1_wrapper__measure();

		w_task_delay(10);

		nrf_gpio_pin_toggle(LED_1);

	}
}

void actuating_task(void * p_context)
{

//	lsm6ds33_wrapper__init();

	for(;;)
	{

		w_task_delay(100);

//		if (digitalRead(LSM6_INT1)) {
//			_lsm6_read_sensor();
//		}

		LOG_DEBUG("Actuating");

		wdt_reload();

	}
}

/**
 * Task triggered every APP_TIMEOUT_DELAY_MS.
 *
 * @param p_context
 */
void peripherals_task(void * p_context)
{
	for(;;)
	{

#if APP_SCHEDULER_ENABLED
		app_sched_execute();
#endif

#if defined (BLE_STACK_SUPPORT_REQD)
		ble_nus_tasks();
#endif

		// BSP tasks
//		bsp_tasks();

		w_task_delay(100);

	}
}

/**
 *
 * @param p_context
 */
void idle_task(void * p_context)
{
	for(;;)
	{
		NRF_LOG_PROCESS();

		pwr_mgmt_run();

		w_task_yield();
	}
}
