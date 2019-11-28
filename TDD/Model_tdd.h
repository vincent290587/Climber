/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_TDD_H_
#define SOURCE_MODEL_TDD_H_

#include <stdbool.h>
#include "parameters.h"
#include "g_structs.h"
#include "data_dispatcher.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper_tdd.h"

typedef struct {
	task_id_t peripherals_id;
	task_id_t boucle_id;
	task_id_t system_id;
	task_id_t ls027_id;
} sTasksIDs;

extern sTasksIDs m_tasks_id;

#if defined(__cplusplus)

#include "PowerZone.h"
#include "UserSettings.h"

extern sAppErrorDescr m_app_error;

extern PowerZone     zPower;

extern UserSettings   u_settings;

extern "C" {
#endif // defined C++

void model_dispatch_sensors_update(void);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

void print_mem_state(void);

void backlighting_tasks(void);

void idle_task(void * p_context);

void peripherals_task(void * p_context);

void system_task(void * p_context);


#if defined(__cplusplus)
}
#endif // defined C++

#endif /* SOURCE_MODEL_H_ */
