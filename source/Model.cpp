/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "sdk_config.h"
#include "helper.h"
#include "app_scheduler.h"
#include "segger_wrapper.h"

#include "i2c_scheduler.h"

#if defined (BLE_STACK_SUPPORT_REQD)
#include "ble_api_base.h"
#endif

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant.h"
#endif


PowerZone     zPower;

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

/**
 *
 * @param p_context
 */
void idle_task(void * p_context)
{
    for(;;)
    {

		// BSP tasks
		bsp_tasks();

    	//No more logs to process, go to sleep
		sysview_task_idle();
    	pwr_mgmt_run();

    	task_yield();
    }
}
