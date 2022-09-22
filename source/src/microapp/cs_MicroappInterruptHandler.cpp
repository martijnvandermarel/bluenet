/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Aug 29, 2022
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <ble/cs_Stack.h>
#include <events/cs_Event.h>
#include <events/cs_EventDispatcher.h>
#include <microapp/cs_MicroappController.h>
#include <microapp/cs_MicroappInterruptHandler.h>
#include <microapp/cs_MicroappSdkUtil.h>

/*
 * Enable throttling of BLE scanned devices by rssi.
 */
#define DEVELOPER_OPTION_THROTTLE_BY_RSSI 1

MicroappInterruptHandler::MicroappInterruptHandler() : EventListener() {
	EventDispatcher::getInstance().addListener(this);
}

void MicroappInterruptHandler::handleEvent(event_t& event) {
	switch (event.type) {
		case CS_TYPE::EVT_GPIO_UPDATE: {
			auto data = CS_TYPE_CAST(EVT_GPIO_UPDATE, event.data);
			onGpioUpdate(*data);
			break;
		}
		case CS_TYPE::EVT_DEVICE_SCANNED: {
			auto data = CS_TYPE_CAST(EVT_DEVICE_SCANNED, event.data);
			onDeviceScanned(*data);
			break;
		}
		case CS_TYPE::EVT_RECV_MESH_MSG: {
			auto data = CS_TYPE_CAST(EVT_RECV_MESH_MSG, event.data);
			onReceivedMeshMessage(*data);
			break;
		}

		// Central
		case CS_TYPE::EVT_BLE_CENTRAL_CONNECT_RESULT: {
			auto data = CS_TYPE_CAST(EVT_BLE_CENTRAL_CONNECT_RESULT, event.data);
			onBleCentralConnectResult(*data);
			break;
		}
		case CS_TYPE::EVT_BLE_CENTRAL_DISCONNECTED: {
			onBleCentralDisconnected();
			break;
		}
		case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY: {
			auto data = CS_TYPE_CAST(EVT_BLE_CENTRAL_DISCOVERY, event.data);
			onBleCentralDiscovery(*data);
			break;
		}
		case CS_TYPE::EVT_BLE_CENTRAL_DISCOVERY_RESULT: {
			auto data = CS_TYPE_CAST(EVT_BLE_CENTRAL_DISCOVERY_RESULT, event.data);
			onBleCentralDiscoveryResult(*data);
			break;
		}
		case CS_TYPE::EVT_BLE_CENTRAL_WRITE_RESULT: {
			auto data = CS_TYPE_CAST(EVT_BLE_CENTRAL_WRITE_RESULT, event.data);
			onBleCentralWriteResult(*data);
			break;
		}
		case CS_TYPE::EVT_BLE_CENTRAL_READ_RESULT: {
			auto data = CS_TYPE_CAST(EVT_BLE_CENTRAL_READ_RESULT, event.data);
			onBleCentralReadResult(*data);
			break;
		}

		// Peripheral
		case CS_TYPE::EVT_BLE_CONNECT: {
			auto data = CS_TYPE_CAST(EVT_BLE_CONNECT, event.data);
			onBlePeripheralConnect(*data);
			break;
		}
		case CS_TYPE::EVT_BLE_DISCONNECT: {
			auto data = CS_TYPE_CAST(EVT_BLE_DISCONNECT, event.data);
			onBlePeripheralDisconnect(*data);
			break;
		}
		default: {
			break;
		}
	}
}

uint8_t* MicroappInterruptHandler::getOutputBuffer(MicroappSdkMessageType type, uint8_t id) {
	if (!MicroappController::getInstance().allowSoftInterrupts()) {
		LogMicroappInterruptDebug("New interrupts blocked, ignore event");
		return nullptr;
	}
	if (!MicroappController::getInstance().isSoftInterruptRegistered(type, id)) {
		LogMicroappInterruptDebug("No interrupt registered");
		return nullptr;
	}
	return MicroappController::getInstance().getOutputMicroappBuffer();
}

/*
 * Do some checks to validate if we want to generate an interrupt for the pin event
 * and if so, prepare the outgoing buffer and call generateSoftInterrupt()
 */
void MicroappInterruptHandler::onGpioUpdate(cs_gpio_update_t& event) {
	LogMicroappInterruptDebug("onGpioUpdate");
	uint8_t interruptPin  = MicroappSdkUtil::digitalPinToInterrupt(event.pinIndex);

	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_PIN, interruptPin);
	if (outputBuffer == nullptr) {
		LogMicroappInterrupDebug("No output buffer");
		return;
	}

	microapp_sdk_pin_t* pin = reinterpret_cast<microapp_sdk_pin_t*>(outputBuffer);
	pin->header.messageType = CS_MICROAPP_SDK_TYPE_PIN;
	pin->pin                = interruptPin;

	LogMicroappInterruptDebug("Incoming GPIO interrupt for microapp on virtual pin %i", interruptPin);
	MicroappController::getInstance().generateSoftInterrupt();
}

/*
 * Do some checks to validate if we want to generate an interrupt for the scanned device
 * and if so, prepare the outgoing buffer and call generateSoftInterrupt()
 */
void MicroappInterruptHandler::onDeviceScanned(scanned_device_t& dev) {
	if (!MicroappController::getInstance().microappData.isScanning) {
		return;
	}

#ifdef DEVELOPER_OPTION_THROTTLE_BY_RSSI
	// Throttle by rssi by default to limit number of scanned devices forwarded
	if (dev.rssi < -50) {
		return;
	}
#endif
	LogMicroappInterruptDebug("onDeviceScanned");

	// Write bluetooth device to buffer
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_SCAN);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	if (dev.dataSize > sizeof(ble->scan.eventScan.data)) {
		LOGw("BLE advertisement data too large");
		return;
	}

	ble->header.messageType          = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                        = CS_MICROAPP_SDK_BLE_SCAN;
	ble->scan.type                   = CS_MICROAPP_SDK_BLE_SCAN_EVENT_SCAN;
	ble->scan.eventScan.address.type = dev.addressType;
	std::copy(dev.address, dev.address + MAC_ADDRESS_LENGTH, ble->scan.eventScan.address.address);
	ble->scan.eventScan.rssi = dev.rssi;
	ble->scan.eventScan.size = dev.dataSize;
	memcpy(ble->scan.eventScan.data, dev.data, dev.dataSize);

	LogMicroappInterruptDebug("Incoming BLE scanned device for microapp");
	MicroappController::getInstance().generateSoftInterrupt();
}

/*
 * Do some checks to validate if we want to generate an interrupt for the received message
 * and if so, prepare the outgoing buffer and call generateSoftInterrupt()
 */
void MicroappInterruptHandler::onReceivedMeshMessage(MeshMsgEvent& event) {
	if (event.type != CS_MESH_MODEL_TYPE_MICROAPP) {
		// Mesh message received, but not for the microapp.
		return;
	}
	LogMicroappInterruptDebug("onReceivedMeshMessage");

	if (event.isReply) {
		// We received the empty reply.
		LogMicroappInterruptDebug("Reply received");
		return;
	}
	if (event.reply != nullptr) {
		// Send an empty reply.
		event.reply->type     = CS_MESH_MODEL_TYPE_MICROAPP;
		event.reply->dataSize = 0;
	}

	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_MESH, CS_MICROAPP_SDK_MESH_READ);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_mesh_t* meshMsg = reinterpret_cast<microapp_sdk_mesh_t*>(outputBuffer);
	meshMsg->header.messageType  = CS_MICROAPP_SDK_TYPE_MESH;
	meshMsg->type                = CS_MICROAPP_SDK_MESH_READ;
	meshMsg->stoneId             = event.srcStoneId;
	meshMsg->size                = event.msg.len;
	memcpy(meshMsg->data, event.msg.data, event.msg.len);

	LogMicroappInterruptDebug("Incoming mesh message for microapp");
	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralConnectResult(cs_ret_code_t& retCode) {
	LogMicroappInterruptDebug("onBleCentralConnectResult");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble          = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType          = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                        = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type                = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_CONNECT;
	ble->central.connectionHandle    = Stack::getInstance().getConnectionHandle();
	ble->central.eventConnect.result = MicroappSdkUtil::bluenetResultToMicroapp(retCode);

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralDisconnected() {
	LogMicroappInterruptDebug("onBleCentralDisconnected");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble       = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType       = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                     = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type             = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCONNECT;
	ble->central.connectionHandle = Stack::getInstance().getConnectionHandle();

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralDiscovery(ble_central_discovery_t& event) {
	LogMicroappInterruptDebug("onBleCentralDiscovery");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble                = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType                = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                              = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type                      = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCOVER;
	ble->central.connectionHandle          = Stack::getInstance().getConnectionHandle();
	ble->central.eventDiscover.uuid        = MicroappSdkUtil::convertUuid(event.uuid);
	ble->central.eventDiscover.valueHandle = event.valueHandle;
	ble->central.eventDiscover.cccdHandle  = event.cccdHandle;

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralDiscoveryResult(cs_ret_code_t& retCode) {
	LogMicroappInterruptDebug("onBleCentralDiscoveryResult");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble               = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType               = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                             = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type                     = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_DISCOVER_DONE;
	ble->central.connectionHandle         = Stack::getInstance().getConnectionHandle();
	ble->central.eventDiscoverDone.result = MicroappSdkUtil::bluenetResultToMicroapp(retCode);

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralWriteResult(cs_ret_code_t& retCode) {
	LogMicroappInterruptDebug("onBleCentralWriteResult");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble             = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType             = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                           = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type                   = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_WRITE;
	ble->central.connectionHandle       = Stack::getInstance().getConnectionHandle();  // TODO: get handle from event.
	ble->central.eventWrite.valueHandle = 0;                                           // TODO: get handle from event.
	ble->central.eventWrite.result      = MicroappSdkUtil::bluenetResultToMicroapp(retCode);

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBleCentralReadResult(ble_central_read_result_t& event) {
	LogMicroappInterruptDebug("onBleCentralReadResult");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_CENTRAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble            = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType            = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                          = CS_MICROAPP_SDK_BLE_CENTRAL;
	ble->central.type                  = CS_MICROAPP_SDK_BLE_CENTRAL_EVENT_READ;
	ble->central.connectionHandle      = Stack::getInstance().getConnectionHandle();  // TODO: get handle from event.
	ble->central.eventRead.valueHandle = 0;                                           // TODO: get handle from event.
	ble->central.eventRead.result      = MicroappSdkUtil::bluenetResultToMicroapp(event.retCode);

	// TODO: check size
	ble->central.eventRead.size        = event.data.len;
	memcpy(ble->central.eventRead.data, event.data.data, event.data.len);

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBlePeripheralConnect(ble_connected_t& event) {
	LogMicroappInterruptDebug("onBlePeripheralConnect");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_PERIPHERAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble                   = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType                   = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                                 = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	ble->peripheral.type                      = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_CONNECT;
	ble->peripheral.connectionHandle          = event.connectionHandle;

	// TODO: get mac address
	ble->peripheral.eventConnect.address.type = MICROAPP_SDK_BLE_ADDRESS_PUBLIC;
	memset(ble->peripheral.eventConnect.address.address, 0, sizeof(ble->peripheral.eventConnect.address));

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBlePeripheralDisconnect(uint16_t connectionHandle) {
	LogMicroappInterruptDebug("onBlePeripheralConnect");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_PERIPHERAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble          = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType          = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                        = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	ble->peripheral.type             = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_DISCONNECT;
	ble->peripheral.connectionHandle = connectionHandle;

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBlePeripheralWrite(
		uint16_t connectionHandle, uint16_t characteristicHandle, cs_data_t value) {
	LogMicroappInterruptDebug("onBlePeripheralWrite");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_PERIPHERAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble          = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType          = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                        = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	ble->peripheral.type             = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_WRITE;
	ble->peripheral.connectionHandle = connectionHandle;
	ble->peripheral.handle           = characteristicHandle;
	ble->peripheral.eventWrite.size  = value.len;

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBlePeripheralSubscription(
		uint16_t connectionHandle, uint16_t characteristicHandle, bool subscribed) {
	LogMicroappInterruptDebug("onBlePeripheralSubscription subscribed=%u", subscribed);
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_PERIPHERAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type               = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	if (subscribed) {
		ble->peripheral.type = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_SUBSCRIBE;
	}
	else {
		ble->peripheral.type = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_UNSUBSCRIBE;
	}
	ble->peripheral.connectionHandle = connectionHandle;
	ble->peripheral.handle           = characteristicHandle;

	MicroappController::getInstance().generateSoftInterrupt();
}

void MicroappInterruptHandler::onBlePeripheralNotififyDone(uint16_t connectionHandle, uint16_t characteristicHandle) {
	LogMicroappInterruptDebug("onBlePeripheralNotififyDone");
	uint8_t* outputBuffer = getOutputBuffer(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_PERIPHERAL);
	if (outputBuffer == nullptr) {
		return;
	}

	microapp_sdk_ble_t* ble          = reinterpret_cast<microapp_sdk_ble_t*>(outputBuffer);
	ble->header.messageType          = CS_MICROAPP_SDK_TYPE_BLE;
	ble->type                        = CS_MICROAPP_SDK_BLE_PERIPHERAL;
	ble->peripheral.type             = CS_MICROAPP_SDK_BLE_PERIPHERAL_EVENT_NOTIFICATION_DONE;
	ble->peripheral.connectionHandle = connectionHandle;
	ble->peripheral.handle           = characteristicHandle;

	MicroappController::getInstance().generateSoftInterrupt();
}
