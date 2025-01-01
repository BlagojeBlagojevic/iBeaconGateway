#ifndef MQTT_H
#define MQTT_H
#include "defs.h"
#include "cert.h"
#include "tag.h"
#define TAG "MQTT"




//TBD CHANGE A NAME OF TOPICS
//DO WE SEND SOME TEST STUF TO NOW ARE WE ARE CONECTED
extern void mqtt_app_start(void);
extern void sendSavedTasks(void* parms);

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
	esp_mqtt_client_handle_t client = event->client;
	switch (event->event_id) {
		//IF WE CONECT WE NEED TO SUBSCRIBE TO ALL TOPIC

		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED\n");
			SET_STATE(MQTT);
			for(int i = 0; i < NUM_OF_TOPICS; i++) {
				esp_mqtt_client_subscribe(client, topics[i], 0);
				}

			//esp_mqtt_client_publish(client, "my_topic", "Hi to all from ESP32 .........", 0, 1, 0);
			break;
		case MQTT_EVENT_DISCONNECTED:
			countSubsripcitionEvents = 0;
			CLEAR_STATE(MQTT);
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			break;
		case MQTT_EVENT_SUBSCRIBED:
			//PROBOBLY MAKE SOME STUF TO BE ABEL TO SEND A DATA MAYBE REGISTER IT IN A STATE
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
			countSubsripcitionEvents++;
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			//TBD SHOUD NOT EXIST AS A EVENT
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			//TBD WAT WE ARE SETING UP HEAR
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			printf("\nTOPIC= %.*s\r\n", event->topic_len, event->topic);
			printf("DATA= %.*s\r\n", event->data_len, event->data);
			printf("addd\n\n");
			if(!strncmp(event->topic, topics[ADD_UUID], event->topic_len)) {
				printf("add uuid\n\n");
				int is = 0;
				for(int i = 0; i < num_reg_uuid; i++){
					if(!memcmp(saved_uuids[i], event->data, sizeof(uint8_t)*16)){
						is = 1;
						break;
					}
				}
				if(!is){
					printf("Does not exist\n");
					uint8_t data[event->data_len];
					//data = *event->data;
					memcpy(data, event->data, event->data_len);
					printf("DATA= %.*s\r\n", event->data_len, data);
					uint8_t temp[16];
					int counter = 0;
					for(int i = 0; i < 16; i++){
						char tempS[2];
						tempS[0] = data[counter++];
						tempS[1] = data[counter++];
						printf("%s ", tempS);
						counter++; 	
						temp[i] = (uint8_t)ahex2int(tempS[0], tempS[1]);
					}
					if(num_reg_uuid > MAX_TAGS){
						memcpy(saved_uuids[num_reg_uuid++], temp, sizeof(uint8_t)*16);
					}
					ESP_LOG_BUFFER_HEX("\n\n\nUUID", temp, ESP_UUID_LEN_128);
				}	
				}
			else if(!strcmp(topics[REMOVE_UUID], event->topic)) {
				for(uint8_t i = 0; i < num_reg_uuid; i++) {
					if(!memcmp(saved_uuids[i], event->data, sizeof(uint8_t)*16)) {
						for(uint8_t j = i; j < num_reg_uuid - 1; j++) {
							memcpy(&saved_uuids[j], &saved_uuids[j + 1], sizeof(uint8_t) * 16);
							}
						num_reg_uuid--;
						break;
						}
					}
				}
			else if(!strcmp(topics[ENV_FACTOR], event->topic)) {
				enviroment_factor = strtof(event->data, NULL);
				}
			else if(!strcmp(topics[POS_X], event->topic)) {
				pos_x = strtof(event->data, NULL);
				}
			else if(!strcmp(topics[POS_Y], event->topic)) {
				pos_y = strtof(event->data, NULL);
				}
			else if(!strcmp(topics[RSSI_TRESHOLD], event->topic)){
				rssi_treshold = (int)atoi(event->data);
			}
			break;
		case MQTT_EVENT_ERROR:
			//TBD WHAT IF ERROR IS HAPANING
			//ERROR IF WE CHANGE A CERT OR SOMETHING
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			break;
		default:
			ESP_LOGI(TAG, "Other event id:%d", event->event_id);
			break;
		}
	return ESP_OK;
	}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

	mqtt_event_handler_cb(event_data);
	vTaskDelay(1);
	}

extern void mqtt_app_start(void) {
	ESP_LOGI(TAG, "Starting mqtt");
	const esp_mqtt_client_config_t mqtt_cfg = {
		.broker = {
			//.address.hostname = "127.0.0.1",
			//.address.port = 1883,
			.address.uri = MQTTS,
			//.verification.use_global_ca_store = false,
			//.verification.certificate = NULL,
			///.verification = {
			//	.certificate = (const char*)cert,
		//		},

			//.verification.skip_cert_common_name_check = true,
			},
		//. credentials = {
	///		.username =  USER_CRED,
	//		.authentication.password = PASS_CRED,
	//		},
		};
	ESP_LOGI(TAG, "%s", cert);
	client = esp_mqtt_client_init(&mqtt_cfg);
	//ESP_LOGI(TAG, "Starting mqtt");
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
	esp_mqtt_client_start(client);
	}

extern void sendSavedTasks(void* parms) {
	while(1) {

		while(countNumOfWritenFilesIntoASpiff > 0 && GET_STATE(MQTT)
		      && (countSubsripcitionEvents == NUM_OF_TOPICS)) {
			char filePath[20];
			sprintf(filePath, "/storage/%d.bin", countNumOfWritenFilesIntoASpiff);
			ESP_LOGI("mqtt", "File path (%s)\n", filePath);
			FILE *f = fopen(filePath, "rb");
			if(f == NULL) {
				printf("Coude not open a file (%s)\n", filePath);
				countNumOfWritenFilesIntoASpiff--;
				break;

				}

			Topics top;
			int size;
			fread(&top, sizeof(Topics), 1, f);
			fread(&size, sizeof(int), 1, f); //SHOUDE BE THE SAME MAYBE ERROR CHECKING I COURUPTED
			switch(top) {
				case ADD_TAG: {
						Tag tempTag;
						fread(&tempTag, sizeof(Tag), 1, f);
						PUBLISH_TAG(tempTag, ADD_TAG);
						break;
						}
				case REMOVE_TAG : {
						Tag tempTag;
						fread(&tempTag, sizeof(Tag), 1, f);
						PUBLISH_TAG(tempTag, REMOVE_TAG);
						break;
						}

				default: {
						ESP_LOGI("mqtt", "We shoud not get hear\n");
						break;
						}
				}
			fclose(f);
			countNumOfWritenFilesIntoASpiff--;
			DELAY(100);
			}
		DELAY(10000);
		}
	}
#endif
