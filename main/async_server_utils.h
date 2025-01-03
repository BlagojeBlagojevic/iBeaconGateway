#ifndef _ASYNC_SERVER_UTILS_H
#define _ASYNC_SERVER_UTILS_H
#include "defs.h"
#include "server.h"

#define WORKER_PRIO 1
#define WORKER_STACK_SIZE 4000
#define MAX_AMOUNT_OF_WORKERS 30
//#define LOG_NUM_OF_WORKERS

enum {
	GET_PAGE = 0,
	BLE_POST,
	CAMERA_POST,
	NUM_OF_HANDLERS
	};

static QueueHandle_t xQueueAsyncReq[NUM_OF_HANDLERS];
static SemaphoreHandle_t xSemaphoreCountWorker;
volatile uint8_t countTasks = 0;
#define ASYNC_SERVER_INIT(VOID)\
	xSemaphoreCountWorker = xSemaphoreCreateCounting(MAX_AMOUNT_OF_WORKERS, 0);\
	for(int i = 0; i < MAX_AMOUNT_OF_WORKERS; i++)xSemaphoreGive(xSemaphoreCountWorker);\
	for(int i = 0; i < NUM_OF_HANDLERS; i++) {\
		xQueueAsyncReq[i] = xQueueCreate(MAX_AMOUNT_OF_WORKERS, sizeof(httpd_req_t *));}


//THIS MACRO IS USED ON START OF A TASK HANDLER THIS MACRO DEFINES req OBJECT CUZ
//OUR HANDLER HAS A PROTTIPE FOR A TASK CUZ IT IS A ONE
#define START_ASYNC_HANDLE(H_ENUM)\
	httpd_req_t* req = NULL; \
	while(xQueueReceive(xQueueAsyncReq[(H_ENUM)], &req, 0) != pdTRUE) {\
		printf("Fail\n");req = NULL;\
		vTaskDelay(100);\
		}


//THIS MACRO IS TO BE USED ON END OF A TASK HANDLER
//THIS MACRO FREE MEMORY FROM COPY AND DELETE A TASK
#define END_ASYNC_HANDLE(H_ENUM)\
		{httpd_req_async_handler_complete(req);\
		xSemaphoreGive(xSemaphoreCountWorker);\
		vTaskDelete(NULL);}

//THIS MACRO GENERATES A FUNCTION BASED ON A FUNCTION NAME
//AND A ENUM_HANDLER NAME
// USE THIS IN TESTING 
//ESP_LOGI("tag", "Available num of tasks = %d\n",(int)uxSemaphoreGetCount(xSemaphoreCountWorker));
#define ASYNC_HANDLER_REGISTER_FUNCTION(FUNCTION_NAME, ENUM_HANDLER)\
	esp_err_t FUNCTION_NAME##_async_handler(httpd_req_t *req) {\
		if(xSemaphoreTake(xSemaphoreCountWorker, 0) == pdTRUE){\
			httpd_req_t *copy = NULL;\
			esp_err_t err = httpd_req_async_handler_begin(req, &copy);\
			if (err != ESP_OK) {\
				printf("Error\n");\
				return err;\
				}\
			xQueueSend(xQueueAsyncReq[ENUM_HANDLER], &copy, 10);\
			xTaskCreate(FUNCTION_NAME, NULL, WORKER_STACK_SIZE, NULL, WORKER_PRIO , NULL);vTaskDelay(10);\
			return ESP_OK;}\
		else{httpd_resp_send_408(req); \
		return ESP_OK;}\
		}

//THIS MACRO IS TO BE USED IN REGISTREING URI
#define ASYNC_HANDLER_FUNCTION_NAME(FUNCTION_NAME) FUNCTION_NAME##_async_handler


#endif
