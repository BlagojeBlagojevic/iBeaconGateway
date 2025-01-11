#ifndef WIFI_H
#define WIFI_H
#include "defs.h"

#define  tag  "WIFI"


//OR WE PUT THEM IN FILE SYSTEM AND THEN GET THEM OUT
#define SSID "Blagojevic"
#define PASSWORD "znam1234"

extern void wifi_init();



#ifdef WIFI_IMPLEMENTATION
static inline void _conection_start(u16 delay_ms){

       u8 repate_times = 0;
       while(esp_wifi_connect() != ESP_OK){
        repate_times++;
        DELAY(delay_ms);
        if(repate_times == 100){
          ERROR("We tried to conect to wifi to many times!!! \n\n");
          break;
        }


} 
}

static void _wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, 
                               i32 event_id, void *event_data)
{
   switch (event_id)
   {
   case WIFI_EVENT_STA_START:
       ESP_LOGI(tag, "WiFi connecting ... \n");
       CLEAR_STATE(WIFI);
       _conection_start(100);
       
       break;
   case WIFI_EVENT_STA_CONNECTED:
       ESP_LOGI(tag, "WiFi connected ... \n");
       break;
   case WIFI_EVENT_STA_DISCONNECTED:
       ESP_LOGI(tag, "WiFi lost connection ... \n");
       CLEAR_STATE(WIFI);
       _conection_start(100);
       break;
   case IP_EVENT_STA_GOT_IP:
   ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
       ESP_LOGI(tag, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
       ESP_LOGI(tag, "\nWiFi got IP ... \n\n");
       SET_STATE(WIFI);
       break;
   default:
       break;
   }
}


//WIFI EVENT HANDLER MUST BE DEFINED

extern void wifi_init(){
    
     
    nvs_flash_init();
    esp_netif_init();                    // TCP/IP initiation 					
    esp_event_loop_create_default();     // event loop 			                
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,  _wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, _wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD}};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    while(esp_wifi_connect() != ESP_OK);
    //we block all other tasks if we do not have wifi conection
    //BLOCK_TASK(WIFI, 100);
}

#endif

#endif
