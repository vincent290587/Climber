/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "fram.h"
#include "Model.h"
#include "helper.h"
#include "gpio.h"
#include "boards.h"
#include "sdk_config.h"
#include "app_scheduler.h"
#include "vl53l1_wrapper.h"
#include "lsm6ds33_wrapper.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"
#include "data_dispatcher.h"


#if defined (BLE_STACK_SUPPORT_REQD)
#include "ble_api_base.h"
#endif

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant.h"
#endif


UserSettings   u_settings;

sAppErrorDescr m_app_error __attribute__ ((section(".noinit")));


void perform_system_tasks_light(void) {

#if APP_SCHEDULER_ENABLED
	app_sched_execute();
#endif

	if (LOG_PROCESS() == false)
	{
		pwr_mgmt_run();
	}
}

void sensing_task(void * p_context)
{
	w_task_delay(50);

	vl53l1_wrapper__init();

#ifdef HAS_LSM6
	lsm6ds33_wrapper__init();
#endif

	for(;;)
	{

		vl53l1_wrapper__measure();

		gpio_toggle(LED_1);

	}
}

void actuating_task(void * p_context)
{
	// init parameters
	fram_init_sensor();
	u_settings.checkConfigVersion();

	// init computational things
	data_dispatcher__init(w_task_id_get());

	for(;;)
	{
		// run filter
		data_dispatcher__run();

		wdt_reload();

	}
}

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

#if defined (ANT_STACK_SUPPORT_REQD)
		ant_tasks();
#endif

		// BSP tasks
		bsp_tasks();

		w_task_delay(20);

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
		if (LOG_PROCESS() == false) {
			pwr_mgmt_run();
		}

		w_task_yield();
	}
}
