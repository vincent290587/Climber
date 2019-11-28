/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_H_
#define SOURCE_MODEL_H_

#ifndef TDD

#include <stdbool.h>
#include "parameters.h"
#include "g_structs.h"
#include "segger_wrapper.h"

typedef struct {
	uint8_t peripherals_id;
	uint8_t boucle_id;
	uint8_t system_id;
	uint8_t ls027_id;
} sTasksIDs;

extern sTasksIDs m_tasks_id;

#if defined(__cplusplus)

#include "PowerZone.h"
#include "UserSettings.h"

extern PowerZone     zPower;

extern UserSettings   u_settings;

extern sAppErrorDescr m_app_error;

extern "C" {
#endif // defined C++

void model_input_virtual_uart(char c);

void model_go_to_msc_mode(void);

void model_dispatch_sensors_update(void);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

void app_shutdown(void);

void bsp_tasks(void);

void backlighting_tasks(void);

void idle_task(void * p_context);

void boucle_task(void * p_context);

void peripherals_task(void * p_context);

void system_task(void * p_context);

#if defined(__cplusplus)
}
#endif // defined C++

#else /* TDD */

#include "Model_tdd.h"

#endif /* TDD */

#endif /* SOURCE_MODEL_H_ */
