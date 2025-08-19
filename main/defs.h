#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


//DELAY IN MS DEFINITION
#define DELAY(x)  vTaskDelay((x) / portTICK_PERIOD_MS);

//ESP LIBS WE DEPEND ON
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "esp_eth_enc28j60.h"
#include "driver/spi_master.h"
#include "mqtt_client.h"
//#include "esp_tls.h"
#include "esp_ota_ops.h"
#include "esp_spiffs.h"

//BLE LIBS
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_sntp.h"




//external libs
#include "cJSON.h"

#define ERROR(...) ESP_LOGE(tag, __VA_ARGS__); ESP_ERROR_CHECK(ESP_FAIL);
#define ERR_JSON(jsona) if(jsona == NULL) {printf("ERR in "#jsona"\n");if(json!=NULL) cJSON_Delete(json);return;}
//TYPES DEFINES

#include<stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;


//THIS PART IS LIGHT WEIGHT IMPLEMENTATION OF SEMAPHORES FOR BLOCKING OF A TASK

typedef enum {
	ETHERNET = 0,
	WIFI,
	MQTT,
	EVENT_ADDED_TAG,
	EVENT_REMOVED_TAG,
	EVENT_TRIGER_PARKING_AREA,
	EVENT_ERROR,
	EVENT_RECIVE_PARAMETAR,
	NUM_OF_STATES
	} State;


volatile u8 _STATE[NUM_OF_STATES];
#define BLOCK_TASK(x, time)  while(!_STATE[x]){DELAY((time));}
#define SET_STATE(x)        _STATE[x] =  true
#define CLEAR_STATE(x)      _STATE[x] =  false
#define GET_STATE(x)        _STATE[x]


#include "freertos/queue.h"



//MQTT GLOBAL CLIENT i GLOBALNI ENUM
typedef enum {
	
	ADD_TAG = 0,
	REMOVE_TAG,
	CHANGE_PARKING,
	ADD_UUID,
	REMOVE_UUID,
	ENV_FACTOR,
	POS_X,
	POS_Y,
	RSSI_TRESHOLD,
	STREAMING,
	NAME_OF_BEACON,
	NUM_OF_TOPICS

	} Topics;
//PRAVI PUT DO
const char* topics[]= {
	"parking/ADD_TAG",
	"parking/REMOVE_TAG",
	"parking/CHANGE_PARKING",
	"parking/ADD_UUID",
	"parking/REMOVING_UUID",
	"parking/ENV_FACTOR",
	"parking/POS_X",
	"parking/POS_Y",
	"parking/RSSI_TRESHOLD",
	"parking/STREAMING",
	"parking/NAME_OF_BEACON",
	"NUM_OF_TOPICS",
	};

esp_mqtt_client_handle_t client;
static volatile float enviroment_factor = 2.1;
static volatile float pos_x = 0;
static volatile float pos_y = 0;

//RSSI = -10.0f * N * log10((distance)) + RSSI(Distance.1m);

//Distance = 10^((Measured Power - Instant RSSI)/(10*N)).
#include<math.h>
static inline double distance(int rssi, int ref){
	const double temp = (ref - rssi) / (double)(10.0f * enviroment_factor);
	return pow(10.0f, temp);
}

//char *uuid = (char*)tempTag.proximity_uuid;
#define PUBLISH_TAG(tempTag, top)\
	char uuid[35];\
	memset(uuid, 0, sizeof(char) * 35);\
	int counter = 0;\
	for(int i = 0; i < 16; i++){\
		uuid[counter++] = (int)tempTag.proximity_uuid[i]/10 + '0';\
		uuid[counter++] = (int)tempTag.proximity_uuid[i]%10 + '0';\
	}\	
	char buffer[1000];\
	const int refPower = tempTag.refpower;\
	const int majorId = tempTag.majorID;\
	const int rssi = tempTag.rssi;\
	const int minorId = tempTag.minorID;\
	const char* time = tempTag.time;\
	const double dist = distance(rssi, refPower);\
	sprintf(buffer,\
	        "{\n"\
			"\"name\": \"%s\",\n"\
	        "\"refpower\": %d,\n"\
	        "\"uuid\": \"%s\",\n"\
	        "\"majorID\": %d,\n"\
	        "\"rssi\": %d,\n"\
	        "\"minorID\": %d,\n"\
	        "\"time\": \"%s\",\n"\
	        "\"distance\": %f,\n"\
	        "\"x\": %f,\n"\
	        "\"y\": %f\n"\
					"}"\
	         ,name_of_beacon, refPower, uuid,  majorId, rssi, minorId, time, dist, pos_x, pos_y);\
	esp_mqtt_client_publish(client,topics[top],buffer, 0, 1, 0);



#define PUBLISH_TAG_HTTP(tempTag)\
	char *temp = "";\
	char buffer[1000];\
	const int refPower = tempTag.refpower;\
	const int majorId = tempTag.majorID;\
	const int rssi = tempTag.rssi;\
	const int minorId = tempTag.minorID;\
	const char* time = tempTag.time;\
	sprintf(buffer,\
	        "{\n"\
	        "\"msg\": \"advData\", \"obj\": [\n"\
	        "{\n"\
	        "\"dmac\": \"%s\",\n"\
	        "\"refpower\": %d,\n"\
	        "\"uuid\": \"%s\",\n"\
	        "\"majorID\": %d,\n"\
	        "\"rssi\": %d,\n"\
	        "\"minorID\": %d,\n"\
	        "\"type\": 4,\n"\
	        "\"time\": \"%s\"\n"\
	        "},\n"\
	        "],\n"\
	        "\"gmac\": \"D03304002122\" }\n"\
	        ,temp, refPower, temp,  majorId, rssi, minorId, time);\
	httpd_resp_send_chunk(req, buffer, HTTPD_RESP_USE_STRLEN);


#define PUBLISH_PARKING(tempParking, top) {\
		const char* time = tempParking.time;\
		const char* parking_area = tempParking.name;\
		const int occupancy = tempParking.isOcupied;\
		char buffer[1000];\
		sprintf(buffer,\
		        "{\"event\": \"Parking Space Detection\"\n""\"device\": \"Network Camera\",\n"\
		        "\"time\": \"%s\",\n"\
		        "\"parking_area\": \"%s\",\n"\
		        "\"occupancy\": %u,\n}"\
		        , time, parking_area, occupancy);\
		esp_mqtt_client_publish(client,topics[top], buffer, 0, 1, 0);\
		}


// makes a number from two ascii hexa characters
int ahex2int(char a, char b){

    a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
    b = (b <= '9') ? b - '0' : (b & 0x7) + 9;

    return (a << 4) + b;
}

static volatile int countSubsripcitionEvents = 0; 
static volatile int countNumOfWritenFilesIntoASpiff = 0;
struct timeval *tv = NULL;
static volatile int justSend = 1;
static volatile int applayKalman = 1;
static volatile char name_of_beacon[25];
QueueHandle_t xQueueTagAddedToList;  //xQueueCreate(100, sizeof(int));
QueueHandle_t xQueueTagRemovedFromList;
QueueHandle_t xQueueTagStreaming;

QueueHandle_t xQueueParking;


//QueueHandle_t xQueueCarLeavePlace;



#endif
