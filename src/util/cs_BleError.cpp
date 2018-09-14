/**
 * Author: Crownstone Team
 * Date: 21 Sep., 2013
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <util/cs_BleError.h>
#include <drivers/cs_Serial.h>

//! Called by BluetoothLE.h classes when exceptions are disabled.
void ble_error_handler (const char * msg, uint32_t line_num, const char * p_file_name) {
	volatile const char* message __attribute__((unused)) = msg;
	volatile uint16_t line __attribute__((unused)) = line_num;
	volatile const char* file __attribute__((unused)) = p_file_name;

	LOGf("FATAL ERROR %s, at %s:%d", message, file, line);

	__asm("BKPT");
	while(1) {}
}

//! called by soft device when you pass bad parameters, etc.
void app_error_handler (uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
	volatile uint32_t error __attribute__((unused)) = error_code;
	volatile uint16_t line __attribute__((unused)) = line_num;
	volatile const uint8_t* file __attribute__((unused)) = p_file_name;

	const char * str_error = NordicTypeName(error);
	LOGe("%s", str_error);
	LOGf("FATAL ERROR 0x%x, at %s:%d", error, file, line);

	__asm("BKPT");
	while(1) {}
}

//! Called by softdevice
void app_error_handler_bare(ret_code_t error_code)
{
	volatile uint32_t error __attribute__((unused)) = error_code;
	volatile uint16_t line __attribute__((unused)) = 0;
	volatile const uint8_t* file __attribute__((unused)) = NULL;

	const char * str_error = NordicTypeName(error);
	LOGe("%s", str_error);
	LOGf("FATAL ERROR 0x%x", error);

	__asm("BKPT");
	while(1) {}
}

//called by NRF SDK when it has an internal error.
void assert_nrf_callback (uint16_t line_num, const uint8_t *file_name) {
	volatile uint16_t line __attribute__((unused)) = line_num;
	volatile const uint8_t* file __attribute__((unused)) = file_name;

	LOGf("FATAL ERROR at %s:%d", file, line);

	__asm("BKPT");
	while(1) {}
}

/*

//called by soft device when it has an internal error.
void softdevice_assertion_handler(uint32_t pc, uint16_t line_num, const uint8_t * file_name){
	volatile uint16_t line __attribute__((unused)) = line_num;
	volatile const uint8_t* file __attribute__((unused)) = file_name;
	__asm("BKPT");
	while(1) {}
}

*/
