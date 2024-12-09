#include "server.h"

//TBD full asynchronus server

void _get_page_handler(void *params) {
	int send = 0;
	
	START_ASYNC_HANDLE(GET_PAGE);
	httpd_resp_set_type(req, "Content-Type: text/json");
	const char *resp = "";
	for(int i = 0; i < 1; i++){
			PUBLISH_TAG_HTTP(g_tag[i]);
	}
	//const char *respa = "";
	//httpd_resp_send_chunk(req, respa, HTTPD_RESP_USE_STRLEN);
	END_ASYNC_HANDLE(GET_PAGE);
	}

void _post_change_stateBle(void *params) {
	START_ASYNC_HANDLE(BLE_POST);
	char content[2000];
	//memset(content, '\0', sizeof(char)*1000);
	i32 ret_time = httpd_req_recv(req, content, req->content_len);
	vTaskDelay(1);
	if(ret_time == HTTPD_SOCK_ERR_TIMEOUT) {
		httpd_resp_send_408(req);
		vTaskDelay(1);
		END_ASYNC_HANDLE(BLE_POST);
		}
	parseTags(content);
	//ESP_LOGI("Content","%s\n\n", content);
	const char *resp = "";
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	vTaskDelay(1);
	//DELAY(10000);
	END_ASYNC_HANDLE(BLE_POST);
	}



void _post_change_stateCamera(void *params) {
	START_ASYNC_HANDLE(CAMERA_POST);
	volatile char content[1000];
	i32 ret_time = httpd_req_recv(req, content, req->content_len);
	//ESP_LOGI(tag, "content:\n%s\n", content);
	vTaskDelay(1);
	if(strstr(content, "trigger")!= NULL) {
		const char *resp = "";
		httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
		vTaskDelay(1);
		parseParkingTriger(content);
		vTaskDelay(1);
		END_ASYNC_HANDLE(CAMERA_POST);
		}

	else if(strstr(content, "interval")!=NULL) {
		parseParkingInterval(content);
		const char *resp = "";
		httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
		vTaskDelay(1);
		END_ASYNC_HANDLE(CAMERA_POST);
		}
	vTaskDelay(1);
	END_ASYNC_HANDLE(CAMERA_POST);
	}



void server_init() {

	ASYNC_SERVER_INIT();

	httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
	httpd_handle_t server_handle = NULL;
	server_config.lru_purge_enable = true;
	httpd_start(&server_handle, &server_config);
	memset(g_p, 0, sizeof(ParkingPlace*) * 100);
	if(server_handle == NULL) {
		ERROR("We coude not init server!!!\n");
		}
		//httpd_uri_t default_uri = {

		//.uri = "/get",
		//.method = HTTP_GET,
		//.handler = ASYNC_HANDLER_FUNCTION_NAME(_get_page_handler),
		//.user_ctx = NULL
		//};
	httpd_uri_t post_uri = {
		.uri      = "/postCamera",
		.method   = HTTP_POST,
		.handler  = ASYNC_HANDLER_FUNCTION_NAME(_post_change_stateCamera),
		.user_ctx = NULL,
		};

	httpd_uri_t posta_uri = {
		.uri      = "/postBle",
		.method   = HTTP_POST,
		.handler  = ASYNC_HANDLER_FUNCTION_NAME(_post_change_stateBle),
		.user_ctx = NULL,
		};


	//httpd_register_uri_handler(server_handle, &default_uri);
	httpd_register_uri_handler(server_handle, &post_uri);
	httpd_register_uri_handler(server_handle, &posta_uri);
	}
