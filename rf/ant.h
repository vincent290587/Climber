
#ifndef __ANT_C__
#define __ANT_C__

#include "g_structs.h"


#define WILDCARD_TRANSMISSION_TYPE      0x00
#define ANTPLUS_NETWORK_NUMBER          0x00      /**< Network number. */

#define CT_CHANNEL_NUMBER               0x00
#define BS_CHANNEL_NUMBER               0x01
#define FEC_CHANNEL_NUMBER              0x02

#define TACX_DEVICE_NUMBER              2846U


typedef enum {
	eAntSensorsChannelHRM,
	eAntSensorsChannelBSC,
	eAntSensorsChannelFEC,
	eAntSensorsChannelBS
} eAntSensorsChannelNumber;

typedef enum {
	eAntPairingSensorTypeNone,
	eAntPairingSensorTypeHRM,
	eAntPairingSensorTypeBSC,
	eAntPairingSensorTypeFEC
} eAntPairingSensorType;


#ifdef __cplusplus
extern "C" {
#endif


void ant_tasks(void);

void ant_stack_init(void);

void ant_setup_init(void);

void ant_setup_stop(void);

void ant_setup_start(void);

void ant_search_start(eAntPairingSensorType search_type);

void ant_search_end(eAntPairingSensorType search_type, uint16_t dev_id);

void ant_timers_init(void);


#ifdef __cplusplus
}
#endif

#endif
