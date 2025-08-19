#include "tag.h"

static void addTag(const char* time, int majorID, int minorID, int rssi, int refpower, uint8_t* uuid) {
	uint8_t isTagExist = 0;
	for(int i = 0; i < num_g_tag; i++) {
		//IF
		if(majorID == g_tag[i].majorID && (minorID == g_tag[i].minorID)) {
			
			int64_t curentTime = esp_timer_get_time() - g_tag[i].systemTime;
			if(applayKalman){
				kalman1d_update(&g_tag[i].kalman, rssi);
				g_tag[i].rssi = g_tag[i].kalman.x;
			}
			else if(!g_tag[i].isStanding) {
				xQueueSend(xQueueTagAddedToList, &i, 10);
				}
			g_tag[i].isStanding = 1;
			g_tag[i].systemTime = esp_timer_get_time();
			if(applayKalman && justSend && (countSubsripcitionEvents == NUM_OF_TOPICS && GET_STATE(MQTT))){
				xQueueSend(xQueueTagStreaming, &g_tag[i], 1);
			}
			return;
			}
		}
	//ADDING NEW
	if(!isTagExist && (rssi >= rssi_treshold)) {
		if(num_g_tag >= MAX_TAGS) {
			printf("We have recived max amount of tags\n");
			return;
			}
		//xQueueSend(xQueueTagAddedToList, &i, 10);
		strcpy(g_tag[num_g_tag].time, time);
		g_tag[num_g_tag].majorID = majorID;
		g_tag[num_g_tag].minorID = minorID;
		if(uuid != NULL)
			memcpy(g_tag[num_g_tag].proximity_uuid, uuid, sizeof(uint8_t)*16);
		g_tag[num_g_tag].rssi = rssi;
		g_tag[num_g_tag].refpower = refpower;
		kalman1d_init(&g_tag[num_g_tag].kalman, 0, VAR0, MESURMENT_VAR, PROCES_VAR); 
		g_tag[num_g_tag].systemTime = esp_timer_get_time();
		g_tag[num_g_tag].isStanding = 0;
		num_g_tag++;
		}
	}

extern void checkIfTagIsForRemoval(void) {
	for(int i = 0; i < num_g_tag; i++) {
		int64_t elapsedTime =  esp_timer_get_time() -  g_tag[i].systemTime ;
		if(elapsedTime < 0) {
			elapsedTime = -elapsedTime;
			}
		if(elapsedTime > time_limit) {
			if(g_tag[i].isStanding == 1) {
				xQueueSend(xQueueTagRemovedFromList, &g_tag[i], 10);
				}

			for(int j = i; j < num_g_tag - 1; j++) {
				strcpy(g_tag[j].time, g_tag[j+1].time);
				g_tag[j].majorID = g_tag[j+1].majorID;
				g_tag[j].minorID = g_tag[j+1].minorID;
				g_tag[j].rssi = g_tag[j+1].rssi;
				g_tag[j].refpower = g_tag[j+1].refpower;
				g_tag[j].isStanding = g_tag[j+1].isStanding;
				g_tag[j].systemTime = g_tag[j+1].systemTime;
				}
			if(num_g_tag > 0) {
				num_g_tag--;
				}
			}
		}
	}

extern void parseTags(const char* content) {

	cJSON *json = cJSON_Parse(content);
	ERR_JSON(json)
	cJSON *obj = cJSON_GetObjectItemCaseSensitive(json, "obj");
	ERR_JSON(obj);
	cJSON *tagg = NULL;
	cJSON_ArrayForEach(tagg, obj) {
		cJSON *refpower = cJSON_GetObjectItemCaseSensitive(tagg, "refpower");
		ERR_JSON(refpower);
		cJSON *majorID = cJSON_GetObjectItemCaseSensitive(tagg, "majorID");
		ERR_JSON(majorID);
		cJSON *minorID = cJSON_GetObjectItemCaseSensitive(tagg, "minorID");
		ERR_JSON(minorID);
		cJSON *rssi = cJSON_GetObjectItemCaseSensitive(tagg, "rssi");
		ERR_JSON(rssi);
		//printf("rssi %d\n", rssi->valueint);
		cJSON *time = cJSON_GetObjectItemCaseSensitive(tagg, "time");
		ERR_JSON(time);
		addTag(time->valuestring, majorID->valueint, minorID->valueint,
		       rssi->valueint, refpower->valueint, NULL);
		}
	cJSON_Delete(json);
	}

static void printTags() {
	printf("\nNum of tags on parking %d\n", num_g_tag);
	for(int i = 0; i < num_g_tag; i++) {
		printf("\n------------------------------------------\n");
		printf("refpower : %d\n", g_tag[i].refpower);
		printf("majorID : %d\n", g_tag[i].majorID);
		printf("minorID : %d\n", g_tag[i].minorID);
		printf("rssi: %d\n", g_tag[i].rssi);
		printf("system time: %lld\n", g_tag[i].systemTime);
		printf("time: %s\n", g_tag[i].time);
		printf("is tag on location %u\n", g_tag[i].isStanding);
		printf("------------------------------------------\n");
		}
	}

static void printTag(int i) {
	printf("\nNum of tags on parking %d\n", num_g_tag);
		{
		printf("\nNum of tag %d\n", i);
		printf("\trefpower : %d\n", g_tag[i].refpower);
		printf("\tmajorID : %d\n", g_tag[i].majorID);
		printf("\tminorID : %d\n", g_tag[i].minorID);
		printf("\trssi: %d\n", g_tag[i].rssi);
		printf("\tsystem time: %lld\n", g_tag[i].systemTime);
		printf("\ttime: %s\n", g_tag[i].time);
		printf("\tis tag on location %u\n", g_tag[i].isStanding);
		printf("------------------------------------------\n");
		}
	}

//KAKO DA ZNAMO DA JE DODATO DALI IMA NESTO ILI DRUGACIJI OBLIK JSONA
//ONDA BI SE TREBALO DA SE IMPLEMENTIRA LOGIKA NA BECKU DA MI RADIMO OBRADU
//MOZDA DA SE DODA SAMO JEDNO POLJE KAO ILI BROJ ILI STRING ILI STA GOD
//DA OZNACI DA JE ADDDED TAG
//ILI DA IMA TOPIC DA JE TAG DODAT KAKO GOD
//OVO JE MOZDA LAKSE ZA IMP
//const char* event = "ADDED";


//FILE STRUCTURE IS %u%d<Elemet>
//              topic numE Element   
//MAYBE NO COPY
static void WriteTagToAFile(Tag tempTag, Topics topic){
	char filePath[40];
	
	sprintf(filePath, "/storage/%d.bin", countNumOfWritenFilesIntoASpiff);
	printf("File path %s\n", filePath);
	FILE *f = fopen(filePath, "wb+");
	if(f == NULL){
		ESP_LOGI("tag", "Failed to open a file for a writing\n");
		return; 
	}
	fwrite(&topic, sizeof(Topics), 1, f);
	int sizeTag = (int)sizeof(Tag);
	fwrite(&sizeTag, sizeof(int), 1, f);
	fwrite(&tempTag, sizeof(Tag), 1, f); 
	fclose(f);
	countNumOfWritenFilesIntoASpiff++;
}

extern void AddTagTask(void* parms) {
	static int numOfTag;
	while(1) {
		while(xQueueReceive(xQueueTagAddedToList, &numOfTag, 10) == pdTRUE) {
			printf("Added tag:\n");
			if(countSubsripcitionEvents == NUM_OF_TOPICS && GET_STATE(MQTT)) {
				PUBLISH_TAG(g_tag[numOfTag], ADD_TAG);
				DELAY(10);
				}
			else {
				WriteTagToAFile(g_tag[numOfTag], ADD_TAG);
				//printf("TBD load into a file systems\n");
				DELAY(10);
				}
			}
		DELAY(1000);
		}
	}
extern void AddTagTaskStreaming(void* parms) {
	Tag tags;
	while(1) {
		while(xQueueReceive(xQueueTagStreaming, &tags, 10) == pdTRUE) {
			printf("Added tag stream:\n");
			if(countSubsripcitionEvents == NUM_OF_TOPICS && GET_STATE(MQTT)) {
				
				PUBLISH_TAG(tags, ADD_TAG);
				
				DELAY(10);
				}
			}
		DELAY(1000);
		}
	}





extern void RemoveTagTask(void* parms) {
	static Tag tempTag;
	while(1) {
		while(xQueueReceive(xQueueTagRemovedFromList, &tempTag, 10) == pdTRUE) {
			printf("removed tag:\n");
			if(countSubsripcitionEvents == NUM_OF_TOPICS && GET_STATE(MQTT)) {

				PUBLISH_TAG(tempTag, REMOVE_TAG);
				DELAY(10);
				}
			else {
				//TBD Write this as a task
				WriteTagToAFile(tempTag, REMOVE_TAG);
				DELAY(10);
				//printf("TBD load into a file systems\n");
				}
			}
		DELAY(1000);
		}
	}

extern void checkIfTagIsForRemovalTask(void* parms) {
	while(1) {
		checkIfTagIsForRemoval();
		DELAY(6000);
		}
	}
	
extern int isUuidRegister(uint8_t *uuid){
	int is = 0;
	for(int i = 0; i < num_reg_uuid; i++){
		ESP_LOG_BUFFER_HEX("\n\n\nUUID", saved_uuids[i],
					                   ESP_UUID_LEN_128);
		if(!memcmp(uuid, saved_uuids[i], sizeof(uint8_t) * 16)){
			return 1;
		}
	}
	return is;
}
