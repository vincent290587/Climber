/*
 * ant_tdd.c
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#include <stdbool.h>
#include "ant.h"
#include "millis.h"
#include "segger_wrapper.h"

static bool m_is_searching = false;

void ant_timers_init(void) {

}

void ant_stack_init(void) {

}

void ant_setup_init(void) {

}

void ant_setup_start(void) {
	LOG_INFO("ANT started");
}
