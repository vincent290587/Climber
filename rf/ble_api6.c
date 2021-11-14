

#ifdef BLE_STACK_SUPPORT_REQD

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "gpio.h"
#include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_db_discovery.h"
#include "ble_srv_common.h"
#include "ble_radio_notification.h"
#include "nrf_sdh.h"
#include "nrf_queue.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_pwr_mgmt.h"
#include "app_util.h"
#include "app_error.h"
#include "peer_manager.h"
#include "app_util.h"
#include "app_timer.h"
#include "fds.h"
#include "nrf_fstorage.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"
#include "nrf_ble_gq.h"
#include "helper.h"
#include "ble_bas_c.h"
#include "ble_nus_c.h"
#include "ble_advdata.h"
#include "ant.h"
#include "Model.h"
#include "millis.h"
#include "ble_api_base.h"
#include "data_dispatcher.h"
#include "zpm_decoder.h"
#include "segger_wrapper.h"
#include "ring_buffer.h"
#include "Model.h"

#define BLE_DEVICE_NAME             "Climber"

#define APP_BLE_CONN_CFG_TAG        1                                   /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_BLE_OBSERVER_PRIO       1                                   /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_SOC_OBSERVER_PRIO       1                                   /**< Applications' SoC observer priority. You shoulnd't need to modify this value. */


#define SEC_PARAM_BOND              1                                   /**< Perform bonding. */
#define SEC_PARAM_MITM              0                                   /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC              0                                   /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS          0                                   /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES   BLE_GAP_IO_CAPS_NONE                /**< No I/O capabilities. */
#define SEC_PARAM_OOB               0                                   /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE      7                                   /**< Minimum encryption key size in octets. */
#define SEC_PARAM_MAX_KEY_SIZE      16                                  /**< Maximum encryption key size in octets. */

#define SCAN_INTERVAL               0x00A0                              /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                 0x0050                              /**< Determines scan window in units of 0.625 millisecond. */
#define SCAN_DURATION               0x0000                              /**< Duration of the scanning in units of 10 milliseconds. If set to 0x0000, scanning will continue until it is explicitly disabled. */
#define SCAN_DURATION_WITELIST      600                                /**< Duration of the scanning in units of 10 milliseconds. */

#define MIN_CONNECTION_INTERVAL     MSEC_TO_UNITS(7.5, UNIT_1_25_MS)    /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL     MSEC_TO_UNITS(30, UNIT_1_25_MS)     /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY               2                                   /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT         MSEC_TO_UNITS(4000, UNIT_10_MS)     /**< Determines supervision time-out in units of 10 millisecond. */


#define TARGET_NAME                 "climberAP"


typedef enum {
	eNusTransferStateIdle,
	eNusTransferStateInit,
	eNusTransferStateRun,
	eNusTransferStateWait,
	eNusTransferStateFinish
} eNusTransferState;


NRF_BLE_SCAN_DEF(m_scan);
BLE_NUS_C_DEF(m_ble_nus_c);
NRF_BLE_GATT_DEF(m_gatt);                                           /**< GATT module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                    /**< DB discovery module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                /**< BLE GATT Queue instance. */
        NRF_SDH_BLE_CENTRAL_LINK_COUNT,
        NRF_BLE_GQ_QUEUE_SIZE);

static bool                  m_retry_db_disc;              /**< Flag to keep track of whether the DB discovery should be retried. */
static uint16_t              m_pending_db_disc_conn = BLE_CONN_HANDLE_INVALID;  /**< Connection handle for which the DB discovery is retried. */

static volatile bool m_nus_cts = false;
static volatile bool m_connected = false;
static uint16_t m_nus_packet_nb = 0;
static uint16_t m_mtu_length = 20;

static eNusTransferState m_nus_xfer_state = eNusTransferStateIdle;

static task_id_t m_periph_id = TASK_ID_INVALID;

typedef struct {
	uint8_t p_xfer_str[200];
	uint16_t length;
	uint16_t p_xfer_idx;
} sNusXfer;

NRF_QUEUE_DEF(sNusXfer, m_tx_queue, 10, NRF_QUEUE_MODE_NO_OVERFLOW);

static sNusXfer m_nus_xfer_rx_array;

static void scan_start(void);



/**@brief Function for handling database discovery events.
 *
 * @details This function is callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
	ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}

/**@brief Function for handling the LED Button Service client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void _service_c_error_handler(uint32_t nrf_error)
{
	if (nrf_error == NRF_ERROR_RESOURCES) {

		LOG_DEBUG("NUS RESSSS %u", m_nus_packet_nb);
		m_nus_cts = false;
		return;
	}
	APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
	ret_code_t            err_code;
	ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

	W_SYSVIEW_RecordEnterISR();

	switch (p_ble_evt->header.evt_id)
	{
	case BLE_GAP_EVT_CONNECTED:
	{
		LOG_INFO("GAP Connected.");
		m_pending_db_disc_conn = p_ble_evt->evt.gap_evt.conn_handle;
		m_retry_db_disc = false;
		// Discover peer's services.
		err_code = ble_db_discovery_start(&m_db_disc, m_pending_db_disc_conn);
		if (err_code == NRF_ERROR_BUSY)
		{
			LOG_INFO("ble_db_discovery_start() returned busy, will retry later.");
			m_retry_db_disc = true;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}

        const uint16_t mtu_desired = 200;

        LOG_INFO("mtu of %d requested", mtu_desired);
        err_code = nrf_ble_gatt_data_length_set(&m_gatt, p_ble_evt->evt.gap_evt.conn_handle, mtu_desired);      // UPDATE MTU HERE
        APP_ERROR_CHECK(err_code);

	} break;

	case BLE_GAP_EVT_DISCONNECTED:
	{
		LOG_INFO("Disconnected, reason 0x%x.",
				p_ble_evt->evt.gap_evt.params.disconnected.reason);

		m_connected = false;

		// Reset DB discovery structure.
		memset(&m_db_disc, 0 , sizeof (m_db_disc));

		scan_start();

	} break;

	case BLE_GAP_EVT_TIMEOUT:
	{
		if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
		{
			NRF_LOG_DEBUG("Scan timed out.");
			scan_start();
		}
		else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
		{
			LOG_INFO("Connection Request timed out.");
		}
	} break;


	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		// Pairing not supported.
		err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
		// Accepting parameters requested by peer.
		err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
				&p_gap_evt->params.conn_param_update_request.conn_params);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
	{
		NRF_LOG_DEBUG("PHY update request.");
		ble_gap_phys_t const phys =
		{
				.rx_phys = BLE_GAP_PHY_AUTO,
				.tx_phys = BLE_GAP_PHY_AUTO,
		};
		err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
		APP_ERROR_CHECK(err_code);
	} break;

	case BLE_GATTC_EVT_TIMEOUT:
		// Disconnect on GATT Client timeout event.
		LOG_INFO("GATT Client Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
				BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTS_EVT_TIMEOUT:
		// Disconnect on GATT Server timeout event.
		LOG_INFO("GATT Server Timeout.");
		err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
				BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
		break;

	case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE:
		LOG_DEBUG("GATTC WRITE_CMD_TX Complete %u", m_nus_cts);

		// clear to send more packets
		m_nus_cts = true;

		// unblock NUS servicing task
		if (m_periph_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_periph_id);
		}
		break;

	case BLE_GATTS_EVT_HVN_TX_COMPLETE:
		// unused here
		LOG_INFO("GATTC HVN_TX Complete");
		break;

	default:
		break;
	}


	W_SYSVIEW_RecordExitISR();

}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
	ret_code_t err_code;

	// Configure the BLE stack using the default settings.
	// Fetch the start address of the application RAM.
	uint32_t ram_start = 0;
	err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = nrf_sdh_ble_enable(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Register handlers for BLE and SoC events.
	NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

	// set name
	ble_gap_conn_sec_mode_t sec_mode; // Struct to store security parameters
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	/*Get this device name*/
	uint8_t device_name[20];
	memset(device_name, 0, sizeof(device_name));
	memcpy(device_name, BLE_DEVICE_NAME, strlen(BLE_DEVICE_NAME));
	err_code = sd_ble_gap_device_name_set(&sec_mode, device_name, strlen(BLE_DEVICE_NAME));
}

/**@brief Battery level Collector Handler.
 */
static void nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_evt)
{
	ret_code_t err_code;

	switch (p_evt->evt_type)
	{
	case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
		LOG_INFO("Discovery complete.");
		err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_evt->conn_handle, &p_evt->handles);
		APP_ERROR_CHECK(err_code);

		err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
		APP_ERROR_CHECK(err_code);
		LOG_INFO("Connected to stravaAP");
		m_connected = true;
		m_nus_cts = true;
		break;

	case BLE_NUS_C_EVT_NUS_TX_EVT:
		// handle received chars
		LOG_DEBUG("Received %u chars from BLE !", p_evt->data_len);

		{
			m_nus_xfer_rx_array.length = p_evt->data_len;
			memcpy(m_nus_xfer_rx_array.p_xfer_str, p_evt->p_data, p_evt->data_len);

			// unblock NUS servicing task
			if (m_periph_id != TASK_ID_INVALID) {
				w_task_delay_cancel(m_periph_id);
			}
			break;
		}
		break;

	case BLE_NUS_C_EVT_DISCONNECTED:
		if (m_nus_xfer_state == eNusTransferStateRun) m_nus_xfer_state = eNusTransferStateFinish;
		break;
	}
}

/**
 * @brief Battery level collector initialization.
 */
static void nus_c_init(void)
{
	ble_nus_c_init_t nus_c_init_obj;

	nus_c_init_obj.evt_handler = nus_c_evt_handler;
	nus_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
	nus_c_init_obj.error_handler = _service_c_error_handler;

	uint32_t err_code = ble_nus_c_init(&m_ble_nus_c, &nus_c_init_obj);
	APP_ERROR_CHECK(err_code);
}


/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
	ble_db_discovery_init_t db_init;

	memset(&db_init, 0, sizeof(db_init));

	db_init.evt_handler  = db_disc_handler;
	db_init.p_gatt_queue = &m_ble_gatt_queue;

	ret_code_t err_code = ble_db_discovery_init(&db_init);
	APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling Scanning Module events.
 */
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
	ret_code_t err_code;

	switch(p_scan_evt->scan_evt_id)
	{
	case NRF_BLE_SCAN_EVT_FILTER_MATCH:
	{
		NRF_LOG_INFO("Filter match: %x", p_scan_evt->params.filter_match.filter_match.uuid_filter_match);
	} break;

	case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
	{
		err_code = p_scan_evt->params.connecting_err.err_code;
		APP_ERROR_CHECK(err_code);
	} break;

	case NRF_BLE_SCAN_EVT_CONNECTED:
	{
		ble_gap_evt_connected_t const * p_connected =
				p_scan_evt->params.connected.p_connected;
		// Scan is automatically stopped by the connection.
		NRF_LOG_DEBUG("Connecting to target %02x%02x%02x%02x%02x%02x",
				p_connected->peer_addr.addr[0],
				p_connected->peer_addr.addr[1],
				p_connected->peer_addr.addr[2],
				p_connected->peer_addr.addr[3],
				p_connected->peer_addr.addr[4],
				p_connected->peer_addr.addr[5]
		);
	} break;

	case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
	{
		NRF_LOG_INFO("Scan timed out.");
		scan_start();
	} break;

	default:
		break;
	}
}


/**@brief Function to start scanning.
 */
static void scan_init(void)
{
	ret_code_t          err_code;
	nrf_ble_scan_init_t init_scan;

	memset(&init_scan, 0, sizeof(init_scan));

	init_scan.connect_if_match = true;
	init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

	err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, TARGET_NAME);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_NAME_FILTER, false);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function to start scanning.
 */
static void scan_start(void)
{
	ret_code_t ret;

	ret = nrf_ble_scan_start(&m_scan);
	APP_ERROR_CHECK(ret);
}

/**@brief Function to start scanning.
 */
static void scan_stop(void)
{
	nrf_ble_scan_stop();
}

/**@brief GATT module event handler.
 */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
	switch (p_evt->evt_id)
	{
	case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
	{

		LOG_INFO("Desired MTU: central %u peripheral %u",
				p_gatt->att_mtu_desired_central,
				p_gatt->att_mtu_desired_periph);

		LOG_INFO("ATT MTU exchange completed. MTU set to %u bytes.",
				p_evt->params.att_mtu_effective);

		m_mtu_length = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;

	} break;

	case NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED:
	{
		LOG_INFO("Data length for connection 0x%x updated to %d.",
				p_evt->conn_handle,
				p_evt->params.data_length);
	} break;

	default:
		break;
	}

	if (m_retry_db_disc)
	{
		LOG_INFO("Retrying DB discovery.");

		m_retry_db_disc = false;

		// Discover peer's services.
		ret_code_t err_code;
		err_code = ble_db_discovery_start(&m_db_disc, m_pending_db_disc_conn);

		if (err_code == NRF_ERROR_BUSY)
		{
			LOG_INFO("ble_db_discovery_start() returned busy, will retry later.");
			m_retry_db_disc = true;
		}
		else
		{
			APP_ERROR_CHECK(err_code);
		}
	}
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
	APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
	APP_ERROR_CHECK(err_code);
}

void ble_start_evt(eBleEventType evt) {



}

/**
 * Init BLE stack
 */
void ble_init(void)
{
	ble_stack_init();

	gatt_init();

	db_discovery_init();

	nus_c_init();

	scan_init();

	// Start scanning for peripherals and initiate connection
	// with devices
	scan_start();
}

void ble_uninit(void) {

	if (!m_connected) {

		LOG_WARNING("BLE Scan stop");
		scan_stop();
	} else {

		LOG_WARNING("BLE Disconnection");

		uint32_t err_code = sd_ble_gap_disconnect(m_pending_db_disc_conn,
				BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
		APP_ERROR_CHECK(err_code);
	}

}

void ble_nus_log_text(const char * text) {

	if (!text) return;

	if (!nrf_queue_is_full(&m_tx_queue)) {

		sNusXfer _nus_xfer_array;

		memset(&_nus_xfer_array, 0, sizeof(_nus_xfer_array));

		// create log
		_nus_xfer_array.length = snprintf(
			(char*)_nus_xfer_array.p_xfer_str,
			sizeof(_nus_xfer_array.p_xfer_str),
			text);

		ret_code_t err_code = nrf_queue_push(&m_tx_queue, &_nus_xfer_array);
		APP_ERROR_CHECK(err_code);

		if (m_periph_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_periph_id);
		}
	}

}

void ble_nus_log(const char* format, ...)
{
	if (!nrf_queue_is_full(&m_tx_queue)) {

		va_list va;
		sNusXfer _nus_xfer_array;

		memset(&_nus_xfer_array, 0, sizeof(_nus_xfer_array));

		va_start(va, format);
		_nus_xfer_array.length = vsnprintf((char*)_nus_xfer_array.p_xfer_str, sizeof(_nus_xfer_array.p_xfer_str), format, va);
		va_end(va);

		if (_nus_xfer_array.length + 3 < sizeof(_nus_xfer_array.p_xfer_str)) {
			_nus_xfer_array.p_xfer_str[_nus_xfer_array.length++] = '\r';
			_nus_xfer_array.p_xfer_str[_nus_xfer_array.length++] = '\n';
		}

		ret_code_t err_code = nrf_queue_push(&m_tx_queue, &_nus_xfer_array);
		APP_ERROR_CHECK(err_code);

		if (m_periph_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_periph_id);
		}
	}
}

/**
 * Send the log file to a remote computer
 */
void ble_nus_tasks(void) {

	static sNusXfer m_nus_xfer_tx_array;
	m_periph_id = w_task_id_get();

	if (m_nus_xfer_rx_array.length) {

		// decode
		switch(m_nus_xfer_rx_array.p_xfer_str[0]) {

		// ZPM ERG mode information
		case '>':
		{
			// decode packet
			zpm_decoder__handle(&m_nus_xfer_rx_array.p_xfer_str[1], m_nus_xfer_rx_array.length);
		} break;

		// calibration commands
		case 'U':
		{
			for (int i=0; i < m_nus_xfer_rx_array.length; i++) {
				if (m_nus_xfer_rx_array.p_xfer_str[0] == 'U') data_dispatcher__offset_calibration(  5 );
			}
		} break;
		case 'D':
		{
			for (int i=0; i < m_nus_xfer_rx_array.length; i++) {
				if (m_nus_xfer_rx_array.p_xfer_str[0] == 'D') data_dispatcher__offset_calibration( -5 );
			}
		} break;

		default:
			LOG_WARNING("Unknown NUS packet");
			break;
		}

		memset(&m_nus_xfer_rx_array, 0, sizeof(m_nus_xfer_rx_array));
	}

	if (!m_nus_xfer_tx_array.length && !nrf_queue_is_empty(&m_tx_queue)) {
		ret_code_t err_code = nrf_queue_pop(&m_tx_queue, &m_nus_xfer_tx_array);
		APP_ERROR_CHECK(err_code);
	}

	while (m_connected &&
			m_ble_nus_c.conn_handle != BLE_CONN_HANDLE_INVALID &&
			m_nus_xfer_tx_array.length &&
			m_nus_cts) {

		uint32_t err_code = 0;

		if (m_nus_xfer_tx_array.length > m_mtu_length + m_nus_xfer_tx_array.p_xfer_idx) {

			err_code = ble_nus_c_string_send(&m_ble_nus_c, &m_nus_xfer_tx_array.p_xfer_str[m_nus_xfer_tx_array.p_xfer_idx], m_mtu_length);

			if (err_code == 0 &&
					m_nus_cts) {

				// in this case the transfer is a success: prepare for next
				m_nus_xfer_tx_array.p_xfer_idx += m_mtu_length;
			}

		} else {

			if (m_nus_xfer_tx_array.p_xfer_idx > 0) {

				// need to finish sending
				err_code = ble_nus_c_string_send(&m_ble_nus_c,
						&m_nus_xfer_tx_array.p_xfer_str[m_nus_xfer_tx_array.p_xfer_idx],
						m_nus_xfer_tx_array.length - m_nus_xfer_tx_array.p_xfer_idx);

				if (err_code == 0 &&
						m_nus_cts) {

					// in this case the transfer is a success
					m_nus_xfer_tx_array.p_xfer_idx = 0;
				}
			} else {

				err_code = ble_nus_c_string_send(&m_ble_nus_c, m_nus_xfer_tx_array.p_xfer_str, m_nus_xfer_tx_array.length);
			}
		}

		switch (err_code) {
		case NRF_ERROR_BUSY:
			LOG_WARNING("NUS BUSY");
			return;
			break;

		case NRF_ERROR_RESOURCES:
			LOG_DEBUG("NUS RESSSS %u", m_nus_packet_nb);
			m_nus_cts = false;
			break;

		case NRF_ERROR_TIMEOUT:
			LOG_WARNING("NUS timeout", err_code);
			return;
			break;

		case NRF_SUCCESS:
		{
			if (m_nus_cts && !m_nus_xfer_tx_array.p_xfer_idx) {
				LOG_DEBUG("Packet %u sent size %u", m_nus_packet_nb, m_nus_xfer_tx_array.length);

				m_nus_packet_nb++;
				m_nus_xfer_tx_array.length = 0;
				m_nus_xfer_tx_array.p_xfer_idx = 0;

				if (!nrf_queue_is_empty(&m_tx_queue)) {
					ret_code_t err_code = nrf_queue_pop(&m_tx_queue, &m_nus_xfer_tx_array);
					APP_ERROR_CHECK(err_code);
				}
			}

		} break;

		default:
		{
			LOG_WARNING("NUS unknown error: 0x%X MTU %u / %u", err_code, m_nus_xfer_tx_array.length, m_mtu_length);
			return;
		} break;
		}

	}

}


#endif


