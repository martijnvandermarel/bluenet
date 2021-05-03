/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Nov 29, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */
#pragma once

#include <events/cs_EventListener.h>
#include <protocol/cs_TrackableParserPackets.h>
#include <structs/cs_TrackableParserStructs.h>

#include <optional>

/**
 * Transforms EVT_DEVICE_SCANNED and EVT_ADV_BACKGROUND_PARSED
 * into EVT_TRACKING_UPDATE events.
 *
 * Responsible for throttling input to the localisation module using filter parsers.
 */
class TrackableParser : public EventListener {
public:
	/**
	 * set filterModificationInProgress to false.
	 */
	void init();
	void handleEvent(event_t& evt);

	constexpr static size_t MAX_FILTER_IDS     = 8;
	constexpr static size_t FILTER_BUFFER_SIZE = 512;

private:
	// -------------------------------------------------------------
	// --------------------- Filter buffer data --------------------
	// -------------------------------------------------------------

	/**
	 * List of pointers to the currently allocated filters.
	 * The filters in this array are always sorted by filterId,
	 * and nullptrs only occur at the back in the sense that:
	 *   for any i <= j < MAX_FILTER_IDS:
	 *   if   _parsingFilters[i] == nullptr
	 *   then _parsingFilters[j] == nullptr
	 */
	tracking_filter_t* _parsingFilters[MAX_FILTER_IDS] = {};

	/**
	 * Number of allocated filters in the parsingFilters array.
	 * Shall suffice: 0 <=  _parsingFiltersCount < MAX_FILTER_IDS
	 */
	uint8_t _parsingFiltersCount = 0;

	/**
	 * Hash of all the hashes of the allocated filters, sorted by filterId.
	 * This is updated by the commit command if it matches.
	 */
	uint16_t _masterHash;

	/**
	 * Keeps track of the version of the filters -- not in use yet.
	 */
	uint16_t _masterVersion;

	/**
	 * When this value is true:
	 * - no incoming advertisements are parsed.
	 * - filters may be in inconsistent state.
	 *
	 * Defaults to true, so that the system has time to load data from flash.
	 */
	bool filterModificationInProgress = true;

	// -------------------------------------------------------------
	// ------------------ Advertisment processing ------------------
	// -------------------------------------------------------------

	/**
	 * Dispatches a TrackedEvent for the given advertisement.
	 */
	void handleScannedDevice(scanned_device_t* device);

	/**
	 * Not in use yet
	 */
	void handleBackgroundParsed(adv_background_parsed_t* trackableAdvertisement);

	// -------------------------------------------------------------
	// ------------------- Filter data management ------------------
	// -------------------------------------------------------------

	/**
	 * Heap allocates a tracking_filter_t object, if there is enough free
	 * space on the heap and if the total allocated size of this TrackableParser
	 * component used for filters will not exceed the maximum as result of this
	 * allocation.
	 *
	 * Initializes the .runtimedata of the newly allocated tracking_filter_t
	 * with the given size, filterid and crc == 0.
	 *
	 * Returns nullptr on failure.
	 *
	 * Internally adjusts _filterBufferEnd to point to one byte after this array.
	 */
	tracking_filter_t* allocateParsingFilter(uint8_t filterId, size_t payloadSize);

	/**
	 * Same as deallocateParsingFilterByIndex, but looks up the filter by the filterId.
	 *
	 * Returns true when id is found and filter is deallocated, false else.
	 */
	bool deallocateParsingFilter(uint8_t filterId);

	/**
	 * Deallocates the filter at given index in the _parsingFilters array.
	 *
	 * If a gap is created in the array, this method moves all filters
	 * with an index above the given one down one index to close this gap.
	 */
	void deallocateParsingFilterByIndex(uint8_t parsingFilterIndex);

	/**
	 * Looks up given filter id in the list of filters. Returns nullptr if not found.
	 *
	 * Assumes there are no 'gaps' in the _parsingFilter array. I.e. that
	 * if _parsingFilters[i] == nullptr then _parsingFilters[j] == nullptr
	 * for all j>=i.
	 */
	tracking_filter_t* findParsingFilter(uint8_t filterId);

	/**
	 * Returns the index of the parsing filter with given filterId,
	 * or an empty optional if that wasn't available.
	 */
	std::optional<size_t> findParsingFilterIndex(uint8_t filterId);

	/**
	 * Returns the sums of all getTotalSize of the non-nullptrs in the _parsingFilters array.
	 */
	size_t getTotalHeapAllocatedSize();

	/**
	 * Computes and returns the totalSize allocated for the given filter.
	 */
	size_t getTotalSize(tracking_filter_t& trackingFilter);

	// -------------------------------------------------------------
	// ---------------------- Command interface --------------------
	// -------------------------------------------------------------

	/**
	 * Upon first reception of this command with the given filterId,
	 * allocate space in the buffer. If this fails, abort. Else set the filterId to
	 * 'upload in progress'.
	 */
	cs_ret_code_t handleUploadFilterCommand(trackable_parser_cmd_upload_filter_t* cmdData);

	/**
	 * Removes given filter immediately.
	 * Flags this crownstone as 'filter modification in progress'.
	 */
	cs_ret_code_t handleRemoveFilterCommand(trackable_parser_cmd_remove_filter_t* cmdData);

	/**
	 * Inactive filters are activated.
	 * Crcs are (re)computed.
	 * This crownstones master version and crc are broadcasted over the mesh.
	 * Sets 'filter modification in progress' flag of this crownstone back to off.
	 */
	cs_ret_code_t handleCommitFilterChangesCommand(trackable_parser_cmd_commit_filter_changes_t* cmdData);

	/**
	 * Writes summaries of the filters into the result as a
	 * trackable_parser_cmd_get_filter_summaries_ret_t.
	 *
	 * returnCode is ERR_BUFFER_TOO_SMALL if result data doesn't fit the result buffer.
	 * Else, returnCode is ERR_SUCCESS.
	 */
	void handleGetFilterSummariesCommand(trackable_parser_cmd_get_filter_summaries_t* cmdData, cs_result_t& result);

	// -------------------------------------------------------------
	// ---------------------- Utility functions --------------------
	// -------------------------------------------------------------

	/**
	 * sets _masterVersion to 0 and filterModificationInProgress to true.
	 *
	 * This will result in the TrackableParser not handling incoming advertisements
	 * and ensure that it is safe to adjust the filters.
	 */
	void startProgress();

	/**
	 * The master crc is the crc16 of the filters in the buffer.
	 * This method assumes the filter crcs are up to date.
	 */
	uint16_t masterCrc();

	/**
	 * Checks if the cuckoo filter buffer size plus the constant overhead
	 * equals the total size allocated for the tracking_filter*s in the
	 * _parsingFilter array.
	 *
	 * Deallocates any filters failing the check.
	 *
	 * Check is only performed on filters that are currently in progress (.crc == 0)
	 */
	bool checkFilterSizeConsistency();

	/**
	 * Computes the crcs of filters that currently have their crc set to 0.
	 */
	void computeCrcs();

	// -------------------------------------------------------------
	// ----------------------- OLD interface -----------------------
	// -------------------------------------------------------------

	// TODO(#177858707) remove this.
	/**
	 *  Checks service uuids of the scanned device and returns true
	 *  if we can find an official 16 bit Tile service uuid.
	 *
	 *  Caveats:
	 *  - Tiles are not iBeacons.
	 *  - expects the services to be listed in the type
	 *    BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE.
	 */
	bool isTileDevice(scanned_device_t* scannedDevice);

	void logServiceData(scanned_device_t* scannedDevice);
};
