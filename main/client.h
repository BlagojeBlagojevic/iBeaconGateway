#ifndef CLIENT_H
#define CLIENT_H
#include "defs.h"
#include "esp_http_client.h"

#define TAG "Client"

extern void rest_get(const char* url);
extern void rest_post(const char* url);

#ifdef CLIENT_IMPLEMNTATION
esp_err_t _client_event_get(esp_http_client_event_handle_t event){
    switch(event->event_id){
      case HTTP_EVENT_ON_DATA:
        //esp_http_client_config_t temp;
       // memcpy(&temp, event->user_data, sizeof(esp_http_client_config_t));
        
          //printf("\n\n-----------POST-----------\n\n");
        printf("\n\n-----------GET-----------\n\n");
        printf("%s\n", (char *)event->data);
        break;

      default:
        break;
    }
  return ESP_OK;
}

extern void rest_get(const char* url){
    esp_http_client_config_t client_config = {
      .url = url,
      .method = HTTP_METHOD_GET,
      .cert_pem = NULL,
      .event_handler = _client_event_get,

    };
    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

esp_err_t _client_event_post(esp_http_client_event_handle_t event){
    switch(event->event_id){
      case HTTP_EVENT_ON_DATA:
        //esp_http_client_config_t temp;
        //memcpy(&temp, event->user_data, sizeof(esp_http_client_config_t));
        
        printf("\n\n-----------POST-----------\n\n");
          //printf("\n\n-----------GET-----------\n\n");
        printf("%s\n", (char *)event->data);
        break;

      default:
        break;
    }
  return ESP_OK;
}


extern void rest_post(const char* url)
{
    esp_http_client_config_t config_post = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = _client_event_post};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "test ...";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}


#endif
#endif