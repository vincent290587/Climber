/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include <unistd.h>
#include "Model.h"
#include "Simulator.h"
#include "segger_wrapper.h"


sTasksIDs     m_tasks_id;

sAppErrorDescr m_app_error;

UserSettings   u_settings;

/**
 *
 */
void model_dispatch_sensors_update(void) {


}

/**
 *
 */
void perform_system_tasks(void) {

}

/**
 *
 */
void perform_system_tasks_light(void) {

}

/**
 *
 * @param p_context
 */
void idle_task(void * p_context)
{
    for(;;)
    {
    	sleep(5);

		simulator_tasks();

		w_task_yield();
    }
}

/**
 * System continuous tasks
 *
 * @param p_context
 */
void system_task(void * p_context)
{
    for(;;)
    {
		w_task_yield();
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
		w_task_yield();
	}
}


