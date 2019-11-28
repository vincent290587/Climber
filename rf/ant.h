
#ifndef __ANT_C__
#define __ANT_C__

#include "g_structs.h"


#define WILDCARD_TRANSMISSION_TYPE      0x00
#define ANTPLUS_NETWORK_NUMBER          0x00      /**< Network number. */


#ifdef __cplusplus
extern "C" {
#endif


void ant_tasks(void);

void ant_stack_init(void);

void ant_setup_init(void);

void ant_setup_start(void);

void ant_setup_stop(void);

void ant_timers_init(void);


#ifdef __cplusplus
}
#endif

#endif
