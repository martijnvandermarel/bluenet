/**
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: 14 Aug., 2014
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

/**********************************************************************************************************************
 *
 * The Crownstone is a high-voltage (domestic) switch. It can be used for:
 *   - indoor localization
 *   - building automation
 *
 * It is one of the first, or the first(?), open-source Internet-of-Things devices entering the market.
 *
 * Read more on: https://crownstone.rocks
 *
 * Almost all configuration options should be set in CMakeBuild.config.
 *
 *********************************************************************************************************************/

#include <ble/cs_CrownstoneManufacturer.h>
#include <cfg/cs_Boards.h>
#include <cfg/cs_HardwareVersions.h>
#include <cs_Crownstone.h>
#include <common/cs_Types.h>
#include <drivers/cs_PWM.h>
#include <drivers/cs_RNG.h>
#include <drivers/cs_RTC.h>
#include <drivers/cs_Temperature.h>
#include <drivers/cs_Timer.h>
#include <events/cs_EventDispatcher.h>
#include <processing/cs_EncryptionHandler.h>
#include <protocol/cs_UartProtocol.h>
#include <storage/cs_State.h>
#include <structs/buffer/cs_MasterBuffer.h>
#include <structs/buffer/cs_EncryptionBuffer.h>
#include <util/cs_Utils.h>

extern "C" {
#include <nrf_nvmc.h>
}

// Define test pin to enable gpio debug.
#define TEST_PIN 18

// Define to enable leds. WARNING: this is stored in UICR and not easily reversible!
//#define ENABLE_LEDS


/**********************************************************************************************************************
 * Main functionality
 *********************************************************************************************************************/

void handleZeroCrossing() {
	PWM::getInstance().onZeroCrossing();
}

Crownstone::Crownstone(boards_config_t& board) :
	_boardsConfig(board),
	_switch(NULL), _temperatureGuard(NULL), _powerSampler(NULL), _watchdog(NULL), _enOceanHandler(NULL),
	_deviceInformationService(NULL), _crownstoneService(NULL), _setupService(NULL),
	_serviceData(NULL), _beacon(NULL),
#if BUILD_MESHING == 1
	_mesh(NULL),
#endif
	_commandHandler(NULL), _scanner(NULL), _tracker(NULL), _scheduler(NULL), _factoryReset(NULL),
	_mainTimerId(NULL),
	_operationMode(OperationMode::OPERATION_MODE_SETUP)
{
	_mainTimerData = { {0} };
	_mainTimerId = &_mainTimerData;

	MasterBuffer::getInstance().alloc(MASTER_BUFFER_SIZE);
	EncryptionBuffer::getInstance().alloc(BLE_GATTS_VAR_ATTR_LEN_MAX);

	EventDispatcher::getInstance().addListener(this);

	//! set up the bluetooth stack that controls the hardware.
	_stack = &Stack::getInstance();

	// create all the objects that are needed for execution, but make sure they
	// don't execute any softdevice related code. do that in an init function
	// and call it in the setup/configure phase
	_timer = &Timer::getInstance();

	// persistent storage, configuration, state
	_storage = &Storage::getInstance();
	_state = &State::getInstance();

	// create command handler
	_commandHandler = &CommandHandler::getInstance();
	_factoryReset = &FactoryReset::getInstance();

	// create instances for the scanner and mesh
	// actual initialization is done in their respective init methods
	_scanner = &Scanner::getInstance();
#if CHAR_TRACK_DEVICES == 1
	_tracker = &Tracker::getInstance();
#endif

#if BUILD_MESHING == 1
	_mesh = &Mesh::getInstance();
#endif

	if (IS_CROWNSTONE(_boardsConfig.deviceType)) {
		// switch using PWM or Relay
		_switch = &Switch::getInstance();
		// create temperature guard
		_temperatureGuard = &TemperatureGuard::getInstance();
 
		_powerSampler = &PowerSampling::getInstance();

		_watchdog = &Watchdog::getInstance();

		_enOceanHandler = &EnOceanHandler::getInstance();
	}

};

void Crownstone::init() {
	//! initialize drivers
	LOGi(FMT_HEADER, "init");
	initDrivers();
	LOG_MEMORY;

	LOGi(FMT_HEADER, "mode");
	uint8_t mode;
	_state->get(CS_TYPE::STATE_OPERATION_MODE, &mode, PersistenceMode::STRATEGY1);
	_operationMode = static_cast<OperationMode>(mode);
	switch(_operationMode) {
		case OperationMode::OPERATION_MODE_SETUP: 
		case OperationMode::OPERATION_MODE_NORMAL: 
		case OperationMode::OPERATION_MODE_FACTORY_RESET: 
		case OperationMode::OPERATION_MODE_DFU: 
			break;
		default:
			LOGd("Huh? Set default mode to setup");
			_operationMode = OperationMode::OPERATION_MODE_SETUP;
			_state->set(CS_TYPE::STATE_OPERATION_MODE, &mode, sizeof(mode), PersistenceMode::STRATEGY1);
	}
	LOGd("Operation mode: %s", TypeName(_operationMode));
	
	//! configure the crownstone
	LOGi(FMT_HEADER, "configure");
	configure();

	LOGi(FMT_CREATE, "timer");
	_timer->createSingleShot(_mainTimerId, (app_timer_timeout_handler_t)Crownstone::staticTick);

	switch(_operationMode) {
		case OperationMode::OPERATION_MODE_SETUP: {
			if (serial_get_state() == SERIAL_ENABLE_NONE) {
				serial_enable(SERIAL_ENABLE_RX_ONLY);
			}

			LOGd("Configure setup mode");

			// create services
			createSetupServices();

			// loop through all services added to the stack and create the characteristics
			_stack->createCharacteristics();

			// set it by default into low tx mode
			_stack->changeToLowTxPowerMode();

			// Because iPhones cannot programmatically clear their cache of paired devices, the phone that
			// did the setup is at risk of not being able to connect to the crownstone if the cs clears the device
			// manager. We use our own encryption scheme to counteract this.
			if (_state->isSet(CS_TYPE::CONFIG_ENCRYPTION_ENABLED)) {
				LOGi(FMT_ENABLE, "AES encryption");
				_stack->setAesEncrypted(true);
			}
			//		LOGi(FMT_ENABLE, "PIN encryption");
			//		// use PIN encryption for setup mode
			//		_stack->setPinEncrypted(true);

			//		if (_boardsConfig.deviceType == DEVICE_CROWNSTONE_BUILTIN) {
			//			_switch->delayedSwitch(SWITCH_ON, SWITCH_ON_AT_SETUP_BOOT_DELAY);
			//		}

			break;
		}
		case OperationMode::OPERATION_MODE_NORMAL: {

			LOGd("Configure normal operation mode");

			// setup normal operation mode
			prepareNormalOperationMode();

			// create services
			createCrownstoneServices();

			// loop through all services added to the stack and create the characteristics
			_stack->createCharacteristics();

			// use aes encryption for normal mode (if enabled)
			if (_state->isSet(CS_TYPE::CONFIG_ENCRYPTION_ENABLED)) {
				LOGi(FMT_ENABLE, "AES encryption");
				_stack->setAesEncrypted(true);
			}

			break;
		}
		case OperationMode::OPERATION_MODE_FACTORY_RESET: {

			LOGd("Configure factory reset mode");
			FactoryReset::getInstance().finishFactoryReset(_boardsConfig.deviceType);
			break;
		}
		case OperationMode::OPERATION_MODE_DFU: {

			CommandHandler::getInstance().handleCommand(CMD_GOTO_DFU);
			break;
		}
	}

	LOGi(FMT_HEADER, "init services");

	_stack->initServices();

	// [16.06.16] need to execute app scheduler, otherwise pstorage
	// events will get lost ... maybe need to check why that actually happens??
	app_sched_execute();
}

/**
 * Configurate the Bluetooth stack. This also increments the reset counter.
 */
void Crownstone::configure() {

	LOGi("> stack ...");
	// configure parameters for the Bluetooth stack
	configureStack();

	Storage::getInstance().garbageCollect();

	st_value_t resetCounter;
	State::getInstance().get(CS_TYPE::STATE_RESET_COUNTER, &resetCounter, PersistenceMode::STRATEGY1);
	resetCounter.u32++;
	LOGw("Reset counter at: %d", resetCounter.u8);
	State::getInstance().set(CS_TYPE::STATE_RESET_COUNTER, &resetCounter, sizeof(resetCounter), PersistenceMode::STRATEGY1);

	// set advertising parameters such as the device name and appearance.
	// Note: has to be called after _stack->init or Storage is initialized too early and won't work correctly
	setName();

	writeDefaults();

	LOGi("> advertisement ...");
	// configure advertising parameters
#if EDDYSTONE==1
	_eddystone = new Eddystone();
	_eddystone->advertising_init();
#else
	configureAdvertisement();
#endif

}

/**
 * This must be called after the SoftDevice has started. The order in which things should be initialized is as follows:
 *   1. Stack.               Starts up the softdevice. It controls a lot of devices, so need to set it early.
 *   2. Timer.
 *   3. Storage.             Definitely after the stack has been initialized.
 *   4. State.               Storage should be initialized here.
 */
void Crownstone::initDrivers() {
	_stack->init();
	_timer->init();
	_storage->init();
	_state->init(&_boardsConfig);

//#define ANNE_OH_NO
#ifdef ANNE_OH_NO
	 _state->factoryReset(FACTORY_RESET_CODE);
#endif

	// If not done already, init UART
	// TODO: make into a class with proper init() function
	if (!_boardsConfig.flags.hasSerial) {
		serial_config(_boardsConfig.pinGpioRx, _boardsConfig.pinGpioTx);
		uint8_t uartEnabled;
		_state->get(CS_TYPE::CONFIG_UART_ENABLED, &uartEnabled, PersistenceMode::STRATEGY1);
		serial_enable((serial_enable_t)uartEnabled);
	}

	LOGi(FMT_INIT, "command handler");
	_commandHandler->init(&_boardsConfig);

	LOGi(FMT_INIT, "factory reset");
	_factoryReset->init();

	LOGi(FMT_INIT, "encryption handler");
	EncryptionHandler::getInstance().init();

	if (IS_CROWNSTONE(_boardsConfig.deviceType)) {
		// switch / PWM init
		LOGi(FMT_INIT, "switch / PWM");
		_switch->init(_boardsConfig);

		LOGi(FMT_INIT, "temperature guard");
		_temperatureGuard->init(_boardsConfig);

		LOGi(FMT_INIT, "power sampler");
		_powerSampler->init(_boardsConfig);

		LOGi(FMT_INIT, "watchdog");
		_watchdog->init();

		//LOGi(FMT_INIT, "enocean");
		//_enOceanHandler->init();
	}

	// init GPIOs
	if (_boardsConfig.flags.hasLed) {
		LOGi("Configure LEDs");
		// Note: DO NOT USE THEM WHILE SCANNING OR MESHING
		nrf_gpio_cfg_output(_boardsConfig.pinLedRed);
		nrf_gpio_cfg_output(_boardsConfig.pinLedGreen);
		// Turn the leds off
		if (_boardsConfig.flags.ledInverted) {
			nrf_gpio_pin_set(_boardsConfig.pinLedRed);
			nrf_gpio_pin_set(_boardsConfig.pinLedGreen);
		}
		else {
			nrf_gpio_pin_clear(_boardsConfig.pinLedRed);
			nrf_gpio_pin_clear(_boardsConfig.pinLedGreen);
		}
	}
}

/** Sets default parameters of the Bluetooth connection.
 *
 * Data is transmitted with TX_POWER dBm.
 *
 * On transmission of data within a connection (higher interval -> lower power consumption, slow communication)
 *   - minimum connection interval (in steps of 1.25 ms, 16*1.25 = 20 ms)
 *   - maximum connection interval (in steps of 1.25 ms, 32*1.25 = 40 ms)
 * The supervision timeout multiplier is 400
 * The slave latency is 10
 * On advertising:
 *   - advertising interval (in steps of 0.625 ms, 1600*0.625 = 1 sec) (can be between 0x0020 and 0x4000)
 *   - advertising timeout (disabled, can be between 0x0001 and 0x3FFF, and is in steps of seconds)
 *
 * There is no whitelist defined, nor peer addresses.
 *
 * Process:
 *   [31.05.16] we used to stop / start scanning after a disconnect, now starting advertising is enough
 *   [23.06.16] restart the mesh on disconnect, otherwise we have ~10s delay until the device starts advertising.
 *   [29.06.16] restart the mesh disabled, this was limited to pca10000, it does crash dobeacon v0.7
 */
void Crownstone::configureStack() {
	// Set the stored tx power
	int8_t txPower;
	_state->get(CS_TYPE::CONFIG_TX_POWER, &txPower, PersistenceMode::STRATEGY1);
	_stack->setTxPowerLevel(txPower);

	// Set the stored advertisement interval
	uint16_t advInterval;
	_state->get(CS_TYPE::CONFIG_ADV_INTERVAL, &advInterval, PersistenceMode::STRATEGY1);
	_stack->setAdvertisingInterval(advInterval);

	// Retrieve and Set the PASSKEY for pairing
	uint8_t passkey[BLE_GAP_PASSKEY_LEN];
	_state->get(CS_TYPE::CONFIG_PASSKEY, passkey, PersistenceMode::STRATEGY1);

	// TODO: If key is not set, this leads to a crash
//	_stack->setPasskey(passkey);

	_stack->onConnect([&](uint16_t conn_handle) {
		LOGi("onConnect...");
		// TODO: see https://devzone.nordicsemi.com/index.php/about-rssi-of-ble
		// be neater about it... we do not need to stop, only after a disconnect we do...
#if RSSI_ENABLE==1
		sd_ble_gap_rssi_stop(conn_handle);
		sd_ble_gap_rssi_start(conn_handle, 0, 0);
#endif
		uint32_t gpregret_id = 0;
		uint32_t gpregret_msk = 0xFF;
		sd_power_gpregret_clr(gpregret_id, gpregret_msk);

		_stack->setNonConnectable();
	//	_stack->restartAdvertising();

	});

	_stack->onDisconnect([&](uint16_t conn_handle) {
		LOGi("onDisconnect...");
		if (_operationMode == OperationMode::OPERATION_MODE_SETUP) {
			_stack->changeToLowTxPowerMode();
		}

		_state->disableNotifications();

		_stack->setConnectable();
		_stack->restartAdvertising();
	});
}

void Crownstone::configureAdvertisement() {

	// Initialize service data
	uint8_t mode;
	State::getInstance().get(CS_TYPE::STATE_OPERATION_MODE, &mode, PersistenceMode::STRATEGY1);
	OperationMode _tmpOperationMode = static_cast<OperationMode>(mode);
	LOGd("Operation mode: %s", TypeName(_tmpOperationMode));

	// Create the iBeacon parameter object which will be used to configure the advertisement as an iBeacon.
	uint16_t major, minor;
	TYPIFY(CONFIG_IBEACON_TXPOWER) rssi;
	ble_uuid128_t uuid;
	_state->get(CS_TYPE::CONFIG_IBEACON_MAJOR, &major, PersistenceMode::STRATEGY1);
	_state->get(CS_TYPE::CONFIG_IBEACON_MINOR, &minor, PersistenceMode::STRATEGY1);
	_state->get(CS_TYPE::CONFIG_IBEACON_UUID, uuid.uuid128, PersistenceMode::STRATEGY1);
	_state->get(CS_TYPE::CONFIG_IBEACON_TXPOWER, &rssi, PersistenceMode::STRATEGY1);
	LOGd("iBeacon: major=%i, minor=%i, rssi_on_1m=%i", major, minor, (int8_t)rssi);

	_beacon = new IBeacon(uuid, major, minor, rssi);

	// Create the ServiceData object which will be (mis)used to advertise select state variables from the Crownstone.
	_serviceData = new ServiceData();
	_serviceData->setDeviceType(_boardsConfig.deviceType);
	_serviceData->init();

	// The service data is populated with State information, but only in NORMAL mode.
	if (_tmpOperationMode == OperationMode::OPERATION_MODE_NORMAL) {
		LOGd("Normal mode, fill with state info");
		
		// Write crownstone id to the service data object.
		uint16_t crownstoneId;
		_state->get(CS_TYPE::CONFIG_CROWNSTONE_ID, &crownstoneId, PersistenceMode::STRATEGY1);
		_serviceData->updateCrownstoneId(crownstoneId);
		LOGi("Set crownstone id to %u", crownstoneId);

		// Write switch state to the service data object.
		uint8_t switchState;
		_state->get(CS_TYPE::STATE_SWITCH_STATE, &switchState, PersistenceMode::STRATEGY1);
		_serviceData->updateSwitchState(switchState);

		// Write temperature to the service data object.
		_serviceData->updateTemperature(getTemperature());
	}
	
	LOGd("Init radio");

	// assign service data to stack
	_stack->setServiceData(_serviceData);

	// Need to init radio before configuring ibeacon?
	_stack->initRadio();

	if (_state->isSet(CS_TYPE::CONFIG_IBEACON_ENABLED)) {
		LOGd("Configure iBeacon");
		_stack->configureIBeacon(_beacon, _boardsConfig.deviceType);
	} else {
		LOGd("Configure BLE device");
		_stack->configureBleDevice(_boardsConfig.deviceType);
	}

}

void Crownstone::createSetupServices() {
	LOGi(STR_CREATE_ALL_SERVICES);

	_deviceInformationService = new DeviceInformationService();
	_stack->addService(_deviceInformationService);

	_setupService = new SetupService();
	_stack->addService(_setupService);

}

void Crownstone::createCrownstoneServices() {
	LOGi(STR_CREATE_ALL_SERVICES);

	_deviceInformationService = new DeviceInformationService();
	_stack->addService(_deviceInformationService);

	_crownstoneService = new CrownstoneService();
	_stack->addService(_crownstoneService);
}

/**
 * The default name. This can later be altered by the user if the corresponding service and characteristic is enabled.
 * It is loaded from memory or from the default and written to the Stack.
 */
void Crownstone::setName() {
	static bool addResetCounterToName = false;
#if CHANGE_NAME_ON_RESET==1
	addResetCounterToName = true;
#endif

	char device_name[32];
	size16_t size = 0;
	_state->get(CS_TYPE::CONFIG_NAME, device_name, size, PersistenceMode::STRATEGY1);
	std::string deviceName;
	if (addResetCounterToName) {
		//! clip name to 5 chars and add reset counter at the end
		uint16_t resetCounter;
		size16_t resetCounterSize = sizeof(resetCounter);
		_state->get(CS_TYPE::STATE_RESET_COUNTER, &resetCounter, resetCounterSize, PersistenceMode::STRATEGY1);
		char devicename_resetCounter[32];
		sprintf(devicename_resetCounter, "%.*s_%d", MIN(size, 5), device_name, resetCounter);
		deviceName = std::string(devicename_resetCounter);
	} else {
		deviceName = std::string(device_name, size);
	}
	LOGi(FMT_SET_STR_VAL, "name", deviceName.c_str());
	_stack->updateDeviceName(deviceName); 

}

void Crownstone::prepareNormalOperationMode() {

	//! create scanner object
	_scanner->init();
	_scanner->setStack(_stack);
//	if (_state->isSet(CONFIG_TRACKER_ENABLED)) {
#if CHAR_TRACK_DEVICES == 1
	_tracker->init();
#endif
//	}

	//! create scheduler
	_scheduler = &Scheduler::getInstance();


#if BUILD_MESHING == 1
//	if (_state->isEnabled(CONFIG_MESH_ENABLED)) {
		_mesh->init();
//	}
#endif
}

void Crownstone::writeDefaults() {
	st_value_t value;
	value.u8 = 0;
	_state->set(CS_TYPE::STATE_TEMPERATURE, &value, sizeof(value), PersistenceMode::STRATEGY1);
	_state->set(CS_TYPE::STATE_TIME, &value, sizeof(value), PersistenceMode::STRATEGY1);
	_state->set(CS_TYPE::STATE_ERRORS, &value, sizeof(value), PersistenceMode::STRATEGY1);
}

void Crownstone::startUp() {

	LOGi(FMT_HEADER, "startup");

	uint32_t gpregret_id = 0;
	uint32_t gpregret;
	sd_power_gpregret_get(gpregret_id, &gpregret);
	LOGi("Soft reset count: %d", gpregret);

	uint16_t bootDelay;
	_state->get(CS_TYPE::CONFIG_BOOT_DELAY, &bootDelay, PersistenceMode::STRATEGY1);
	if (bootDelay) {
		LOGi("Boot delay: %d ms", bootDelay);
		nrf_delay_ms(bootDelay);
	}

	//! start advertising
#if EDDYSTONE==1
	_eddystone->advertising_start();
#else
	LOGi("Start advertising");
	_stack->startAdvertising();
#endif
	// Have to give the stack a moment of pause to start advertising, otherwise we get into race conditions.
	// TODO: Is this still the case? Can we solve this differently? 
	nrf_delay_ms(50);

	if (IS_CROWNSTONE(_boardsConfig.deviceType)) {
		//! Start switch, so it can be used.
		_switch->start();

		if (_operationMode == OperationMode::OPERATION_MODE_SETUP && 
				_boardsConfig.deviceType == DEVICE_CROWNSTONE_BUILTIN) {
			_switch->delayedSwitch(SWITCH_ON, SWITCH_ON_AT_SETUP_BOOT_DELAY);
		}

		//! Start temperature guard regardless of operation mode
		LOGi(FMT_START, "temp guard");
		_temperatureGuard->start();

		//! Start power sampler regardless of operation mode (as it is used for the current based soft fuse)
		LOGi(FMT_START, "power sampling");
		_powerSampler->startSampling();
	}

	// Start ticking main and services.
	scheduleNextTick();

	// The rest we only execute if we are in normal operation.
	// During other operation modes, most of the crownstone's functionality is disabled.
	if (_operationMode == OperationMode::OPERATION_MODE_NORMAL) {

		if (IS_CROWNSTONE(_boardsConfig.deviceType)) {
			// Let the power sampler call the PWM callback function on zero crossings.
			_powerSampler->enableZeroCrossingInterrupt(handleZeroCrossing);
		}

		_scheduler->start();

		if (_state->isSet(CS_TYPE::CONFIG_SCANNER_ENABLED)) {
			RNG rng;
			uint16_t delay = rng.getRandom16() / 6; // Delay in ms (about 0-10 seconds)
			_scanner->delayedStart(delay);
		}

//		if (_state->isSet(CONFIG_TRACKER_ENABLED)) {
#if CHAR_TRACK_DEVICES == 1
			_tracker->startTracking();
#endif
//		}

		if (_state->isSet(CS_TYPE::CONFIG_MESH_ENABLED)) {
#if BUILD_MESHING == 1
//			nrf_delay_ms(500);
			//! TODO: start with delay please
			_mesh->start();
#endif
		} else {
			LOGi("Mesh not enabled");
		}

	}

	uint32_t err_code;
	ble_gap_addr_t address;
	err_code = sd_ble_gap_addr_get(&address);
	APP_ERROR_CHECK(err_code);

	_log(SERIAL_INFO, "BLE Address: ");	
	BLEutil::printAddress((uint8_t*)address.addr, BLE_GAP_ADDR_LEN);

#ifdef RELAY_DEFAULT_ON
#if RELAY_DEFAULT_ON==0
	Switch::getInstance().turnOff();
#endif
#if RELAY_DEFAULT_ON==1
	Switch::getInstance().turnOn();
#endif
#endif
}

void Crownstone::tick() {
	st_value_t temperature;
	temperature.s32 = getTemperature();
	_state->set(CS_TYPE::STATE_TEMPERATURE, &temperature, sizeof(temperature), PersistenceMode::STRATEGY1);

#define ADVERTISEMENT_IMPROVEMENT 1
#if ADVERTISEMENT_IMPROVEMENT==1 // TODO: remove this macro
	// Update advertisement parameter (only in operation mode NORMAL)
	if (_operationMode == OperationMode::OPERATION_MODE_NORMAL) {

		// update advertisement parameters (to improve scanning on (some) android phones)
		_stack->updateAdvertisement(true);

		// update advertisement (to update service data)
		_stack->setAdvertisementData();
	}
#endif

	// Check for timeouts
	if (_operationMode == OperationMode::OPERATION_MODE_NORMAL) {
		if (RTC::getCount() > RTC::msToTicks(PWM_BOOT_DELAY_MS)) {
			_switch->startPwm();
		}
	}

	scheduleNextTick();
}

void Crownstone::scheduleNextTick() {
	Timer::getInstance().start(_mainTimerId, HZ_TO_TICKS(CROWNSTONE_UPDATE_FREQUENCY), this);
}

void Crownstone::run() {

	LOGi(FMT_HEADER, "running");

	// Forever, run scheduler, wait for events and handle them
	while(1) {
		app_sched_execute();
		BLE_CALL(sd_app_evt_wait, ());
	}
}

void Crownstone::handleEvent(event_t & event) {

//	LOGd("Event: %s [%i]", TypeName(event.type), +event.type);

	bool reconfigureBeacon = false;
	switch(event.type) {

		case CS_TYPE::CONFIG_NAME: {
			_stack->updateDeviceName(std::string((char*)event.data, event.size));
			_stack->configureScanResponse(_boardsConfig.deviceType);
			_stack->setAdvertisementData();
			break;
		}
		case CS_TYPE::CONFIG_IBEACON_MAJOR: {
			_beacon->setMajor(*(TYPIFY(CONFIG_IBEACON_MAJOR)*)event.data);
			reconfigureBeacon = true;
			break;
		}
		case CS_TYPE::CONFIG_IBEACON_MINOR: {
			_beacon->setMinor(*(TYPIFY(CONFIG_IBEACON_MINOR)*)event.data);
			reconfigureBeacon = true;
			break;
		}
		case CS_TYPE::CONFIG_IBEACON_UUID: {
			_beacon->setUUID(*(ble_uuid128_t*)event.data);
			reconfigureBeacon = true;
			break;
		}
		case CS_TYPE::CONFIG_IBEACON_TXPOWER: {
			_beacon->setTxPower(*(TYPIFY(CONFIG_IBEACON_TXPOWER)*)event.data);
			reconfigureBeacon = true;
			break;
		}
		case CS_TYPE::CONFIG_TX_POWER: {
			_stack->setTxPowerLevel(*(TYPIFY(CONFIG_TX_POWER)*)event.data);
			break;
		}
		case CS_TYPE::CONFIG_ADV_INTERVAL: {
			_stack->updateAdvertisingInterval(*(TYPIFY(CONFIG_ADV_INTERVAL)*)event.data, true);
			break;
		}
		case CS_TYPE::CONFIG_PASSKEY: {
			_stack->setPasskey((uint8_t*)event.data);
			break;
		}
		case CS_TYPE::EVT_ENABLE_ADVERTISEMENT: {
			TYPIFY(EVT_ENABLE_ADVERTISEMENT) enable = *(TYPIFY(EVT_ENABLE_ADVERTISEMENT)*)event.data;
			if (enable) {
				_stack->startAdvertising();
			}
			else {
				_stack->stopAdvertising();
			}
			UartProtocol::getInstance().writeMsg(UART_OPCODE_TX_ADVERTISEMENT_ENABLED, &enable, 1);
			break;
		}
		case CS_TYPE::EVT_ENABLE_MESH: {
#if BUILD_MESHING == 1
			uint8_t enable = *(uint8_t*)event.data;
			if (enable) {
				_mesh->start();
			}
			else {
				_mesh->stop();
			}
			UartProtocol::getInstance().writeMsg(UART_OPCODE_TX_MESH_ENABLED, &enable, 1);
#endif
			break;
		}
		case CS_TYPE::CONFIG_IBEACON_ENABLED: {
			TYPIFY(CONFIG_IBEACON_ENABLED) enabled = *(TYPIFY(CONFIG_IBEACON_ENABLED)*)event.data;
			if (enabled) {
				_stack->configureIBeaconAdvData(_beacon);
			} else {
				_stack->configureBleDeviceAdvData();
			}
			_stack->setAdvertisementData();
			break;
		}
		case CS_TYPE::EVT_ADVERTISEMENT_UPDATED: {
			_stack->setAdvertisementData();
			break;
		}
		case CS_TYPE::EVT_BROWNOUT_IMPENDING: {
			// turn everything off that consumes power
			LOGf("brownout impending!! force shutdown ...")

#if BUILD_MESHING == 1
				rbc_mesh_stop();
#endif
			_scanner->stop();

			if (IS_CROWNSTONE(_boardsConfig.deviceType)) {
				// [11-jul-2017] Since we store the switch state, we don't have to switch here.
				//			_switch->setSwitch(0);
				_powerSampler->stopSampling();
			}

			uint32_t gpregret_id = 0;
			uint32_t gpregret_msk = GPREGRET_BROWNOUT_RESET;
			// now reset with brownout reset mask set.
			// NOTE: do not clear the gpregret register, this way
			//   we can count the number of brownouts in the bootloader
			sd_power_gpregret_set(gpregret_id, gpregret_msk);
			// soft reset, because brownout can't be distinguished from
			// hard reset otherwise
			sd_nvic_SystemReset();
			break;
		}
		case CS_TYPE::EVT_CMD_RESET: {
			CommandHandler::getInstance().resetDelayed(GPREGRET_SOFT_RESET);
			break;
		}
		default: return;
	}

	if (reconfigureBeacon && _state->isSet(CS_TYPE::CONFIG_IBEACON_ENABLED)) {
		_stack->setAdvertisementData();
	}
}

void on_exit(void) {
	LOGf("PROGRAM TERMINATED");
}

/**
 * If UART is enabled this will be the message printed out over a serial connection. In release mode we will not by
 * default use the UART, it will need to be turned on.
 *
 * For DFU, application should be at (BOOTLOADER_REGION_START - APPLICATION_START_CODE - DFU_APP_DATA_RESERVED). For
 * example, for (0x38000 - 0x1C000 - 0x400) this is 0x1BC00 (113664 bytes).
 */
void welcome(uint8_t pinRx, uint8_t pinTx) {
	serial_config(pinRx, pinTx);
	serial_init(SERIAL_ENABLE_RX_AND_TX);
	_log(SERIAL_INFO, SERIAL_CRLF);

#ifdef GIT_HASH
#undef FIRMWARE_VERSION
#define FIRMWARE_VERSION GIT_HASH
#endif

	size16_t size = sizeof(STRINGIFY(FIRMWARE_VERSION));
	char version[sizeof(STRINGIFY(FIRMWARE_VERSION))];
	memcpy(version, STRINGIFY(FIRMWARE_VERSION), size);

	_log(SERIAL_INFO, "Welcome! Bluenet firmware, version %s\r\n\r\n", version);
	_log(SERIAL_INFO, "\033[35;1m");
	_log(SERIAL_INFO, " _|_|_|    _|                                            _|     \r\n");
	_log(SERIAL_INFO, " _|    _|  _|  _|    _|    _|_|    _|_|_|      _|_|    _|_|_|_| \r\n");
	_log(SERIAL_INFO, " _|_|_|    _|  _|    _|  _|_|_|_|  _|    _|  _|_|_|_|    _|     \r\n");
	_log(SERIAL_INFO, " _|    _|  _|  _|    _|  _|        _|    _|  _|          _|     \r\n");
	_log(SERIAL_INFO, " _|_|_|    _|    _|_|_|    _|_|_|  _|    _|    _|_|_|      _|_| \r\n\r\n");
	_log(SERIAL_INFO, "\033[0m");

	LOGi("Compilation date: %s", STRINGIFY(COMPILATION_DAY));
	LOGi("Compilation time: %s", __TIME__);
	LOGi("Hardware version: %s", get_hardware_version());

	LOG_MEMORY;
}

/**********************************************************************************************************************/

/** Overwrite the hardware version.
 *
 * The firmware is compiled with particular defaults. When a particular product comes from the factory line it has
 * by default FFFF FFFF in this UICR location. If this is the case, there are two options to cope with this:
 *   1. Create a custom firmware per device type where this field is adjusted at runtime. 
 *   2. Create a custom firmware per device type with the UICR field state in the .hex file. In the latter case,
 *      if the UICR fields are already set, this might lead to a conflict.
 * There is chosen for the first option. Even if rare cases where there are devices types with FFFF FFFF in the field, 
 * the runtime always tries to overwrite it with the (let's hope) proper state.
 */
void overwrite_hardware_version() {
	uint32_t hardwareBoard = NRF_UICR->CUSTOMER[UICR_BOARD_INDEX];
	if (hardwareBoard == 0xFFFFFFFF) {
		LOGw("Write board type into UICR");
		nrf_nvmc_write_word(HARDWARE_BOARD_ADDRESS, DEFAULT_HARDWARE_BOARD);
	}
	LOGd("Board: %p", hardwareBoard);
}

extern "C" {

#define CROWNSTONE_SOC_OBSERVER_PRIO 1
static void crownstone_soc_evt_handler(uint32_t evt_id, void * p_context) {
	LOGd("SOC event: %i", evt_id);
}

#define CROWNSTONE_STATE_OBSERVER_PRIO 1
static void crownstone_state_handler(nrf_sdh_state_evt_t state, void * p_context) {
	switch (state) {
		case NRF_SDH_EVT_STATE_ENABLE_PREPARE:
			LOGd("Softdevice is about to be enabled");
			break;
		case NRF_SDH_EVT_STATE_ENABLED:
			LOGd("Softdevice is now enabled");
			break;
		case NRF_SDH_EVT_STATE_DISABLE_PREPARE:
			LOGd("Softdevice is about to be disabled");
			break;
		case NRF_SDH_EVT_STATE_DISABLED:
			LOGd("Softdevice is now disabled");
			break;
		default:
			LOGd("Unknown state change");
	}
}

NRF_SDH_SOC_OBSERVER(m_crownstone_soc_observer, CROWNSTONE_SOC_OBSERVER_PRIO, crownstone_soc_evt_handler, NULL);

NRF_SDH_STATE_OBSERVER(m_crownstone_state_handler, CROWNSTONE_STATE_OBSERVER_PRIO) =
{ 
	.handler = crownstone_state_handler,
	.p_context = NULL
};

}	

/**********************************************************************************************************************
 * The main function. Note that this is not the first function called! For starters, if there is a bootloader present,
 * the code within the bootloader has been processed before. But also after the bootloader, the code in
 * cs_sysNrf51.c will set event handlers and other stuff (such as coping with product anomalies, PAN), before calling
 * main. If you enable a new peripheral device, make sure you enable the corresponding event handler there as well.
 *********************************************************************************************************************/

int main() {
#ifdef TEST_PIN
	nrf_gpio_cfg_output(TEST_PIN);
	nrf_gpio_pin_clear(TEST_PIN);
#endif

	// this enabled the hard float, without it, we get a hardfault
	SCB->CPACR |= (3UL << 20) | (3UL << 22); __DSB(); __ISB();

	atexit(on_exit);

	uint32_t errCode;
	boards_config_t board = {};
	errCode = configure_board(&board);
	APP_ERROR_CHECK(errCode);

	// Init gpio pins early in the process!
	if (IS_CROWNSTONE(board.deviceType)) {
		nrf_gpio_cfg_output(board.pinGpioPwm);
		if (board.flags.pwmInverted) {
			nrf_gpio_pin_set(board.pinGpioPwm);
		} else {
			nrf_gpio_pin_clear(board.pinGpioPwm);
		}
		//! Relay pins
		if (board.flags.hasRelay) {
			nrf_gpio_cfg_output(board.pinGpioRelayOff);
			nrf_gpio_pin_clear(board.pinGpioRelayOff);
			nrf_gpio_cfg_output(board.pinGpioRelayOn);
			nrf_gpio_pin_clear(board.pinGpioRelayOn);
		}
	}

#ifdef TEST_PIN1
	nrf_gpio_pin_toggle(TEST_PIN);
#endif
	if (board.flags.hasSerial) {
		// init uart, be nice and say hello
		welcome(board.pinGpioRx, board.pinGpioTx);
	}

#ifdef TEST_PIN1
	nrf_gpio_pin_toggle(TEST_PIN);
#endif
	Crownstone crownstone(board); // 250 ms

	// initialize crownstone (depends on the operation mode) ...
#ifdef TEST_PIN1
	nrf_gpio_pin_toggle(TEST_PIN);
#endif

#ifdef ENABLE_LEDS
	// WARNING: this is stored in UICR and not easily reversible!
	if (NRF_UICR->NFCPINS != 0) {
		LOGw("enable gpio LEDs");
		nrf_nvmc_write_word((uint32_t)&(NRF_UICR->NFCPINS), 0);
	}
#endif
	LOGd("NFC pins: %p", NRF_UICR->NFCPINS);

	overwrite_hardware_version();

	// init drivers, configure(), create services and chars,
	crownstone.init(); // 13 ms

	//! start up phase, start ticking (depends on the operation mode) ...
#ifdef TEST_PIN1
	nrf_gpio_pin_toggle(TEST_PIN);
#endif
	crownstone.startUp(); // 500 ms

	//! run forever ...
#ifdef TEST_PIN1
	nrf_gpio_pin_toggle(TEST_PIN);
#endif
	crownstone.run();

	return 0;
}
