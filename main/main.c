#include "defs.h"
//#define ETHERNET_IMPLEMENTATNION
//#include "ethernet.h"
#define BLE_INTERNAL
#define WIFI_IMPLEMENTATION
#include "wifi.h"
#include "server.c"
#include "tag.c"
#include "parking.c"
#ifdef BLE_INTERNAL
#include "ble.c"
#endif
#include "mqtt.h"

//#include "fileMqtt.c"
void app_main() {
// ethernet_init();
	//BLOCK_TASK(ETHERNET, 100);
	const uint8_t temp[] = {0xca, 0xfe, 0xf0, 0x6d, 0x4a, 0xff, 0x42, 0x52,
	 0x94, 0x8e, 0x68, 0x56, 0x7a, 0x75, 0xc6, 0x39};
	memcpy(saved_uuids[0], temp, sizeof(temp)*15);
	num_reg_uuid++;
	//strcpy(name_of_beacon, "Nesto");
	const esp_vfs_spiffs_conf_t conf = {
		.base_path = "/storage",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true,
		};

	esp_err_t err = esp_vfs_spiffs_register(&conf);
	if(err != ESP_OK) {
		ESP_LOGE("main", "Failed to init a SPIFFS(%s)", esp_err_to_name(err));
		ESP_ERROR_CHECK(ESP_FAIL);
		}
	size_t total = 0, used = 0;
	esp_spiffs_info(conf.partition_label, &total, &used);
	ESP_LOGI("main", "We have a %d bytes, used %d\n", (int)total, (int)used);
	wifi_init();
	BLOCK_TASK(WIFI, 100);
//LOAD STUF FROM FILE 
FILE *f = fopen("/storage/uuid.bin", "r");
if(f != NULL){
	fread(&num_reg_uuid, sizeof(uint8_t), 1, f);
	fread(saved_uuids, sizeof(saved_uuids), 1, f);
}
else{
	printf("Not exisit\n");
	//DELAY(100000);
}
fclose(f);


#ifdef BLE_INTERNAL
	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, "pool.ntp.org");
	esp_sntp_init();
	sntp_sync_time(tv);
	init_nvs();
	init_ble();
	esp_ble_gap_register_callback(gap_event_handler);
	esp_err_t ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	if (ret == ESP_OK) {
		ESP_LOGI(TAG, "BLE scan parameters set successfully");
		}
	else {
		ESP_LOGE(TAG, "Failed to set scan params: %s", esp_err_to_name(ret));
		}
	xTaskCreate(bleTaskScane, "bleTaskScane", 10000, NULL, 1, NULL);
#endif
	
	server_init();
	xQueueTagAddedToList = xQueueCreate(100, sizeof(int));
	xQueueTagRemovedFromList = xQueueCreate(100, sizeof(Tag));
	xQueueTagStreaming = xQueueCreate(100, sizeof(Tag));
	
	xQueueParking = xQueueCreate(100, sizeof(int));

	xTaskCreate(AddTagTask,    "AddTagTask", 4096, NULL, 1, NULL);
	xTaskCreate(RemoveTagTask, "RemoveTagTask", 4096, NULL, 1, NULL);
	xTaskCreate(AddTagTaskStreaming, "AddTagTaskStream", 4096, NULL, 1, NULL);
	xTaskCreate(checkIfTagIsForRemovalTask, "checkIfTagIsForRemovalTask", 4096, NULL, 10, NULL);
	xTaskCreate(ParkingTask, "ParkingTask", 4096, NULL, 1, NULL);
	xTaskCreate(sendSavedTasks, "SendSaveTask", 4096, NULL, 3, NULL);
	mqtt_app_start();

	}
