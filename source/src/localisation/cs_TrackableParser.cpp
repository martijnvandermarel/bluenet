/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Nov 29, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <ble/cs_BleConstants.h>
#include <common/cs_Types.h>
#include <localisation/cs_Nearestnearestwitnessreport.h>
#include <localisation/cs_TrackableEvent.h>
#include <localisation/cs_TrackableParser.h>
#include <logging/cs_Logger.h>
#include <structs/cs_PacketsInternal.h>
#include <structs/cs_StreamBufferAccessor.h>
#include <util/cs_Utils.h>


// REVIEW: This won't be recognized by binary logger.
#define LOGTrackableParserDebug LOGnone

void TrackableParser::init() {
}

void TrackableParser::handleEvent(event_t& evt) {
	switch(evt.type) {
		// incoming devices to filter
		case CS_TYPE::EVT_ADV_BACKGROUND_PARSED: {
			adv_background_parsed_t *parsedAdv = CS_TYPE_CAST(EVT_ADV_BACKGROUND_PARSED, evt.data);
			handleBackgroundParsed(parsedAdv);
			return;
		}
		case CS_TYPE::EVT_DEVICE_SCANNED: {
			scanned_device_t* scannedDevice = CS_TYPE_CAST(EVT_DEVICE_SCANNED, evt.data);
			handleAsTileDevice(scannedDevice);
			// add other trackable device types here

			return;
		}

		// incoming filter commands
		case CS_TYPE::CMD_UPLOAD_FILTER: {
			LOGd("CMD_UPLOAD_FILTER");
			return;

			trackable_parser_cmd_upload_filter_t* cmd_data =
					CS_TYPE_CAST(CMD_UPLOAD_FILTER, evt.data);


			handleUploadFilterCommand(cmd_data);
			break;
		}
		case CS_TYPE::CMD_REMOVE_FILTER: {
			LOGd("CMD_REMOVE_FILTER");
			return;

			trackable_parser_cmd_remove_filter_t* cmd_data =
					CS_TYPE_CAST(CMD_REMOVE_FILTER, evt.data);

			handleRemoveFilterCommand(cmd_data);
			break;
		}
		case CS_TYPE::CMD_COMMIT_FILTER_CHANGES: {
			LOGd("CMD_COMMIT_FILTER_CHANGES");
			return;

			trackable_parser_cmd_commit_filter_changes_t* cmd_data =
					CS_TYPE_CAST(CMD_COMMIT_FILTER_CHANGES, evt.data);
			handleCommitFilterChangesCommand(cmd_data);
			break;
		}
		case CS_TYPE::CMD_GET_FILTER_SUMMARIES: {
			LOGd("CMD_GET_FILTER_SUMMARIES");
			return;

			trackable_parser_cmd_get_filer_summaries_t* cmd_data =
					CS_TYPE_CAST(CMD_GET_FILTER_SUMMARIES, evt.data);
			handleGetFilterSummariesCommand(cmd_data);
			break;
		}
		default:
			break;

	}
}


void TrackableParser::handleBackgroundParsed(adv_background_parsed_t *trackableAdv) {
	// TODO: implement when we have a good representation of trackables in the mesh.
	//
	//	TrackableEvent trackEvent;
	//	trackEvent.id = TrackableId(trackableAdv->macAddress);
	//	trackEvent.rssi = trackableAdv->adjustedRssi;
	//
	//	trackEvent.dispatch();
}

// -------------------------------------------------------------
// ------------------ Internal filter management ---------------
// -------------------------------------------------------------

TrackableParser::ParsingFilter* TrackableParser::allocateParsingFilter(uint8_t filterId, size_t size) {
	if(_filterBufferEndIndex + size > FILTER_BUFFER_SIZE) {
		// not enough space for filter of this total size.
		return nullptr;
	}

	_parsingFilters[_parsingFiltersEndIndex] = reinterpret_cast<ParsingFilter*>(_filterBuffer + _filterBufferEndIndex);
	_filterBufferEndIndex += size;

	// don't forget to postcrement the EndIndex for the filter list in return statement.
	return _parsingFilters[_parsingFiltersEndIndex++];
}

TrackableParser::ParsingFilter* TrackableParser::findParsingFilter(uint8_t filterId) {
	ParsingFilter* parsingFilter;
	for (size_t index = 0; index < _parsingFiltersEndIndex; ++index) {
		parsingFilter = _parsingFilters[index];

		if(parsingFilter == nullptr) {
			LOGw("_parsingFiltersEndIndex incorrect: found nullptr before reaching end of filter list.");
			return nullptr;
		}

		if (parsingFilter->runtimedata.filterId == filterId) {
			return _parsingFilters[index];
		}
	}

	return nullptr;
}

void TrackableParser::deallocateParsingFilter(uint8_t filterId) {
	// TODO(Arend);
	// find filter pointer in _parsingFiltser
	// wipe memory in buffer at that location
	// memcpy the tail onto the created opening
	// remove filterId from _parsingFilters list
}


// -------------------------------------------------------------
// ---------------------- Command interface --------------------
// -------------------------------------------------------------

bool TrackableParser::handleUploadFilterCommand(
		trackable_parser_cmd_upload_filter_t* cmd_data) {

	// find or allocate a parsing filter
	ParsingFilter* parsingFilter = findParsingFilter(cmd_data->filterId);
	if(parsingFilter == nullptr) {
		parsingFilter = allocateParsingFilter(cmd_data->filterId, cmd_data->totalSize);

		if(parsingFilter == nullptr) {
			// failed to handle command, no space.
			return false;
		}

		// initialize runtime data.
		parsingFilter->runtimedata.filterId = cmd_data->filterId;
		parsingFilter->runtimedata.crc = 0;

		// meta data and filter data will be memcpy'd from chunks,
		// no need to copy those.
	}

	// WARNING(Arend): we're not doing corruption checks here yet.
	// E.g.: when totalSize changes for a specific filterId before
	//       a commit is reached, we have a parsingFilter allocated
	//       but it's of the wrong size.
	// Edit(Arend):
	// 		As we keep a list of pointers _parsingFilters, as well
	// 		as the end pointer to the byte array, this information
	// 		is implicitly available even if multiple filters are
	// 		uploaded in chunks concurrently. Just ptrdiff them.

	// apply filter chunk, counting chunk index from metadata onwards:
	std::memcpy (&(parsingFilter->metadata) + cmd_data->chunkStartIndex, cmd_data->chunk, cmd_data->chunkSize);

	return true;
}

void TrackableParser::handleRemoveFilterCommand(trackable_parser_cmd_remove_filter_t* cmd_data) {
	// TODO(Arend): implement later.
}

void TrackableParser::handleCommitFilterChangesCommand(trackable_parser_cmd_commit_filter_changes_t* cmd_data) {
	// TODO(Arend): implement later.
	// - compute and check all filter sizes
	// - compute and check all filter crcs
	// - compute and check(?) master crc
	// - persist all filters
	// - unset in progress flag (async?)
	// - broadcast update to the mesh
}

void TrackableParser::handleGetFilterSummariesCommand(trackable_parser_cmd_get_filer_summaries_t* cmd_data) {
	// TODO(Arend): implement later.
}


// -------------------------------------------------------------
// ----------------------- OLD interface -----------------------
// -------------------------------------------------------------




// ====================== Mac Filter =====================

bool TrackableParser::isMyTrackable(scanned_device_t* scannedDevice) {
	// Note: mac address here as read in nrf connect app, hence the std::reverse call.
	uint8_t myTileMac[] = {0xe4, 0x96, 0x62, 0x0d, 0x5a, 0x5b};
	std::reverse(std::begin(myTileMac), std::end(myTileMac));
	TrackableId myTrackable(myTileMac);

	// construct TrackableId for incomming scan
	TrackableId mac(scannedDevice->address);

	return mac == myTrackable;
}

// ======================== Tile ========================

bool TrackableParser::isTileDevice(scanned_device_t* scannedDevice) {
	uint32_t errCode;
	cs_data_t serviceUuid16List;

	errCode = BLEutil::findAdvType(
			BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
			scannedDevice->data,
			scannedDevice->dataSize,
			&serviceUuid16List);

	if (errCode != ERR_SUCCESS) {
		return false;
	}

	uint16_t* uuidList = reinterpret_cast<uint16_t*>(serviceUuid16List.data);

	BleServiceUuid tileServiceUuids[] = {
			BleServiceUuid::TileX,
			BleServiceUuid::TileY,
			BleServiceUuid::TileZ };

	for (auto i = 0u; i < serviceUuid16List.len; i++) {
		for (auto j = 0u; j < std::size(tileServiceUuids); j++) {
			if (uuidList[i] == tileServiceUuids[j]) {
				return true;
			}
		}
	}

	return false;
}


bool TrackableParser::handleAsTileDevice(scanned_device_t* scannedDevice) {
	if (!isTileDevice(scannedDevice)) {
		return false;
	}


	TrackableId tile(scannedDevice->address);

	LOGTrackableParserDebug("Tile device: rssi=%i ", scannedDevice->rssi);
	tile.print(" ");

	if (!isMyTrackable(scannedDevice)) {
		// it was a Tile device, so return true.
		return true;
	}

	// logServiceData("Tile device servicedata", scanned_device);

	TrackableEvent trackEvt;
	trackEvt.rssi = scannedDevice->rssi;
	trackEvt.dispatch();

	return true;
}


// ======================== Utils ========================

// REVIEW: string as argument can't be left out from binary size.
void TrackableParser::logServiceData(const char* headerStr, scanned_device_t* scannedDevice) {
	uint32_t errCode;
	cs_data_t serviceData;
	errCode = BLEutil::findAdvType(BLE_GAP_AD_TYPE_SERVICE_DATA,
			scannedDevice->data,
			scannedDevice->dataSize,
			&serviceData);

	if (errCode != ERR_SUCCESS) {
		return;
	}

	// REVIEW: Use the Util function for this.
	_log(SERIAL_INFO, false, "len=%d data=[", serviceData.len);
	for (auto i = 0u; i < serviceData.len; i++) {
		_log(SERIAL_INFO, false, " %2x,", serviceData.data[i]);
	}
	_log(SERIAL_INFO, true, "]");
}

