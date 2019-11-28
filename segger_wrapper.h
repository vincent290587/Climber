/*
 * segger_wrapper.h
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_SEGGER_WRAPPER_H_
#define LIBRARIES_UTILS_SEGGER_WRAPPER_H_

#include <stdint.h>
#include "hardfault.h"

#define EMPTY_MACRO                    do {} while (0)

#define SYSTEM_DESCR_POS_CRC           0xFD

typedef struct {
	HardFault_stack_t stck;
	uint8_t crc;
} sAppHardFaultDesc;

typedef struct {
	char _buffer[210];
	uint32_t pc;
	uint32_t id;
	uint8_t crc;
} sAppErrorDesc;

typedef struct {
	uint8_t special;
	uint32_t void_id;
	uint32_t task_id;
	sAppErrorDesc err_desc;
	sAppHardFaultDesc hf_desc;
} sAppErrorDescr;

extern sAppErrorDescr m_app_error;

#ifdef TDD
#include "segger_wrapper_tdd.h"
#else
#include "segger_wrapper_arm.h"
#endif

#include "task_manager_wrapper.h"

#endif /* LIBRARIES_UTILS_SEGGER_WRAPPER_H_ */
