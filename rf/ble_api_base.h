/*
 * ble_api.h
 *
 *  Created on: 21 déc. 2018
 *      Author: Vincent
 */

#ifndef RF_BLE_API_BASE_H_
#define RF_BLE_API_BASE_H_

#include "g_structs.h"

typedef enum {
	eBleEventTypeStartXfer,
} eBleEventType;

typedef struct {
	char*    str;
	size_t length;
} sCharArray;

#ifdef __cplusplus
extern "C" {
#endif


void ble_init(void);

void ble_uninit(void);

void ble_nus_tasks(void);

void ble_start_evt(eBleEventType evt);

void ble_nus_log_text(const char * text);

void ble_nus_log(const char* format, ...);


#ifdef __cplusplus
}
#endif

#endif /* RF_BLE_API_BASE_H_ */
