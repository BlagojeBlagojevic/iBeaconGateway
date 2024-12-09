#include "parking.h"

static void addValueToParking(const char* name, const char* time, uint8_t isOcupied) {
	uint8_t isParkingExist = 0;
	for(int i = 0; i < num_g_p; i++) {
		if(!strcmp(name, g_p[i].name)) {
			if(g_p[i].isOcupied != isOcupied) {
				xQueueSend(xQueueParking, &i,10);
				}
			g_p[i].isOcupied = isOcupied;
			strcpy(g_p[i].time, time);
			isParkingExist = 1;
			return;
			}
		}
	if(!isParkingExist) {
		g_p[num_g_p].isOcupied = isOcupied;
		strcpy(g_p[num_g_p].time, time);
		strcpy(g_p[num_g_p].name, name);
		xQueueSend(xQueueParking, &num_g_p,10);
		if(num_g_p < MAX_PARKING)
			num_g_p++;
		}
	}

extern void parseParkingTriger(const char* content) {
	cJSON *json = cJSON_Parse(content);
	ERR_JSON(json);
	cJSON *time = cJSON_GetObjectItemCaseSensitive(json, "time");
	ERR_JSON(time);
	cJSON *occupancy = cJSON_GetObjectItemCaseSensitive(json, "occupancy");
	ERR_JSON(occupancy);
	cJSON *parking_area = cJSON_GetObjectItemCaseSensitive(json, "parking_area");
	ERR_JSON(parking_area);
	cJSON *index_number = cJSON_GetObjectItemCaseSensitive(json, "index_number");
	ERR_JSON(index_number);   //AKO SE PROMJENI ONDA PRVO
	char buffer[40];
	sprintf(buffer, "%s%d",parking_area->valuestring, index_number->valueint);
	addValueToParking(buffer, time->valuestring, occupancy->valueint);
	cJSON_Delete(json);
	}

extern void parseParkingInterval(const char* content) {
	num_g_p = 0;
	cJSON *json = cJSON_Parse(content);
	ERR_JSON(json);
	cJSON *time = cJSON_GetObjectItemCaseSensitive(json, "time");
	ERR_JSON(time);
	cJSON *parkings = cJSON_GetObjectItemCaseSensitive(json, "parking_detail");
	ERR_JSON(parkings);
	cJSON *parking = NULL;
	cJSON_ArrayForEach(parking, parkings) {
		cJSON *area_name =  cJSON_GetObjectItemCaseSensitive(parking, "area_name");
		ERR_JSON(area_name);
		cJSON *occupancy =  cJSON_GetObjectItemCaseSensitive(parking, "occupancy");
		ERR_JSON(occupancy);
		cJSON *place;
		cJSON *numbering_scheme = cJSON_GetObjectItemCaseSensitive(parking, "numbering_scheme");
		ERR_JSON(numbering_scheme);
		cJSON *schema = cJSON_GetArrayItem(numbering_scheme, 0);
		ERR_JSON(schema);
		int counter = schema->valueint;
		cJSON_ArrayForEach(place, occupancy) {
			char buffer[40];
			sprintf(buffer, "%s%d", area_name->valuestring, counter);
			addValueToParking(buffer, time->valuestring, place->valueint);
			counter++;
			}

		}
	cJSON_Delete(json);
	}

static void printParkings() {
	for(int i = 0; i < num_g_p; i++) {
		printf("Place %d\n", i);
		printf("------------------------------------------\n");
		printf("time: %s\n", g_p[i].time);
		printf("name: %s\n", g_p[i].name);
		printf("is ocupied: %d\n", (int)g_p[i].isOcupied);
		printf("------------------------------------------\n");
		}
	}

static void printParking(int i) {
		{
		printf("\nPlace %d\n", i);
		printf("------------------------------------------\n");
		printf("time: %s\n", g_p[i].time);
		printf("name: %s\n", g_p[i].name);
		printf("is ocupied: %d\n", (int)g_p[i].isOcupied);
		printf("------------------------------------------\n");
		}
	}

static void WriteParkingToAFile(ParkingPlace tempParking, Topics topic){
	char filePath[40];
	sprintf(filePath, "/storage/%d.bin", countNumOfWritenFilesIntoASpiff);
	printf("File path %s\n", filePath);
	FILE *f = fopen(filePath, "wb+");
	if(f == NULL){
		ESP_LOGI("parking", "Failed to open a file for a writing\n");
		return; 
	}
	fwrite(&topic, sizeof(Topics), 1, f);
	int sizeParking = (int)sizeof(ParkingPlace);
	fwrite(&sizeParking, sizeof(int), 1, f);
	fwrite(&tempParking, sizeof(ParkingPlace), 1, f); 
	fclose(f);
	countNumOfWritenFilesIntoASpiff++;
}




extern void ParkingTask(void* parms) {
	static int numOfParkingPlace;
	while(1) {
		while(xQueueReceive(xQueueParking, &numOfParkingPlace, 10) == pdTRUE) {
			printf("Parking changed:\n");
			if(countSubsripcitionEvents == NUM_OF_TOPICS && GET_STATE(MQTT)) {
				PUBLISH_PARKING(g_p[numOfParkingPlace], CHANGE_PARKING);
				DELAY(10);
				}
			else {
				//TBD Write this as a task
				WriteParkingToAFile(g_p[numOfParkingPlace], CHANGE_PARKING);
				DELAY(10);
				//printf("TBD implement a loading into a file\n");
				}
			}
		}
	DELAY(100);
	}
	