/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_H_
#define SOURCE_MODEL_H_

#include <stdbool.h>
#include "parameters.h"
#include "g_structs.h"
#include "segger_wrapper.h"


#if defined(__cplusplus)

#include "UserSettings.h"

extern UserSettings   u_settings;

extern sAppErrorDescr m_app_error;

extern "C" {
#endif // defined C++

void model_input_virtual_uart(char c);

void model_dispatch_sensors_update(void);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

void app_shutdown(void);

void bsp_tasks(void);

void idle_task(void * p_context);

void peripherals_task(void * p_context);

void system_task(void * p_context);

void sensing_task(void * p_context);

void actuating_task(void * p_context);

#if defined(__cplusplus)
}
#endif // defined C++


#endif /* SOURCE_MODEL_H_ */
