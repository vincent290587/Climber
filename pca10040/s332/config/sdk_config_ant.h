
//==========================================================
// <o> ANTPLUS_NETWORK_NUM - ANT PLUS network number.
#ifndef ANTPLUS_NETWORK_NUM
#define ANTPLUS_NETWORK_NUM 0
#endif

// <o> BSC_CHANNEL_NUM - Channel number assigned to BSC profile.
#ifndef BSC_CHANNEL_NUM
#define BSC_CHANNEL_NUM 0
#endif

// <o> CHAN_ID_DEV_NUM - Channel ID: Device Number.
#ifndef CHAN_ID_DEV_NUM
#define CHAN_ID_DEV_NUM 49
#endif

// <o> CHAN_ID_TRANS_TYPE - Channel ID: Transmission type.
#ifndef CHAN_ID_TRANS_TYPE
#define CHAN_ID_TRANS_TYPE 1
#endif

// <o> MODIFICATION_TYPE  - Type of speed and cadence values update


// <i> They can be updated by buttons or periodically rise and fall in auto mode
// <0=> MODIFICATION_TYPE_BUTTON
// <1=> MODIFICATION_TYPE_AUTO

#ifndef MODIFICATION_TYPE
#define MODIFICATION_TYPE 0
#endif

// <h> PRODUCT_INFORMATION

//==========================================================
// <o> BSC_HW_VERSION - Hardware revision
#ifndef BSC_HW_VERSION
#define BSC_HW_VERSION 5
#endif

// <o> BSC_MF_ID - Manufacturer ID
#ifndef BSC_MF_ID
#define BSC_MF_ID 2
#endif

// <o> BSC_MODEL_NUMBER - Model number
#ifndef BSC_MODEL_NUMBER
#define BSC_MODEL_NUMBER 2
#endif

// <o> BSC_SW_VERSION - Software version number
#ifndef BSC_SW_VERSION
#define BSC_SW_VERSION 0
#endif

// <o> BSC_SERIAL_NUMBER - Serial number
#ifndef BSC_SERIAL_NUMBER
#define BSC_SERIAL_NUMBER 43981
#endif

// </h>
//==========================================================

// <o> SENSOR_TYPE  - Type of transmitted data

// <121=> Combined data
// <122=> Cadence data
// <123=> Speed data

#ifndef SENSOR_TYPE
#define SENSOR_TYPE 122
#endif

// </h>


//==========================================================
// <e> ANT_BSC_ENABLED - ant_bsc - Bicycle Speed and Cadence Profile
//==========================================================
#ifndef ANT_BSC_ENABLED
#define ANT_BSC_ENABLED 1
#endif
#if  ANT_BSC_ENABLED
// <e> ANT_BSC_LOG_ENABLED - Enables general logging in the module.
//==========================================================
#ifndef ANT_BSC_LOG_ENABLED
#define ANT_BSC_LOG_ENABLED 0
#endif
#if  ANT_BSC_LOG_ENABLED
// <o> ANT_BSC_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_LOG_LEVEL
#define ANT_BSC_LOG_LEVEL 3
#endif

// <o> ANT_BSC_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_INFO_COLOR
#define ANT_BSC_INFO_COLOR 0
#endif

#endif //ANT_BSC_LOG_ENABLED
// </e>

// <e> ANT_BSC_COMBINED_PAGE_0_LOG_ENABLED - Enables logging of BSC Combined page 0 in the module.
//==========================================================
#ifndef ANT_BSC_COMBINED_PAGE_0_LOG_ENABLED
#define ANT_BSC_COMBINED_PAGE_0_LOG_ENABLED 0
#endif
#if  ANT_BSC_COMBINED_PAGE_0_LOG_ENABLED
// <o> ANT_BSC_COMBINED_PAGE_0_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_COMBINED_PAGE_0_LOG_LEVEL
#define ANT_BSC_COMBINED_PAGE_0_LOG_LEVEL 3
#endif

// <o> ANT_BSC_COMBINED_PAGE_0_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_COMBINED_PAGE_0_INFO_COLOR
#define ANT_BSC_COMBINED_PAGE_0_INFO_COLOR 0
#endif

#endif //ANT_BSC_COMBINED_PAGE_0_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_0_LOG_ENABLED - Enables logging of BSC page 0 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_0_LOG_ENABLED
#define ANT_BSC_PAGE_0_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_0_LOG_ENABLED
// <o> ANT_BSC_PAGE_0_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_0_LOG_LEVEL
#define ANT_BSC_PAGE_0_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_0_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_0_INFO_COLOR
#define ANT_BSC_PAGE_0_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_0_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_1_LOG_ENABLED - Enables logging of BSC page 1 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_1_LOG_ENABLED
#define ANT_BSC_PAGE_1_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_1_LOG_ENABLED
// <o> ANT_BSC_PAGE_1_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_1_LOG_LEVEL
#define ANT_BSC_PAGE_1_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_1_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_1_INFO_COLOR
#define ANT_BSC_PAGE_1_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_1_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_2_LOG_ENABLED - Enables logging of BSC page 2 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_2_LOG_ENABLED
#define ANT_BSC_PAGE_2_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_2_LOG_ENABLED
// <o> ANT_BSC_PAGE_2_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_2_LOG_LEVEL
#define ANT_BSC_PAGE_2_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_2_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_2_INFO_COLOR
#define ANT_BSC_PAGE_2_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_2_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_3_LOG_ENABLED - Enables logging of BSC page 3 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_3_LOG_ENABLED
#define ANT_BSC_PAGE_3_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_3_LOG_ENABLED
// <o> ANT_BSC_PAGE_3_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_3_LOG_LEVEL
#define ANT_BSC_PAGE_3_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_3_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_3_INFO_COLOR
#define ANT_BSC_PAGE_3_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_3_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_4_LOG_ENABLED - Enables logging of BSC page 4 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_4_LOG_ENABLED
#define ANT_BSC_PAGE_4_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_4_LOG_ENABLED
// <o> ANT_BSC_PAGE_4_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_4_LOG_LEVEL
#define ANT_BSC_PAGE_4_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_4_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_4_INFO_COLOR
#define ANT_BSC_PAGE_4_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_4_LOG_ENABLED
// </e>

// <e> ANT_BSC_PAGE_5_LOG_ENABLED - Enables logging of BSC page 5 in the module.
//==========================================================
#ifndef ANT_BSC_PAGE_5_LOG_ENABLED
#define ANT_BSC_PAGE_5_LOG_ENABLED 1
#endif
#if  ANT_BSC_PAGE_5_LOG_ENABLED
// <o> ANT_BSC_PAGE_5_LOG_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef ANT_BSC_PAGE_5_LOG_LEVEL
#define ANT_BSC_PAGE_5_LOG_LEVEL 3
#endif

// <o> ANT_BSC_PAGE_5_INFO_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef ANT_BSC_PAGE_5_INFO_COLOR
#define ANT_BSC_PAGE_5_INFO_COLOR 0
#endif

#endif //ANT_BSC_PAGE_5_LOG_ENABLED
// </e>

#endif //ANT_BSC_ENABLED
// </e>

//==========================================================
// <q> ANT_CHANNEL_CONFIG_ENABLED  - ant_channel_config - ANT common channel configuration


#ifndef ANT_CHANNEL_CONFIG_ENABLED
#define ANT_CHANNEL_CONFIG_ENABLED 1
#endif

// <q> ANT_KEY_MANAGER_ENABLED  - ant_key_manager - Software Component


#ifndef ANT_KEY_MANAGER_ENABLED
#define ANT_KEY_MANAGER_ENABLED 1
#endif


// <q> ANT_STATE_INDICATOR_ENABLED  - ant_state_indicator - ANT state indicator using BSP


#ifndef ANT_STATE_INDICATOR_ENABLED
#define ANT_STATE_INDICATOR_ENABLED 0
#endif

// <e> ANT_SEARCH_CONFIG_ENABLED - ant_search_config - ANT common search configuration
//==========================================================
#ifndef ANT_SEARCH_CONFIG_ENABLED
#define ANT_SEARCH_CONFIG_ENABLED 1
#endif
// <o> ANT_DEFAULT_LOW_PRIORITY_TIMEOUT - Default low priority search time-out.  <0-255>


#ifndef ANT_DEFAULT_LOW_PRIORITY_TIMEOUT
#define ANT_DEFAULT_LOW_PRIORITY_TIMEOUT 2
#endif

// <o> ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT - Default high priority search time-out.  <0-255>


#ifndef ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT
#define ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT 10
#endif

// </h>

//==========================================================

// <h> nRF_SoftDevice

//==========================================================
// <e> NRF_SDH_ANT_ENABLED - nrf_sdh_ant - SoftDevice ANT event handler
//==========================================================
#ifndef NRF_SDH_ANT_ENABLED
#define NRF_SDH_ANT_ENABLED 1
#endif
// <h> ANT Channels

//==========================================================
// <o> NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED - Allocated ANT channels.
#ifndef NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED
#define NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED 2
#endif

// <o> NRF_SDH_ANT_ENCRYPTED_CHANNELS - Encrypted ANT channels.
#ifndef NRF_SDH_ANT_ENCRYPTED_CHANNELS
#define NRF_SDH_ANT_ENCRYPTED_CHANNELS 0
#endif

// </h>
//==========================================================

// <h> ANT Queues

//==========================================================
// <o> NRF_SDH_ANT_EVENT_QUEUE_SIZE - Event queue size.
#ifndef NRF_SDH_ANT_EVENT_QUEUE_SIZE
#define NRF_SDH_ANT_EVENT_QUEUE_SIZE 32
#endif

// <o> NRF_SDH_ANT_BURST_QUEUE_SIZE - ANT burst queue size.
#ifndef NRF_SDH_ANT_BURST_QUEUE_SIZE
#define NRF_SDH_ANT_BURST_QUEUE_SIZE 128
#endif

// </h>
//==========================================================

// <h> ANT Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_ANT_OBSERVER_PRIO_LEVELS - Total number of priority levels for ANT observers.
// <i> This setting configures the number of priority levels available for the ANT event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_ANT_OBSERVER_PRIO_LEVELS
#define NRF_SDH_ANT_OBSERVER_PRIO_LEVELS 2
#endif

// <h> ANT Observers priorities - Invididual priorities

//==========================================================
// <o> ANT_BPWR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Power Profile.

#ifndef ANT_BPWR_ANT_OBSERVER_PRIO
#define ANT_BPWR_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_BSC_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Speed and Cadence Profile.

#ifndef ANT_BSC_ANT_OBSERVER_PRIO
#define ANT_BSC_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_ENCRYPT_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Cryptographic ANT stack configuration module.

#ifndef ANT_ENCRYPT_ANT_OBSERVER_PRIO
#define ANT_ENCRYPT_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_HRM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Heart Rate Monitor.

#ifndef ANT_HRM_ANT_OBSERVER_PRIO
#define ANT_HRM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_SDM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Stride Based Speed and Distance Monitor Profile.

#ifndef ANT_SDM_ANT_OBSERVER_PRIO
#define ANT_SDM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the ANT state indicator module.

#ifndef ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
#define ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO 1
#endif

// <o> BSP_BTN_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Button Control module.

#ifndef BSP_BTN_ANT_OBSERVER_PRIO
#define BSP_BTN_ANT_OBSERVER_PRIO 1
#endif

// <o> NRF_SDH_ANT_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which ANT events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have ANT events dispatched before or after other stack events, such as BLE or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_ANT_STACK_OBSERVER_PRIO
#define NRF_SDH_ANT_STACK_OBSERVER_PRIO 0
#endif

// </h>
