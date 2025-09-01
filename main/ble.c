
#include "defs.h"
#include "tag.h"
#define TAG "ble"
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

const int64_t startTime = 0;

typedef struct {
	uint8_t flags[3];
	uint8_t length;
	uint8_t type;
	uint16_t company_id;
	uint16_t beacon_type;
	} __attribute__((packed)) esp_ble_ibeacon_head_t;

typedef struct {
	uint8_t proximity_uuid[16];
	uint16_t major;
	uint16_t minor;
	int8_t measured_power;
	} __attribute__((packed)) esp_ble_ibeacon_vendor_t;


typedef struct {
	esp_ble_ibeacon_head_t ibeacon_head;
	esp_ble_ibeacon_vendor_t ibeacon_vendor;
	} __attribute__((packed)) esp_ble_ibeacon_t;


/* For iBeacon packet format, please refer to Apple "Proximity Beacon Specification" doc */
/* Constant part of iBeacon data */
static esp_ble_ibeacon_head_t ibeacon_common_head;

bool esp_ble_is_ibeacon_packet (uint8_t *adv_data, uint8_t adv_data_len);

esp_err_t esp_ble_config_ibeacon_data (esp_ble_ibeacon_vendor_t *vendor_config, esp_ble_ibeacon_t *ibeacon_adv_data);




bool esp_ble_is_ibeacon_packet (uint8_t *adv_data, uint8_t adv_data_len) {
	bool result = true;

	if ((adv_data != NULL) && (adv_data_len == 0x1E)) {
		if (!memcmp(adv_data, (uint8_t*)&ibeacon_common_head, sizeof(ibeacon_common_head))) {
			result = true;
			}
		}

	return result;
	}



static void init_nvs(void) {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
		}
	ESP_ERROR_CHECK(ret);
	}

static void init_ble(void) {
	esp_err_t ret;

	// Initialize the Bluetooth controller
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(TAG, "Failed to initialize BT controller: %s", esp_err_to_name(ret));
		return;
		}

	// Enable the Bluetooth controller
	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
		return;
		}

	// Initialize Bluedroid
	ret = esp_bluedroid_init();
	if (ret) {
		ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
		return;
		}

	// Enable Bluedroid
	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
		return;
		}
	}

static esp_ble_scan_params_t ble_scan_params = {
	.scan_type = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE,
	.scan_interval = 0x50,
	.scan_window = 0x30,
	};

typedef esp_ble_adv_data_type bleEnum;

static void get_device_name(esp_ble_gap_cb_param_t *param, char *name, int name_len) {
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;

	// Extract the device name from the advertisement data
	adv_name = esp_ble_resolve_adv_data(param->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);

	if (adv_name != NULL && adv_name_len > 0) {
		// Copy the device name to the output buffer
		snprintf(name, name_len, "%.*s", adv_name_len, adv_name);
		}
	else {
		// If no name is found, show "Unknown device"
		snprintf(name, name_len, "Unknown device");
		}
	}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
	switch (event) {
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			if (param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS) {
				ESP_LOGI(TAG, "Scan parameters set, starting scan...");
				//esp_ble_gap_start_scanning(10);  // Scan for 10 seconds
				}
			else {
				ESP_LOGE(TAG, "Failed to set scan parameters");
				}
			break;

		case ESP_GAP_BLE_SCAN_RESULT_EVT:
			if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
				// Get the device name
				char device_name[64];
				get_device_name(param, device_name, sizeof(device_name));

				// Log the device name and RSSI
				//ESP_LOGI(TAG, "Device found: Name: %s, RSSI %d", device_name, param->scan_rst.rssi);
				//if (esp_ble_is_ibeacon_packet(param->scan_rst.ble_adv,param->scan_rst.adv_data_len))
					{
					esp_ble_ibeacon_t *ibeacon_data = (esp_ble_ibeacon_t*)(param->scan_rst.ble_adv);
					//	ESP_LOGI(TAG, "----------iBeacon Found----------");
					///ESP_LOGI(TAG, "Device address: "ESP_BD_ADDR_STR"", ESP_BD_ADDR_HEX(param->scan_rst.bda));
					uint16_t major = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.major);
					uint16_t minor = ENDIAN_CHANGE_U16(ibeacon_data->ibeacon_vendor.minor);

					if(major == 0  || minor == 0 ) {
						break;
						}
					ESP_LOG_BUFFER_HEX("\n\n\nUUID", ibeacon_data->ibeacon_vendor.proximity_uuid,
					                   ESP_UUID_LEN_128);
					printf("\n");
					//printf("\tUUID: %s\n", (char*)ibeacon_data->ibeacon_vendor.proximity_uuid);
					printf("\tMajor: 0x%04x (%d)\n", major, major);
					printf("\tMinor: 0x%04x (%d)\n", minor, minor);
					printf("\tMeasured power (RSSI at a 1m distance): %d dBm\n", ibeacon_data->ibeacon_vendor.measured_power);
					printf("\tRSSI of packet: %d dbm\n", param->scan_rst.rssi);
					time_t now = 0;
					time(&now);
					now+=startTime;
					struct tm   ttm;
					ttm = * localtime(&now);
					printf("%jd seconds since the epoch began\n", (intmax_t)now);
					//const int year =
					//	const int h = now%3600;
					//	const int min = (now - h*3600)/60;
					//	const int sec = now%60;
					char timeS[40];
					sprintf(timeS, "%d:%d:%d %d. %d. %d.", ttm.tm_hour+1,ttm.tm_min,ttm.tm_sec
					        ,ttm.tm_mday, ttm.tm_mon+1, ttm.tm_year%100);
					printf("Time: %s\n", timeS);
					if(isUuidRegister(ibeacon_data->ibeacon_vendor.proximity_uuid)){
						addTag(timeS, (int)major, (int)minor, param->scan_rst.rssi, 
									 ibeacon_data->ibeacon_vendor.measured_power, 
									 ibeacon_data->ibeacon_vendor.proximity_uuid);
						/*Moved to addTag
						if(justSend && (!applayKalman) && 
						  (countSubsripcitionEvents == NUM_OF_TOPICS && 
						   GET_STATE(MQTT))){
							Tag tempTag;
							//memset(tempTag, 0 , sizeof(Tag));
							strcpy(tempTag.time, timeS);
							tempTag.majorID = major;
							tempTag.minorID = minor;
							memcpy(tempTag.proximity_uuid, 
									ibeacon_data->ibeacon_vendor.proximity_uuid,
									sizeof(uint8_t)*16);
									
							tempTag.rssi = param->scan_rst.rssi;
							tempTag.refpower = ibeacon_data->ibeacon_vendor.measured_power;
							//tempTag.systemTime = esp_timer_get_time();
							tempTag.isStanding = 0;
							//PUBLISH_TAG(tempTag, ADD_TAG);
							//TBD ADD A WAY TO BE SEND NOT FROM HEAR
							xQueueSend(xQueueTagStreaming, &tempTag, 1);
						}
						//*/
					}
				}
				}
			break;

		case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
			ESP_LOGI(TAG, "Scan complete");
			break;

		default:
			break;
		}
	}


extern void bleTaskScane(void* parms) {
	vTaskDelay(2000);
	while(1) {
		esp_ble_gap_start_scanning(1000);
		vTaskDelay(2000);
		esp_ble_gap_stop_scanning();
		}

	}
