/*
 * vl53l1_platform_log.c
 *
 *  Created on: 1 déc. 2019
 *      Author: Vincent
 */


#include "millis.h"
#include "vl53l1_platform_log.h"
#include "segger_wrapper.h"

#ifdef VL53L1_LOG_ENABLE

uint32_t _trace_level = VL53L1_TRACE_LEVEL_ALL;
/*
 * NOTE: dynamically exported if we enable logging.
 *       this way, Python interfaces can access this function, but we don't
 *       need to include it in the .def files.
 */
int8_t VL53L1_trace_config(
		char *filename,
		uint32_t modules,
		uint32_t level,
		uint32_t functions) {

	return 0;
}

/**
 * @brief Print trace module function.
 *
 * @param module   - ??
 * @param level    - ??
 * @param function - ??
 * @param format   - ??
 *
 */
void VL53L1_trace_print_module_function(
		uint32_t module,
		uint32_t level,
		uint32_t function,
		const char *format, ...) {



}

uint32_t m_tr_func = (uint32_t)&VL53L1_trace_print_module_function;

/**
 * @brief Get global _trace_functions parameter
 *
 * @return _trace_functions
 */
uint32_t VL53L1_get_trace_functions(void) {
	return m_tr_func;
}

/**
 * @brief Set global _trace_functions parameter
 *
 * @param[in] function : new function code
 */
void VL53L1_set_trace_functions(uint32_t function) {
	m_tr_func = function;
}


/*
 * @brief Returns the current system tick count in [ms]
 *
 * @return  time_ms : current time in [ms]
 *
 */
uint32_t VL53L1_clock(void) {

	return millis();
}

#endif
