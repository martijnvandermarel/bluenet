/**
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Oct 9, 2019
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <cfg/cs_AutoConfig.h>
#include <cfg/cs_Boards.h>
#include <cfg/cs_Config.h>
#include <cfg/cs_StaticConfig.h>
#include <common/cs_Types.h>
#include <storage/cs_StateData.h>
#include <util/cs_UuidParser.h>

cs_ret_code_t getDefault(cs_state_data_t & data, const boards_config_t& boardsConfig)  {

	// for all non-string types we already know the to-be expected size
	data.size = TypeSize(data.type);

	switch (data.type) {
	case CS_TYPE::CONFIG_NAME: {
		data.size = MIN(data.size, sizeof(STRINGIFY(BLUETOOTH_NAME)));
		memcpy(data.value, STRINGIFY(BLUETOOTH_NAME), data.size);
		return ERR_SUCCESS;
	}
	case CS_TYPE::CONFIG_MESH_ENABLED:
		*(TYPIFY(CONFIG_MESH_ENABLED)*)data.value = g_CONFIG_MESH_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_ENCRYPTION_ENABLED:
		*(TYPIFY(CONFIG_ENCRYPTION_ENABLED)*)data.value = g_CONFIG_ENCRYPTION_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_IBEACON_ENABLED:
		*(TYPIFY(CONFIG_IBEACON_ENABLED)*)data.value = g_CONFIG_IBEACON_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SCANNER_ENABLED:
		*(TYPIFY(CONFIG_SCANNER_ENABLED)*)data.value = g_CONFIG_SCANNER_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_PWM_ALLOWED:
		*(TYPIFY(CONFIG_PWM_ALLOWED)*)data.value = g_CONFIG_PWM_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_START_DIMMER_ON_ZERO_CROSSING:
		*(TYPIFY(CONFIG_START_DIMMER_ON_ZERO_CROSSING)*)data.value = g_CONFIG_START_DIMMER_ON_ZERO_CROSSING_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SWITCH_LOCKED:
		*(TYPIFY(CONFIG_SWITCH_LOCKED)*)data.value = g_CONFIG_SWITCH_LOCK_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SWITCHCRAFT_ENABLED:
		*(TYPIFY(CONFIG_SWITCHCRAFT_ENABLED)*)data.value = g_CONFIG_SWITCHCRAFT_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_TAP_TO_TOGGLE_ENABLED:
		*(TYPIFY(CONFIG_TAP_TO_TOGGLE_ENABLED)*)data.value = g_CONFIG_TAP_TO_TOGGLE_ENABLED_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_TAP_TO_TOGGLE_RSSI_THRESHOLD_OFFSET:
		*(TYPIFY(CONFIG_TAP_TO_TOGGLE_RSSI_THRESHOLD_OFFSET)*)data.value = g_CONFIG_TAP_TO_TOGGLE_RSSI_THRESHOLD_OFFSET_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_IBEACON_MAJOR:
		*(TYPIFY(CONFIG_IBEACON_MAJOR)*)data.value = g_BEACON_MAJOR;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_IBEACON_MINOR:
		*(TYPIFY(CONFIG_IBEACON_MINOR)*)data.value = g_BEACON_MINOR;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_IBEACON_UUID: {
		std::string uuidString = g_BEACON_UUID;
		if (BLEutil::parseUuid(uuidString.c_str(), uuidString.size(), data.value, data.size)) {
			return ERR_SUCCESS;
		}
		return ERR_WRONG_PAYLOAD_LENGTH;
	}
	case CS_TYPE::CONFIG_IBEACON_TXPOWER:
		*(TYPIFY(CONFIG_IBEACON_TXPOWER)*)data.value = g_BEACON_RSSI;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_TX_POWER:
		*(TYPIFY(CONFIG_TX_POWER)*)data.value = g_TX_POWER;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_ADV_INTERVAL:
		*(TYPIFY(CONFIG_ADV_INTERVAL)*)data.value = g_ADVERTISEMENT_INTERVAL;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SCAN_DURATION:
		*(TYPIFY(CONFIG_SCAN_DURATION)*)data.value = g_SCAN_DURATION;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SCAN_BREAK_DURATION:
		*(TYPIFY(CONFIG_SCAN_BREAK_DURATION)*)data.value = g_SCAN_BREAK_DURATION;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_BOOT_DELAY:
		*(TYPIFY(CONFIG_BOOT_DELAY)*)data.value = g_BOOT_DELAY;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_MAX_CHIP_TEMP:
		*(TYPIFY(CONFIG_MAX_CHIP_TEMP)*)data.value = g_MAX_CHIP_TEMPERATURE;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SPHERE_ID:
		*(TYPIFY(CONFIG_SPHERE_ID)*)data.value = g_CONFIG_SPHERE_ID_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_CROWNSTONE_ID:
		*(TYPIFY(CONFIG_CROWNSTONE_ID)*)data.value = g_CONFIG_CROWNSTONE_ID_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_KEY_ADMIN:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_KEY_MEMBER:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_KEY_BASIC:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_KEY_SERVICE_DATA:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_MESH_DEVICE_KEY:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_MESH_APP_KEY:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_MESH_NET_KEY:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_KEY_LOCALIZATION:
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SCAN_INTERVAL_625US:
		*(TYPIFY(CONFIG_SCAN_INTERVAL_625US)*)data.value = boardsConfig.scanIntervalUs / 625;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SCAN_WINDOW_625US:
		*(TYPIFY(CONFIG_SCAN_WINDOW_625US)*)data.value = boardsConfig.scanWindowUs / 625;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_RELAY_HIGH_DURATION:
		*(TYPIFY(CONFIG_RELAY_HIGH_DURATION)*)data.value = g_CONFIG_RELAY_HIGH_DURATION_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_LOW_TX_POWER:
		*(TYPIFY(CONFIG_LOW_TX_POWER)*)data.value = boardsConfig.minTxPower;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_VOLTAGE_MULTIPLIER:
		*(TYPIFY(CONFIG_VOLTAGE_MULTIPLIER)*)data.value = boardsConfig.voltageMultiplier;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_CURRENT_MULTIPLIER:
		*(TYPIFY(CONFIG_CURRENT_MULTIPLIER)*)data.value = boardsConfig.currentMultiplier;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_VOLTAGE_ADC_ZERO:
		*(TYPIFY(CONFIG_VOLTAGE_ADC_ZERO)*)data.value = boardsConfig.voltageZero;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_CURRENT_ADC_ZERO:
		*(TYPIFY(CONFIG_CURRENT_ADC_ZERO)*)data.value = boardsConfig.currentZero;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_POWER_ZERO:
		*(TYPIFY(CONFIG_POWER_ZERO)*)data.value = CONFIG_POWER_ZERO_INVALID;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_PWM_PERIOD: {
		*(TYPIFY(CONFIG_PWM_PERIOD)*)data.value = (TYPIFY(CONFIG_PWM_PERIOD))PWM_PERIOD;
		return ERR_SUCCESS;
	}
	case CS_TYPE::CONFIG_SOFT_FUSE_CURRENT_THRESHOLD:
		*(TYPIFY(CONFIG_SOFT_FUSE_CURRENT_THRESHOLD)*)data.value = CURRENT_USAGE_THRESHOLD;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SOFT_FUSE_CURRENT_THRESHOLD_DIMMER:
		*(TYPIFY(CONFIG_SOFT_FUSE_CURRENT_THRESHOLD_DIMMER)*)data.value = CURRENT_USAGE_THRESHOLD_DIMMER;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_UP:
		*(TYPIFY(CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_UP)*)data.value = boardsConfig.pwmTempVoltageThreshold;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_DOWN:
		*(TYPIFY(CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_DOWN)*)data.value = boardsConfig.pwmTempVoltageThresholdDown;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_SWITCHCRAFT_THRESHOLD:
		*(TYPIFY(CONFIG_SWITCHCRAFT_THRESHOLD)*)data.value = SWITCHCRAFT_THRESHOLD;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_UART_ENABLED:
		*(TYPIFY(CONFIG_UART_ENABLED)*)data.value = g_CS_SERIAL_ENABLED;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_RESET_COUNTER:
		*(TYPIFY(STATE_RESET_COUNTER)*)data.value = STATE_RESET_COUNTER_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_SWITCH_STATE:
		reinterpret_cast<TYPIFY(STATE_SWITCH_STATE)*>(data.value)->asInt = STATE_SWITCH_STATE_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_ACCUMULATED_ENERGY:
		*(TYPIFY(STATE_ACCUMULATED_ENERGY)*)data.value = STATE_ACCUMULATED_ENERGY_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_POWER_USAGE:
		*(TYPIFY(STATE_POWER_USAGE)*)data.value = STATE_POWER_USAGE_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_OPERATION_MODE:
		*(TYPIFY(STATE_OPERATION_MODE)*)data.value = STATE_OPERATION_MODE_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_TEMPERATURE:
		*(TYPIFY(STATE_TEMPERATURE)*)data.value = STATE_TEMPERATURE_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_SUN_TIME:
		*reinterpret_cast<TYPIFY(STATE_SUN_TIME)*>(data.value) = sun_time_t();
		return ERR_SUCCESS;
	case CS_TYPE::STATE_FACTORY_RESET:
		*(TYPIFY(STATE_FACTORY_RESET)*)data.value = STATE_FACTORY_RESET_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_ERRORS:
		reinterpret_cast<TYPIFY(STATE_ERRORS)*>(data.value)->asInt = STATE_ERRORS_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::CONFIG_CURRENT_LIMIT:
		return ERR_NOT_IMPLEMENTED;
	case CS_TYPE::CONFIG_DO_NOT_USE:
		return ERR_NOT_AVAILABLE;
	case CS_TYPE::STATE_BEHAVIOUR_RULE:
		return ERR_NOT_AVAILABLE;
	case CS_TYPE::STATE_TWILIGHT_RULE:
		return ERR_NOT_AVAILABLE;
	case CS_TYPE::STATE_EXTENDED_BEHAVIOUR_RULE:
		return ERR_NOT_AVAILABLE;
	case CS_TYPE::STATE_BEHAVIOUR_SETTINGS:
		reinterpret_cast<TYPIFY(STATE_BEHAVIOUR_SETTINGS)*>(data.value)->asInt = STATE_BEHAVIOUR_SETTINGS_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_BEHAVIOUR_MASTER_HASH:
		*(TYPIFY(STATE_BEHAVIOUR_MASTER_HASH)*)data.value = STATE_BEHAVIOUR_MASTER_HASH_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_MESH_IV_INDEX:
		reinterpret_cast<TYPIFY(STATE_MESH_IV_INDEX)*>(data.value)->iv_index = STATE_MESH_IV_INDEX_DEFAULT;
		reinterpret_cast<TYPIFY(STATE_MESH_IV_INDEX)*>(data.value)->iv_update_in_progress = STATE_MESH_IV_STATUS_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_MESH_SEQ_NUMBER:
		*reinterpret_cast<TYPIFY(STATE_MESH_SEQ_NUMBER)*>(data.value) = STATE_MESH_SEQ_NUMBER_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_IBEACON_CONFIG_ID: {
//		*reinterpret_cast<TYPIFY(STATE_IBEACON_CONFIG_ID)*>(data.value) = TYPIFY(STATE_IBEACON_CONFIG_ID)();
		*reinterpret_cast<TYPIFY(STATE_IBEACON_CONFIG_ID)*>(data.value) = ibeacon_config_id_packet_t();
//		TYPIFY(STATE_IBEACON_CONFIG_ID)* config = reinterpret_cast<TYPIFY(STATE_IBEACON_CONFIG_ID)*>(data.value);
//		config->timestamp = 0;
//		config->interval = 0;
		return ERR_SUCCESS;
	}
	case CS_TYPE::STATE_MICROAPP: {
		*reinterpret_cast<TYPIFY(STATE_MICROAPP)*>(data.value) = microapp_state_t();
		return ERR_SUCCESS;
	}
	case CS_TYPE::STATE_SOFT_ON_SPEED:
		*(TYPIFY(STATE_SOFT_ON_SPEED)*)data.value = DIMMER_SOFT_ON_SPEED;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_HUB_MODE:
		*(TYPIFY(STATE_HUB_MODE)*)data.value = STATE_HUB_MODE_DEFAULT;
		return ERR_SUCCESS;
	case CS_TYPE::STATE_UART_KEY:
		memset(data.value, 0, TypeSize(CS_TYPE::STATE_UART_KEY));
		return ERR_SUCCESS;
	case CS_TYPE::CMD_CONTROL_CMD:
	case CS_TYPE::CMD_DEC_CURRENT_RANGE:
	case CS_TYPE::CMD_DEC_VOLTAGE_RANGE:
	case CS_TYPE::CMD_ENABLE_ADC_DIFFERENTIAL_CURRENT:
	case CS_TYPE::CMD_ENABLE_ADC_DIFFERENTIAL_VOLTAGE:
	case CS_TYPE::CMD_ENABLE_ADVERTISEMENT:
	case CS_TYPE::CMD_ENABLE_LOG_CURRENT:
	case CS_TYPE::CMD_ENABLE_LOG_FILTERED_CURRENT:
	case CS_TYPE::CMD_ENABLE_LOG_POWER:
	case CS_TYPE::CMD_ENABLE_LOG_VOLTAGE:
	case CS_TYPE::CMD_ENABLE_MESH:
	case CS_TYPE::CMD_FACTORY_RESET:
	case CS_TYPE::CMD_INC_CURRENT_RANGE:
	case CS_TYPE::CMD_INC_VOLTAGE_RANGE:
	case CS_TYPE::CMD_RESET_DELAYED:
	case CS_TYPE::CMD_SEND_MESH_MSG:
	case CS_TYPE::CMD_SEND_MESH_MSG_MULTI_SWITCH:
	case CS_TYPE::CMD_SEND_MESH_MSG_PROFILE_LOCATION:
	case CS_TYPE::CMD_SEND_MESH_MSG_SET_BEHAVIOUR_SETTINGS:
	case CS_TYPE::CMD_SET_TIME:
	case CS_TYPE::CMD_SWITCH_OFF:
	case CS_TYPE::CMD_SWITCH_ON:
	case CS_TYPE::CMD_SWITCH_TOGGLE:
	case CS_TYPE::CMD_SWITCH:
	case CS_TYPE::CMD_MULTI_SWITCH:
	case CS_TYPE::CMD_TOGGLE_ADC_VOLTAGE_VDD_REFERENCE_PIN:
	case CS_TYPE::EVT_ADC_RESTARTED:
	case CS_TYPE::EVT_ADV_BACKGROUND:
	case CS_TYPE::EVT_ADV_BACKGROUND_PARSED:
	case CS_TYPE::EVT_ADV_BACKGROUND_PARSED_V1:
	case CS_TYPE::EVT_ADVERTISEMENT_UPDATED:
	case CS_TYPE::CMD_BLE_CENTRAL_CONNECT:
	case CS_TYPE::CMD_BLE_CENTRAL_DISCONNECT:
	case CS_TYPE::CMD_BLE_CENTRAL_DISCOVER:
	case CS_TYPE::CMD_BLE_CENTRAL_READ:
	case CS_TYPE::CMD_BLE_CENTRAL_WRITE:
	case CS_TYPE::EVT_BLE_CONNECT:
	case CS_TYPE::EVT_BLE_DISCONNECT:
	case CS_TYPE::EVT_BLE_CENTRAL_CONNECT_START:
	case CS_TYPE::EVT_BLE_CENTRAL_CONNECT_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCONNECTED:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_READ_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_WRITE_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_NOTIFICATION:
	case CS_TYPE::CMD_CS_CENTRAL_CONNECT:
	case CS_TYPE::CMD_CS_CENTRAL_DISCONNECT:
	case CS_TYPE::CMD_CS_CENTRAL_WRITE:
	case CS_TYPE::EVT_CS_CENTRAL_CONNECT_RESULT:
	case CS_TYPE::EVT_CS_CENTRAL_READ_RESULT:
	case CS_TYPE::EVT_CS_CENTRAL_WRITE_RESULT:
	case CS_TYPE::EVT_BROWNOUT_IMPENDING:
	case CS_TYPE::EVT_CHIP_TEMP_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_CHIP_TEMP_OK:
	case CS_TYPE::EVT_CURRENT_USAGE_ABOVE_THRESHOLD_DIMMER:
	case CS_TYPE::EVT_CURRENT_USAGE_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_DEVICE_SCANNED:
	case CS_TYPE::EVT_DIMMER_FORCED_OFF:
	case CS_TYPE::EVT_DIMMER_OFF_FAILURE_DETECTED:
	case CS_TYPE::EVT_DIMMER_ON_FAILURE_DETECTED:
	case CS_TYPE::EVT_DIMMER_POWERED:
	case CS_TYPE::EVT_DIMMER_TEMP_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_DIMMER_TEMP_OK:
	case CS_TYPE::CMD_DIMMING_ALLOWED:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_REGISTER:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_REGISTER:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_TOKEN:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_TOKEN:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_LIST_SIZE:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_LIST_SIZE:
	case CS_TYPE::CMD_SEND_MESH_CONTROL_COMMAND:
	case CS_TYPE::EVT_RELAY_FORCED_ON:
	case CS_TYPE::EVT_SCAN_STARTED:
	case CS_TYPE::EVT_SCAN_STOPPED:
	case CS_TYPE::EVT_SESSION_DATA_SET:
	case CS_TYPE::EVT_SETUP_DONE:
	case CS_TYPE::EVT_STATE_EXTERNAL_STONE:
	case CS_TYPE::EVT_STATE_FACTORY_RESET_DONE:
	case CS_TYPE::EVT_STORAGE_INITIALIZED:
	case CS_TYPE::EVT_STORAGE_WRITE_DONE:
	case CS_TYPE::EVT_STORAGE_REMOVE_DONE:
	case CS_TYPE::EVT_STORAGE_REMOVE_ALL_TYPES_WITH_ID_DONE:
	case CS_TYPE::EVT_STORAGE_GC_DONE:
	case CS_TYPE::EVT_STORAGE_FACTORY_RESET_DONE:
	case CS_TYPE::EVT_STORAGE_PAGES_ERASED:
	case CS_TYPE::EVT_MESH_FACTORY_RESET_DONE:
	case CS_TYPE::CMD_STORAGE_GARBAGE_COLLECT:
	case CS_TYPE::EVT_SWITCH_FORCED_OFF:
	case CS_TYPE::CMD_SWITCHING_ALLOWED:
	case CS_TYPE::EVT_TICK:
	case CS_TYPE::EVT_TIME_SET:
	case CS_TYPE::CMD_ADD_BEHAVIOUR:
	case CS_TYPE::CMD_REPLACE_BEHAVIOUR:
	case CS_TYPE::CMD_REMOVE_BEHAVIOUR:
	case CS_TYPE::CMD_GET_BEHAVIOUR:
	case CS_TYPE::CMD_GET_BEHAVIOUR_INDICES:
	case CS_TYPE::CMD_GET_BEHAVIOUR_DEBUG:
	case CS_TYPE::CMD_CLEAR_ALL_BEHAVIOUR:
	case CS_TYPE::EVT_BEHAVIOURSTORE_MUTATION:
	case CS_TYPE::EVT_BEHAVIOUR_OVERRIDDEN:
	case CS_TYPE::CMD_REGISTER_TRACKED_DEVICE:
	case CS_TYPE::CMD_UPDATE_TRACKED_DEVICE:
	case CS_TYPE::CMD_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::EVT_PRESENCE_MUTATION:
	case CS_TYPE::EVT_PRESENCE_CHANGE:
	case CS_TYPE::CMD_GET_PRESENCE:
	case CS_TYPE::CMD_SET_RELAY:
	case CS_TYPE::CMD_SET_DIMMER:
	case CS_TYPE::EVT_GOING_TO_DFU:
	case CS_TYPE::EVT_RECEIVED_PROFILE_LOCATION:
	case CS_TYPE::EVT_TRACKABLE:
	case CS_TYPE::CMD_UPLOAD_FILTER:
	case CS_TYPE::CMD_REMOVE_FILTER:
	case CS_TYPE::CMD_COMMIT_FILTER_CHANGES:
	case CS_TYPE::CMD_GET_FILTER_SUMMARIES:
	case CS_TYPE::EVT_MESH_SYNC_REQUEST_OUTGOING:
	case CS_TYPE::EVT_MESH_SYNC_REQUEST_INCOMING:
	case CS_TYPE::EVT_MESH_SYNC_FAILED:
	case CS_TYPE::EVT_MESH_PAGES_ERASED:
	case CS_TYPE::EVT_MESH_EXT_STATE_0:
	case CS_TYPE::EVT_MESH_EXT_STATE_1:
	case CS_TYPE::CMD_SEND_MESH_MSG_SET_TIME:
	case CS_TYPE::CMD_SET_IBEACON_CONFIG_ID:
	case CS_TYPE::CMD_SEND_MESH_MSG_NOOP:
	case CS_TYPE::EVT_MESH_RSSI_PING:
	case CS_TYPE::EVT_MESH_RSSI_DATA:
	case CS_TYPE::EVT_MESH_NEAREST_WITNESS_REPORT:
	case CS_TYPE::EVT_MESH_TIME_SYNC:
	case CS_TYPE::EVT_RECV_MESH_MSG:
	case CS_TYPE::CMD_GET_ADC_RESTARTS:
	case CS_TYPE::CMD_GET_SWITCH_HISTORY:
	case CS_TYPE::CMD_GET_POWER_SAMPLES:
	case CS_TYPE::CMD_GET_SCHEDULER_MIN_FREE:
	case CS_TYPE::CMD_GET_RESET_REASON:
	case CS_TYPE::CMD_GET_GPREGRET:
	case CS_TYPE::CMD_GET_ADC_CHANNEL_SWAPS:
	case CS_TYPE::CMD_GET_RAM_STATS:
	case CS_TYPE::EVT_GENERIC_TEST:
	case CS_TYPE::CMD_TEST_SET_TIME:
	case CS_TYPE::CMD_MICROAPP_GET_INFO:
	case CS_TYPE::CMD_MICROAPP_UPLOAD:
	case CS_TYPE::CMD_MICROAPP_VALIDATE:
	case CS_TYPE::CMD_MICROAPP_REMOVE:
	case CS_TYPE::CMD_MICROAPP_ENABLE:
	case CS_TYPE::CMD_MICROAPP_DISABLE:
	case CS_TYPE::EVT_MICROAPP_UPLOAD_RESULT:
	case CS_TYPE::EVT_MICROAPP_ERASE_RESULT:
	case CS_TYPE::CMD_MICROAPP_ADVERTISE:
	case CS_TYPE::EVT_HUB_DATA_REPLY:
	case CS_TYPE::EVT_TWI_INIT:
	case CS_TYPE::EVT_TWI_WRITE:
	case CS_TYPE::EVT_TWI_READ:
	case CS_TYPE::EVT_TWI_UPDATE:
	case CS_TYPE::EVT_GPIO_INIT:
	case CS_TYPE::EVT_GPIO_WRITE:
	case CS_TYPE::EVT_GPIO_READ:
	case CS_TYPE::EVT_GPIO_UPDATE:
		return ERR_NOT_FOUND;
	}
	return ERR_NOT_FOUND;
}

PersistenceMode DefaultLocation(CS_TYPE const & type) {
	switch (type) {
	case CS_TYPE::CONFIG_NAME:
	case CS_TYPE::CONFIG_PWM_PERIOD:
	case CS_TYPE::CONFIG_IBEACON_MAJOR:
	case CS_TYPE::CONFIG_IBEACON_MINOR:
	case CS_TYPE::CONFIG_IBEACON_UUID:
	case CS_TYPE::CONFIG_IBEACON_TXPOWER:
	case CS_TYPE::CONFIG_TX_POWER:
	case CS_TYPE::CONFIG_ADV_INTERVAL:
	case CS_TYPE::CONFIG_SCAN_DURATION:
	case CS_TYPE::CONFIG_SCAN_BREAK_DURATION:
	case CS_TYPE::CONFIG_BOOT_DELAY:
	case CS_TYPE::CONFIG_MAX_CHIP_TEMP:
	case CS_TYPE::CONFIG_CURRENT_LIMIT:
	case CS_TYPE::CONFIG_MESH_ENABLED:
	case CS_TYPE::CONFIG_ENCRYPTION_ENABLED:
	case CS_TYPE::CONFIG_IBEACON_ENABLED:
	case CS_TYPE::CONFIG_SCANNER_ENABLED:
	case CS_TYPE::CONFIG_SPHERE_ID:
	case CS_TYPE::CONFIG_CROWNSTONE_ID:
	case CS_TYPE::CONFIG_KEY_ADMIN:
	case CS_TYPE::CONFIG_KEY_MEMBER:
	case CS_TYPE::CONFIG_KEY_BASIC:
	case CS_TYPE::CONFIG_KEY_SERVICE_DATA:
	case CS_TYPE::CONFIG_MESH_DEVICE_KEY:
	case CS_TYPE::CONFIG_MESH_APP_KEY:
	case CS_TYPE::CONFIG_MESH_NET_KEY:
	case CS_TYPE::CONFIG_KEY_LOCALIZATION:
	case CS_TYPE::CONFIG_SCAN_INTERVAL_625US:
	case CS_TYPE::CONFIG_SCAN_WINDOW_625US:
	case CS_TYPE::CONFIG_RELAY_HIGH_DURATION:
	case CS_TYPE::CONFIG_LOW_TX_POWER:
	case CS_TYPE::CONFIG_VOLTAGE_MULTIPLIER:
	case CS_TYPE::CONFIG_CURRENT_MULTIPLIER:
	case CS_TYPE::CONFIG_VOLTAGE_ADC_ZERO:
	case CS_TYPE::CONFIG_CURRENT_ADC_ZERO:
	case CS_TYPE::CONFIG_POWER_ZERO:
	case CS_TYPE::CONFIG_SOFT_FUSE_CURRENT_THRESHOLD:
	case CS_TYPE::CONFIG_SOFT_FUSE_CURRENT_THRESHOLD_DIMMER:
	case CS_TYPE::CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_UP:
	case CS_TYPE::CONFIG_PWM_TEMP_VOLTAGE_THRESHOLD_DOWN:
	case CS_TYPE::CONFIG_PWM_ALLOWED:
	case CS_TYPE::CONFIG_START_DIMMER_ON_ZERO_CROSSING:
	case CS_TYPE::CONFIG_SWITCH_LOCKED:
	case CS_TYPE::CONFIG_SWITCHCRAFT_ENABLED:
	case CS_TYPE::CONFIG_SWITCHCRAFT_THRESHOLD:
	case CS_TYPE::CONFIG_TAP_TO_TOGGLE_ENABLED:
	case CS_TYPE::CONFIG_TAP_TO_TOGGLE_RSSI_THRESHOLD_OFFSET:
	case CS_TYPE::CONFIG_UART_ENABLED:
	case CS_TYPE::STATE_RESET_COUNTER:
	case CS_TYPE::STATE_OPERATION_MODE:
	case CS_TYPE::STATE_SWITCH_STATE:
	case CS_TYPE::STATE_BEHAVIOUR_RULE:
	case CS_TYPE::STATE_TWILIGHT_RULE:
	case CS_TYPE::STATE_EXTENDED_BEHAVIOUR_RULE:
	case CS_TYPE::STATE_BEHAVIOUR_SETTINGS:
	case CS_TYPE::STATE_SUN_TIME:
	case CS_TYPE::STATE_MESH_IV_INDEX:
	case CS_TYPE::STATE_MESH_SEQ_NUMBER:
	case CS_TYPE::STATE_IBEACON_CONFIG_ID:
	case CS_TYPE::STATE_MICROAPP:
	case CS_TYPE::STATE_SOFT_ON_SPEED:
	case CS_TYPE::STATE_HUB_MODE:
	case CS_TYPE::STATE_UART_KEY:
		return PersistenceMode::FLASH;
	case CS_TYPE::STATE_ACCUMULATED_ENERGY:
	case CS_TYPE::STATE_POWER_USAGE:
	case CS_TYPE::STATE_TEMPERATURE:
	case CS_TYPE::STATE_FACTORY_RESET:
	case CS_TYPE::STATE_ERRORS:
	case CS_TYPE::STATE_BEHAVIOUR_MASTER_HASH:
		return PersistenceMode::RAM;
	case CS_TYPE::CONFIG_DO_NOT_USE:
	case CS_TYPE::CMD_SWITCH_OFF:
	case CS_TYPE::CMD_SWITCH_ON:
	case CS_TYPE::CMD_SWITCH_TOGGLE:
	case CS_TYPE::CMD_SWITCH:
	case CS_TYPE::CMD_MULTI_SWITCH:
	case CS_TYPE::EVT_ADV_BACKGROUND:
	case CS_TYPE::EVT_ADV_BACKGROUND_PARSED:
	case CS_TYPE::EVT_ADV_BACKGROUND_PARSED_V1:
	case CS_TYPE::EVT_ADVERTISEMENT_UPDATED:
	case CS_TYPE::EVT_SCAN_STARTED:
	case CS_TYPE::EVT_SCAN_STOPPED:
	case CS_TYPE::EVT_DEVICE_SCANNED:
	case CS_TYPE::EVT_CURRENT_USAGE_ABOVE_THRESHOLD_DIMMER:
	case CS_TYPE::EVT_CURRENT_USAGE_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_DIMMER_ON_FAILURE_DETECTED:
	case CS_TYPE::EVT_DIMMER_OFF_FAILURE_DETECTED:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_REGISTER:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_REGISTER:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_TOKEN:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_TOKEN:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::EVT_MESH_TRACKED_DEVICE_LIST_SIZE:
	case CS_TYPE::CMD_SEND_MESH_MSG_TRACKED_DEVICE_LIST_SIZE:
	case CS_TYPE::CMD_SEND_MESH_CONTROL_COMMAND:
	case CS_TYPE::CMD_BLE_CENTRAL_CONNECT:
	case CS_TYPE::CMD_BLE_CENTRAL_DISCONNECT:
	case CS_TYPE::CMD_BLE_CENTRAL_DISCOVER:
	case CS_TYPE::CMD_BLE_CENTRAL_READ:
	case CS_TYPE::CMD_BLE_CENTRAL_WRITE:
	case CS_TYPE::EVT_BLE_CONNECT:
	case CS_TYPE::EVT_BLE_DISCONNECT:
	case CS_TYPE::EVT_BLE_CENTRAL_CONNECT_START:
	case CS_TYPE::EVT_BLE_CENTRAL_CONNECT_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCONNECTED:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY:
	case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_READ_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_WRITE_RESULT:
	case CS_TYPE::EVT_BLE_CENTRAL_NOTIFICATION:
	case CS_TYPE::CMD_CS_CENTRAL_CONNECT:
	case CS_TYPE::CMD_CS_CENTRAL_DISCONNECT:
	case CS_TYPE::CMD_CS_CENTRAL_WRITE:
	case CS_TYPE::EVT_CS_CENTRAL_CONNECT_RESULT:
	case CS_TYPE::EVT_CS_CENTRAL_READ_RESULT:
	case CS_TYPE::EVT_CS_CENTRAL_WRITE_RESULT:
	case CS_TYPE::EVT_BROWNOUT_IMPENDING:
	case CS_TYPE::EVT_SESSION_DATA_SET:
	case CS_TYPE::EVT_DIMMER_FORCED_OFF:
	case CS_TYPE::EVT_SWITCH_FORCED_OFF:
	case CS_TYPE::EVT_RELAY_FORCED_ON:
	case CS_TYPE::EVT_CHIP_TEMP_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_CHIP_TEMP_OK:
	case CS_TYPE::EVT_DIMMER_TEMP_ABOVE_THRESHOLD:
	case CS_TYPE::EVT_DIMMER_TEMP_OK:
	case CS_TYPE::EVT_TICK:
	case CS_TYPE::EVT_TIME_SET:
	case CS_TYPE::EVT_DIMMER_POWERED:
	case CS_TYPE::CMD_DIMMING_ALLOWED:
	case CS_TYPE::CMD_SWITCHING_ALLOWED:
	case CS_TYPE::EVT_STATE_EXTERNAL_STONE:
	case CS_TYPE::EVT_STATE_FACTORY_RESET_DONE:
	case CS_TYPE::EVT_STORAGE_INITIALIZED:
	case CS_TYPE::EVT_STORAGE_WRITE_DONE:
	case CS_TYPE::EVT_STORAGE_REMOVE_DONE:
	case CS_TYPE::EVT_STORAGE_REMOVE_ALL_TYPES_WITH_ID_DONE:
	case CS_TYPE::EVT_STORAGE_GC_DONE:
	case CS_TYPE::EVT_STORAGE_FACTORY_RESET_DONE:
	case CS_TYPE::EVT_STORAGE_PAGES_ERASED:
	case CS_TYPE::EVT_MESH_FACTORY_RESET_DONE:
	case CS_TYPE::CMD_STORAGE_GARBAGE_COLLECT:
	case CS_TYPE::EVT_SETUP_DONE:
	case CS_TYPE::EVT_ADC_RESTARTED:
	case CS_TYPE::CMD_ENABLE_LOG_POWER:
	case CS_TYPE::CMD_ENABLE_LOG_CURRENT:
	case CS_TYPE::CMD_ENABLE_LOG_VOLTAGE:
	case CS_TYPE::CMD_ENABLE_LOG_FILTERED_CURRENT:
	case CS_TYPE::CMD_RESET_DELAYED:
	case CS_TYPE::CMD_ENABLE_ADVERTISEMENT:
	case CS_TYPE::CMD_ENABLE_MESH:
	case CS_TYPE::CMD_TOGGLE_ADC_VOLTAGE_VDD_REFERENCE_PIN:
	case CS_TYPE::CMD_ENABLE_ADC_DIFFERENTIAL_CURRENT:
	case CS_TYPE::CMD_ENABLE_ADC_DIFFERENTIAL_VOLTAGE:
	case CS_TYPE::CMD_INC_VOLTAGE_RANGE:
	case CS_TYPE::CMD_DEC_VOLTAGE_RANGE:
	case CS_TYPE::CMD_INC_CURRENT_RANGE:
	case CS_TYPE::CMD_DEC_CURRENT_RANGE:
	case CS_TYPE::CMD_CONTROL_CMD:
	case CS_TYPE::CMD_SEND_MESH_MSG:
	case CS_TYPE::CMD_SEND_MESH_MSG_MULTI_SWITCH:
	case CS_TYPE::CMD_SEND_MESH_MSG_PROFILE_LOCATION:
	case CS_TYPE::CMD_SEND_MESH_MSG_SET_BEHAVIOUR_SETTINGS:
	case CS_TYPE::CMD_SET_TIME:
	case CS_TYPE::CMD_FACTORY_RESET:
	case CS_TYPE::CMD_ADD_BEHAVIOUR:
	case CS_TYPE::CMD_REPLACE_BEHAVIOUR:
	case CS_TYPE::CMD_REMOVE_BEHAVIOUR:
	case CS_TYPE::CMD_GET_BEHAVIOUR:
	case CS_TYPE::CMD_GET_BEHAVIOUR_INDICES:
	case CS_TYPE::CMD_GET_BEHAVIOUR_DEBUG:
	case CS_TYPE::CMD_CLEAR_ALL_BEHAVIOUR:
	case CS_TYPE::EVT_BEHAVIOURSTORE_MUTATION:
	case CS_TYPE::EVT_BEHAVIOUR_OVERRIDDEN:
	case CS_TYPE::CMD_REGISTER_TRACKED_DEVICE:
	case CS_TYPE::CMD_UPDATE_TRACKED_DEVICE:
	case CS_TYPE::CMD_TRACKED_DEVICE_HEARTBEAT:
	case CS_TYPE::EVT_PRESENCE_MUTATION:
	case CS_TYPE::EVT_PRESENCE_CHANGE:
	case CS_TYPE::CMD_GET_PRESENCE:
	case CS_TYPE::CMD_SET_RELAY:
	case CS_TYPE::CMD_SET_DIMMER:
	case CS_TYPE::EVT_GOING_TO_DFU:
	case CS_TYPE::EVT_RECEIVED_PROFILE_LOCATION:
	case CS_TYPE::EVT_TRACKABLE:
	case CS_TYPE::CMD_UPLOAD_FILTER:
	case CS_TYPE::CMD_REMOVE_FILTER:
	case CS_TYPE::CMD_COMMIT_FILTER_CHANGES:
	case CS_TYPE::CMD_GET_FILTER_SUMMARIES:
	case CS_TYPE::EVT_MESH_SYNC_REQUEST_OUTGOING:
	case CS_TYPE::EVT_MESH_SYNC_REQUEST_INCOMING:
	case CS_TYPE::EVT_MESH_SYNC_FAILED:
	case CS_TYPE::EVT_MESH_PAGES_ERASED:
	case CS_TYPE::EVT_MESH_EXT_STATE_0:
	case CS_TYPE::EVT_MESH_EXT_STATE_1:
	case CS_TYPE::CMD_SEND_MESH_MSG_SET_TIME:
	case CS_TYPE::CMD_SET_IBEACON_CONFIG_ID:
	case CS_TYPE::CMD_SEND_MESH_MSG_NOOP:
	case CS_TYPE::EVT_MESH_RSSI_PING:
	case CS_TYPE::EVT_MESH_RSSI_DATA:
	case CS_TYPE::EVT_MESH_NEAREST_WITNESS_REPORT:
	case CS_TYPE::EVT_MESH_TIME_SYNC:
	case CS_TYPE::EVT_RECV_MESH_MSG:
	case CS_TYPE::CMD_GET_ADC_RESTARTS:
	case CS_TYPE::CMD_GET_SWITCH_HISTORY:
	case CS_TYPE::CMD_GET_POWER_SAMPLES:
	case CS_TYPE::CMD_GET_SCHEDULER_MIN_FREE:
	case CS_TYPE::CMD_GET_RESET_REASON:
	case CS_TYPE::CMD_GET_GPREGRET:
	case CS_TYPE::CMD_GET_ADC_CHANNEL_SWAPS:
	case CS_TYPE::CMD_GET_RAM_STATS:
	case CS_TYPE::EVT_GENERIC_TEST:
	case CS_TYPE::CMD_TEST_SET_TIME:
	case CS_TYPE::CMD_MICROAPP_GET_INFO:
	case CS_TYPE::CMD_MICROAPP_UPLOAD:
	case CS_TYPE::CMD_MICROAPP_VALIDATE:
	case CS_TYPE::CMD_MICROAPP_REMOVE:
	case CS_TYPE::CMD_MICROAPP_ENABLE:
	case CS_TYPE::CMD_MICROAPP_DISABLE:
	case CS_TYPE::EVT_MICROAPP_UPLOAD_RESULT:
	case CS_TYPE::EVT_MICROAPP_ERASE_RESULT:
	case CS_TYPE::CMD_MICROAPP_ADVERTISE:
	case CS_TYPE::EVT_HUB_DATA_REPLY:
	case CS_TYPE::EVT_TWI_INIT:
	case CS_TYPE::EVT_TWI_WRITE:
	case CS_TYPE::EVT_TWI_READ:
	case CS_TYPE::EVT_TWI_UPDATE:
	case CS_TYPE::EVT_GPIO_INIT:
	case CS_TYPE::EVT_GPIO_WRITE:
	case CS_TYPE::EVT_GPIO_READ:
	case CS_TYPE::EVT_GPIO_UPDATE:
		return PersistenceMode::NEITHER_RAM_NOR_FLASH;
	}
	// should not reach this
	return PersistenceMode::NEITHER_RAM_NOR_FLASH;
}

PersistenceModeGet toPersistenceModeGet(uint8_t mode) {
	PersistenceModeGet persistenceMode = static_cast<PersistenceModeGet>(mode);
	switch (persistenceMode) {
		case PersistenceModeGet::CURRENT:
		case PersistenceModeGet::STORED:
		case PersistenceModeGet::FIRMWARE_DEFAULT:
		case PersistenceModeGet::UNKNOWN:
			return persistenceMode;
	}
	return PersistenceModeGet::UNKNOWN;
}

PersistenceModeSet toPersistenceModeSet(uint8_t mode) {
	PersistenceModeSet persistenceMode = static_cast<PersistenceModeSet>(mode);
	switch (persistenceMode) {
		case PersistenceModeSet::TEMPORARY:
		case PersistenceModeSet::STORED:
		case PersistenceModeSet::UNKNOWN:
			return persistenceMode;
	}
	return PersistenceModeSet::UNKNOWN;
}
