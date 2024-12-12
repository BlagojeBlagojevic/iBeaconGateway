
# ESP32

## esp_log.h

Enable using CONFIG_LOG_MASTER_LEVEL or 

```c
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

```

```c
esp_log_level_set("*", ESP_LOG_VERBOSE);

```
ESP_LOG"LEVEL"  ` ESP_LOGI(const char* tag, const char* other);`

## Timing control 

#include "freertos/timers.h"

` port_TICK_PERIOD_MS `

`esp_timer_get_time() micro sec`

```c
#include "freertos/timers.h"
#define MS(x) (x) / port_TICK_PERIOD_MS
vTaskDelay(MS(x));
```

port_TICK_PERIOD_MS

## Task Control


`#include "freertos.h/tasks.h"`
Create task
```c
 BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                         const char * const pcName,
                         const configSTACK_DEPTH_TYPE uxStackDepth,
                         void *pvParameters,
                         UBaseType_t uxPriority,
                         TaskHandle_t *pxCreatedTask
                       );
```
Create task witch will execute on one core

```c
 BaseType_t xTaskCreatePinnedToCore( TaskFunction_t pvTaskCode,
                         const char * const pcName,
                         const configSTACK_DEPTH_TYPE uxStackDepth,
                         void *pvParameters,
                         UBaseType_t uxPriority,
                         TaskHandle_t *pxCreatedTask,
                         uint8_t witchCore
                       );
```
For handling if task is adel to be executed if contition is met we use Queue or global vars(volotaile)

`#include "freertos.h/queue.h"`

```c 
QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength,
                             UBaseType_t uxItemSize );
```

xQueueCreate returns QueueHandle_t 

probobly we will define global xQueue"SomeName"
```c
 BaseType_t xQueueSend(
                        QueueHandle_t xQueue,
                        const void * pvItemToQueue,
                        TickType_t xTicksToWait
                      );
```
xQueueSend put *pvItemToQueue in queue and will probably increase num of elements in queue ,xTicksToWait how mutch
procesor ticks we nead to wait `Must be converted to TickType_t` returns `pdTRUE` if succes else `pdFALSE`.

For handling if sometingh is on queue we use 
```c
BaseType_t xQueueReceive(
                          QueueHandle_t xQueue,
                          void *pvBuffer,
                          TickType_t xTicksToWait
);

```
If something is on it returns `pdTRUE` else `pdFALSE`  
`if(xQueueRecive(xQueue, &buffer, (TickType_t) 10) == pdTrue){//do something}`

If use queue from interupts use special recive, send functions.

For handling blocking of certian tasks we use semaphors. 

There are 2 types of semaphores binary and mutex.

Main difference between mutex and binary semaphore is in mutext if thread lock the critical section then it has to unlock critical section no other thread can unlock it, but in case of binary semaphore if one thread locks critical section using waits function then value of become "0".

To use semaphores we create global SemaphoreHandle_t xSemaphore;

To allocate binary semaphore use:
```c
SemaphoreHandle_t xSemaphoreCreateBinary( void );
```

To allocate mutex semaphore use:
```c
SemaphoreHandle_t xSemaphoreCreateMutex( void );
```

 `xSemaphoreTake()` locks sections of code to be executed untiled `xSemaphoreGive()`
 is called. For differance beatwen mutex and binary look above.

```c
 BaseType_t xSemaphoreTake( SemaphoreHandle_t xSemaphore,
                 TickType_t xTicksToWait );

```

```c
BaseType_t xSemaphoreGive( SemaphoreHandle_t xSemaphore );
```
Returns value `pdTRUE` , `pdFALSE` like above.

For suspending the task ve use `vTaskSuspend()`.
```c
void vTaskSuspend( TaskHandle_t xTaskToSuspend );
```
For deleting the task we use  `vTaskDelete()`.
```c
void vTaskDelete( TaskHandle_t xTaskToSuspend );

```
For more info on free RTOS https://www.freertos.org/

## GPIO 
In ESP32 near all pin could be changed to be what is desired(UART, I2C, I2S, ADC).
Inside ESP32 ther is IOMUX and GPIOMUX matrix witch job is to reroute pins internaly (like one giant global 
multiplexer).
https://docs.espressif.com/projects/esp-idf/en/v4.4.1/esp32/api-reference/peripherals/gpio.html

We use alredy prebuild driver.
`#include "driver/gpio.h"`
gpio_num_t is like GPIO_NUM_"NUMBER OF PIN".

Init of types of pins(INPUT, OUTPUT) is done using function 
`gpio_set_direction()`.

```c
esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode);
```

Wrap functon in ESP_ERROR_CHECK() for checking is seting of pin is posible (Probobly problem be RTC pins or if we set i2c, adc, ...).
gpio_mode_t `GPIO_MOD_OUTPUT, GPIO_MOD_INPUT` for seting direction of a pin.

If pin direction is set to be input we coulde get value of a pin.
`uint8_t gpio_get_level(gpio_num_t gpio_num)` 1 for HIGH 0 for LOW.
Probobly nead to `#define HIGH 1`, `#define LOW 0`.

If pin direction is set to be output then we coude set value on pin.
`esp_err_t gpio_set_level(gpio_num_t num, uint8_t value)`.

If we want to use interupts on GPIO pins we nead to set interrupt, and then to 
register callback(Function witch will run when interrupt is trigerd) ` gpio_set_intr_type()`.

```c
esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type)
```
gpio_int_type:
1.  GPIO_INTR_POSEDGE
2.  GPIO_INTR_NEGEDGE
3.  GPIO_INTR_ANYEDGE
4.  GPIO_INTR_LOW_LEVEL
5.  GPIO_INTR_HIGH_LEVEL



`gpio_isr_register()` is used to setup callback when interupt is happend.

```c
esp_err_t gpio_isr_register(void (*fn)(void*), void *arg, int intr_alloc_flags, gpio_isr_handle_t *handle)
```
`void (*fn)(void*)` is a shape of our function(just provide name of isr and isr must not return and except void* params).

Last step is to enable the interrupt `gpio_intr_enable()`.
```c
esp_err_t gpio_intr_enable(gpio_num_t gpio_num)
```
All pins have pull up and pull down resistors.
```c

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
//#include "led_strip.h"
#include "sdkconfig.h"
#include<string.h>
#define DELAY(x) vTaskDelay((x) / portTICK_PERIOD_MS);
static const gpio_num_t led_pins[] = {
    GPIO_NUM_10,
    GPIO_NUM_11,
    GPIO_NUM_25,
    GPIO_NUM_26,

};
static const gpio_num_t input_pins[] = {
    GPIO_NUM_15,
    GPIO_NUM_14,
};

#define NUM_OF_LED 4
#define DELAY_MS 1000


volatile IRAM_ATTR uint16_t delayTime = 1000;
void ledChangeInterupt(void){
IRAM_ATTR static uint8_t i;
   while(1){
    for (i = 0; i < NUM_OF_LED; i++)
    {
        gpio_set_level(led_pins[i], 1);
    }
    ESP_LOGI("nesto","HIGH\n");
    DELAY(delayTime);
    for (i = 0; i < NUM_OF_LED; i++)
    {
        gpio_set_level(led_pins[i], 0);
    }
    ESP_LOGI("nesto","LOW\n");
    DELAY(delayTime);
    ///return NULL;
}
}
void ledChangeDelayTime(void){
    while (1)
    {
        
    if (gpio_get_level(input_pins[0]))
    {
        delayTime+=10;
        ESP_LOGI("ledChangeDelayTime", "delay %u MS\n", delayTime);
    }
    if (gpio_get_level(input_pins[1]))
    {
        delayTime-=10;
        ESP_LOGI("ledChangeDelayTime", "delay %u MS\n", delayTime);
    }
    DELAY(100);

    }
    
}


void app_main(void)
{

    //SETUP 
    for (uint8_t i = 0; i < NUM_OF_LED; i++)
    {
        gpio_set_direction(led_pins[i], GPIO_MODE_OUTPUT);
    }
    gpio_set_direction(input_pins[0], GPIO_MODE_INPUT);
    gpio_set_direction(input_pins[0], GPIO_MODE_INPUT);
    DELAY(100);
    UBaseType_t prio = 1;
    TaskHandle_t ledChangeInteruptTaskHendler, ledChangeDelayTimeTaskHandler;
    memset(&ledChangeInteruptTaskHendler,  0, sizeof(TaskHandle_t));
    memset(&ledChangeDelayTimeTaskHandler, 0, sizeof(TaskHandle_t));
    xTaskCreatePinnedToCore(ledChangeInterupt, "TASK LED\n", 2048, 
                (void*)NULL, prio, &ledChangeInteruptTaskHendler, 1);
    xTaskCreatePinnedToCore(ledChangeDelayTime, "DELAY CHANGE\n", 4048, 
                (void*)NULL,prio, &ledChangeDelayTimeTaskHandler, 1);
   // while (1)
    {
     //   DELAY(1000);

    }
        
        
                
        
    }
    
```
### Bare metal gpio


```c
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GPIO_OUT_W1TS_REG 0x3FF44008
#define GPIO_OUT_W1TC_REG 0x3FF4400C
#define GPIO_ENABLE_REG   0x3FF44020
#define GPIO5             5
#define DELAY_TIME_MS     500
#define DELAY(x)          vTaskDelay((x) / portTICK_PERIOD_MS)

void app_main(void)
{
    volatile uint32_t* gpio_out_w1ts_reg = (volatile uint32_t*) GPIO_OUT_W1TS_REG;
    volatile uint32_t* gpio_out_w1tc_reg = (volatile uint32_t*) GPIO_OUT_W1TC_REG;
    volatile uint32_t* gpio_enable_reg   = (volatile uint32_t*) GPIO_ENABLE_REG;

    // Set the mode for GPIO5 to output
    *gpio_enable_reg = (1 << GPIO5);

    while(1) {
        // Set the level for GPIO5 to HIGH
        *gpio_out_w1ts_reg = (1 << GPIO5);

        // Delay
        DELAY(1000);

        // Set the level for GPIO5 to LOW
        *gpio_out_w1tc_reg = (1 << GPIO5);

        // Delay
        DELAY(1000);
    }
}


```

## WIFI 

This is the headers for witch ESP32 wifi app depenth on

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
```
Main recomendation is to use EXAMPLE witch is provided in examples.
Lets write ESP32 wifi in sta mode (MOD IN WITCH ESP32 ACTS LIKE A CLIENT).

Wifi initialisation has 4 main phases:

1. `Wi-Fi/LwIP Init Phase`
2. `Wi-Fi Configuration Phase`
3. `Wi-Fi Start phase`
4. `Wi-Fi Conect phase`

### First phase
First phase consist of:

1. The main task calls `esp_netif_init()` to create an LwIP core task and initialize LwIP-related work.

2. The main task calls `esp_event_loop_create()` to create a system Event task and initialize an application event's callback function. In the scenario above, the application event's callback function does nothing but relaying the event to the application task.

3. The main task calls `esp_netif_create_default_wifi_ap()` or `esp_netif_create_default_wifi_sta()` to create default network interface instance binding station or AP with TCP/IP stack.

4. The main task calls `esp_wifi_init()` to create the Wi-Fi driver task and initialize the Wi-Fi driver.

5. The main task calls OS API to create the application task.

```c
// 1 - Wi-Fi/LwIP Init Phase
	ESP_ERROR_CHECK(esp_netif_init()); //TCP
	ESP_ERROR_CHECK(esp_event_loop_create_default());//event loop	
	esp_netif_create_default_wifi_sta();//WIFI setup as station
	wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT(); //default config
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));	
```

Nvs flash must be enabled  `nvs_flash_init()` . 
All wifi_configs are saved in flash memory and on next setup will be loaded from it. 

### Second phase 

Second phase consist of :

1. Adding event loops for wifi and loading wifi credential(Same as if flash is enabled).

 ```c
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); //ADD EVENT HANDLER
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
 ```
 Code above add event loops for wifi events and ip events .
 
 1. Main wifi events:
   
     1.1 `WIFI_EVENT_STA_START` -> event for start of wifi as station.

     1.2 `WIFI_EVENT_STA_CONNECTED` -> event for handling if wifi is conected

     1.3 `WIFI_EVENT_STA_DISCONNECTED` -> event for handling if wifi is disconected
  
  
  3. Main ip event:

     2.1 `IP_EVENT_STA_GOT_IP`  -> event for handling if we got a ip addres.
   
 For all events consult https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/wifi.html
 or provided example . 
 `static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)`
EXAMPLE OF BASIC EVENT HANDLER IS:
 ```c
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

 
```
Then we nead to define config for wifi

```c
wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = ""}};

```
Set mode `esp_wifi_set_mode(WIFI_MODE_STA);`
and then call config `esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);`
### Third phase

Just consist of a starting wifi. `esp_wifi_start()`

### Fourth phase 

Just consist of a tring to conect to a wifi. `esp_wifi_connect()`


EXAMPLE OF WIFI INITIALISATION IS:

```c
//WIFI EVENT HANDLER MUST BE DEFINED
void wifi_init(){
    nvs_flash_init();
    esp_netif_init();                    // TCP/IP initiation 					
    esp_event_loop_create_default();     // event loop 			                
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = ""}};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}
```
ALL HANDLING EXAMPLE

```c
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

/** DEFINES **/
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

/** GLOBALS **/

// event group to contain status information
static EventGroupHandle_t wifi_event_group;

// retry tracker
static int s_retry_num = 0;

// task tag
static const char *TAG = "WIFI";
/** FUNCTIONS **/

//event handler for wifi events
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "Connecting to AP...");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

//event handler for ip events
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }

}

// connect to wifi and return the result
esp_err_t connect_wifi()
{
	int status = WIFI_FAILURE;

	/** INITIALIZE ALL THE THINGS **/
	//initialize the esp network interface
	ESP_ERROR_CHECK(esp_netif_init());

	//initialize default esp event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	//create wifi station in the wifi driver
	esp_netif_create_default_wifi_sta();

	//setup wifi station with the default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /** EVENT LOOP CRAZINESS **/
	wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    /** START THE WIFI DRIVER **/
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ssid-for-me",
            .password = "super-secure-password",
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    // start the wifi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    /** NOW WE WAIT **/
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_SUCCESS | WIFI_FAILURE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGI(TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

// connect to the server and return the result
void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_err_t status = WIFI_FAILURE;

	//initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // connect to wireless AP
	status = connect_wifi();
	if (WIFI_SUCCESS != status)
	{
		ESP_LOGI(TAG, "Failed to associate to AP, dying...");
		return;
	}
	
}

```
## ESP_HTTP_CLIENT

`esp_http_client` component provides a set of APIs for making HTTP/S requests from ESP-IDF applications. The steps to use these APIs are as follows:

1. `esp_http_client_init()`: Creates an `esp_http_client_handle_t` instance, i.e., an HTTP client handle based on the given `esp_http_client_config_t` configuration. This function must be the first to be called; default values are assumed for the configuration values that are not explicitly defined by the user.

2. `esp_http_client_perform()`: Performs all operations of the esp_http_client - opening the connection, exchanging data, and closing the connection (as required), while blocking the current task before its completion. All related events are invoked through the event handler (as specified in `esp_http_client_config_t`).

3. `esp_http_client_cleanup()`: Closes the connection (if any) and frees up all the memory allocated to the HTTP client instance. This must be the last function to be called after the completion of operations.

`esp_http_client_config_t` consist of next fields:
1. .url -> In this field we provide url (ip address also is aceptable)

2. .method -> This sets what method we want to use(all standard methods get/, post/, put/, delete/, ...)

3. .cer_pem -> In this method we nead to provide cert

4. .event_handler -> Hear we nead to provide handler function `esp_err_t event_http(esp_http_client_event_handle_t event)`:

   4.1 `HTTP_EVENT_ERROR`

   4.2 `HTTP_EVENT_ON_CONNECTED`

   4.3 `HTTP_EVENT_HEADERS_SENT`

   4.4 `HTTP_EVENT_ON_HEADER`

   4.5 `HTTP_EVENT_ON_DATA` 

   4.6 `HTTP_EVENT_ON_FINISH`

   4.7 `HTTP_EVENT_DISCONNECTED`

   4.8 `HTTP_EVENT_REDIRECT`

For other fields look : 
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/esp_http_client.html?highlight=esp%20client


### REST GET EXAMPLE:


1. First we nead to have conection to interent (either wifi or ethernet).
2. Then we nead to include next heders:
   ```c
	#include <stdio.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/timers.h"
	#include "freertos/event_groups.h"
	#include "esp_wifi.h"
	#include "esp_log.h"
	#include "nvs_flash.h"
	#include "esp_netif.h"
	#include "esp_http_client.h"
   ```

3. Then we nead to define http_event_handler: 
   ```c
 	esp_err_t client_event(esp_http_client_event_handle_t event){
    	switch(event->event_id){
      		case HTTP_EVENT_ON_DATA:
        	printf("%s\n", (char *)event->data);  //IF WE ARE GETING THE DATA WE WILL PRINT THEM
        break;

        default:
   		break;
    }
  	return ESP_OK;
	}
   ```

4. For get request we will query http://worldtimeapi.org/api/timezone/Europe/Sarajevo .
   This should return json.
   ```c
    static void rest_get(const char* url){
    	esp_http_client_config_t client_config = {
    	  .url = url,
    	  .method = HTTP_METHOD_GET,
      	  .cert_pem = NULL,                    //CREATE CONFIG
      	  .event_handler = client_event,        //ADD EVENT 

    		};
   	esp_http_client_handle_t client = esp_http_client_init(&client_config);//INIT CONFIG
   	esp_http_client_perform(client);//SEND DATA
    	esp_http_client_cleanup(client);//CLEAN DATA
	}
   ```
     
###  REST POST EXAMPLE 

For post we will indlude same headers and event hendlers
We will query http://httpbin.org/post this should return same staf as we are sending.
`esp_http_client_set_post_field(esp_http_client_handle_t , const char* , size_t )` is used to
sets a data for sending. `esp_http_client_set_header(client, "Content-Type", "application/json")`
is used to send parse data as JSON "Content-Type" probobly as a HTTP 1,1.

```c
static void rest_post(const char* url)
{
    esp_http_client_config_t config_post = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "test ...";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
```

ALL HANDLING EXAMPLE:

```c
#include <stdio.h>
#include<string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"


volatile bool  is_conected = false;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
   switch (event_id)
   {
   case WIFI_EVENT_STA_START:
       printf("WiFi connecting ... \n");
       break;
   case WIFI_EVENT_STA_CONNECTED:
       printf("WiFi connected ... \n");
       break;
   case WIFI_EVENT_STA_DISCONNECTED:
       printf("WiFi lost connection ... \n");
       is_conected = false;
       while(esp_wifi_connect() != ESP_OK);
       break;
   case IP_EVENT_STA_GOT_IP:
   ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
       printf("STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
       printf("\nWiFi got IP ... \n\n");
       is_conected = true;
       break;
   default:
       break;
   }
}


//WIFI EVENT HANDLER MUST BE DEFINED

void wifi_init(){
    
     
    nvs_flash_init();
    esp_netif_init();                    // TCP/IP initiation 					
    esp_event_loop_create_default();     // event loop 			                
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "Wokwi-GUEST",
            .password = ""}};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    while(esp_wifi_connect() != ESP_OK);
    
    
}

esp_err_t client_event_get(esp_http_client_event_handle_t event){
    switch(event->event_id){
      case HTTP_EVENT_ON_DATA:
        esp_http_client_config_t temp;
        memcpy(&temp, event->user_data, sizeof(esp_http_client_config_t));
        
          //printf("\n\n-----------POST-----------\n\n");
        printf("\n\n-----------GET-----------\n\n");
        printf("%s\n", (char *)event->data);
        break;

      default:
        break;
    }
  return ESP_OK;
}

static void rest_get(const char* url){
    esp_http_client_config_t client_config = {
      .url = url,
      .method = HTTP_METHOD_GET,
      .cert_pem = NULL,
      .event_handler = client_event_get,

    };
    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

esp_err_t client_event_post(esp_http_client_event_handle_t event){
    switch(event->event_id){
      case HTTP_EVENT_ON_DATA:
        esp_http_client_config_t temp;
        memcpy(&temp, event->user_data, sizeof(esp_http_client_config_t));
        
        printf("\n\n-----------POST-----------\n\n");
          //printf("\n\n-----------GET-----------\n\n");
        printf("%s\n", (char *)event->data);
        break;

      default:
        break;
    }
  return ESP_OK;
}


static void rest_post(const char* url)
{
    esp_http_client_config_t config_post = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "test ...";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
TaskHandle_t t[2];
//SemaphoreHandle xt;
void t1(void* args){
  
  while(!is_conected){
    //vTaskSuspend(&t[1]);
    printf("GET\n");
    vTaskDelay(100);
    
    //vTaskResume(&t[1]);
  }
  rest_get("http://worldtimeapi.org/api/timezone/Europe/Sarajevo");
  while(1){
    vTaskDelay(100);
  }
}

void t2(void *args){
  while(!is_conected){
    
    vTaskDelay(100);
    printf("POST\n");
  }
   rest_post("http://httpbin.org/post");
  while(1){
    vTaskDelay(100);
  }
}



void app_main() {
  printf("Hello, Wokwi!\n");
  wifi_init();
  //vTaskDelay(200);
  xTaskCreatePinnedToCore(t1, "rest_get",  10048,  NULL, 1, &t[0], 1);

  xTaskCreatePinnedToCore(t2, "rest_post", 10048,  NULL, 10, &t[1], 0);
}

```
```c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#define   LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"

#define DELAY(x)  vTaskDelay((x) / portTICK_PERIOD_MS);
#include<stdint.h>
typedef  uint8_t  u8;
typedef  uint16_t u16;
typedef  uint32_t u32;
typedef  uint64_t u64;
typedef  int32_t  i32;
typedef  int64_t  i64;
typedef  float    f32;
typedef  double   f64;


typedef enum{
  WIFI = 0,
  GET, 
  POST,



}State;
#define NUM_OF_STATES 3

volatile u8 _STATE[NUM_OF_STATES];

#define BLOCK_TASK(x, time)  while(!_STATE[x]){DELAY((time));}
#define SET_STATE(x)        _STATE[x] =  true
#define CLEAR_STATE(x)      _STATE[x] =  false
#define GET_STATE(x)        _STATE[x]




static void wifi_event_handler(void *event_handler_arg, 
                                esp_event_base_t event_base, 
                                int32_t event_id, void *event_data){
    switch (event_id){
    case WIFI_EVENT_STA_START:
        {
            printf("Conecting....\n\n\n");
            esp_wifi_connect();
            //DELAY(10);
        }
        
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        CLEAR_STATE(WIFI);
        esp_wifi_connect();
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        printf("\nWiFi got IP ... \n\n");
        SET_STATE(WIFI);
        break;
    default:
        break;
    }
}

 


 static void wifi_connect(){
    
     
    nvs_flash_init();
    esp_netif_init();                    // TCP/IP initiation                   
    esp_event_loop_create_default();     // event loop                          
    esp_netif_create_default_wifi_sta(); // WiFi station                        
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //                                         
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "Wokwi-GUEST",
            .password = ""}};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    
    esp_wifi_connect();
    
    
    
}   


static void setings(){
    esp_log_level_set("*", LOG_LOCAL_LEVEL);
    for(u8 i = 0; i < NUM_OF_STATES; i++){
        CLEAR_STATE(i);
    }  
}

char* data_for_sending;

esp_err_t get_http_handler(esp_http_client_event_handle_t event){

    switch(event->event_id){

        case HTTP_EVENT_ERROR:{break;}

        case HTTP_EVENT_ON_CONNECTED:{break;}

        case HTTP_EVENT_HEADERS_SENT:{break;}

        case HTTP_EVENT_ON_HEADER:{break;}

        case HTTP_EVENT_ON_DATA:{ 
                                 data_for_sending = event->data;
                                 printf("%s\n", data_for_sending); 
                                 SET_STATE(GET); break;} 

        case HTTP_EVENT_ON_FINISH:{break;}

        case HTTP_EVENT_DISCONNECTED:{break;}

        case HTTP_EVENT_REDIRECT:{break;}

        default:{ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);}
    }

    return ESP_OK;
}   


static void get(const char* url){

    esp_http_client_config_t client_config = {
          .url = url,
          .method = HTTP_METHOD_GET,
          .cert_pem = NULL,                          //CREATE CONFIG
          .event_handler = get_http_handler,        //ADD EVENT 

            };
    esp_http_client_handle_t client = esp_http_client_init(&client_config);//INIT CONFIG
    esp_http_client_perform(client);//SEND DATA
    esp_http_client_cleanup(client);//CLEAN DATA



}


esp_err_t post_http_handler(esp_http_client_event_handle_t event){

    switch(event->event_id){

        case HTTP_EVENT_ERROR:{break;}

        case HTTP_EVENT_ON_CONNECTED:{break;}

        case HTTP_EVENT_HEADERS_SENT:{break;}

        case HTTP_EVENT_ON_HEADER:{break;}

        case HTTP_EVENT_ON_DATA:{printf("%s\n", (char *)event->data); 
                                 SET_STATE(POST); break;} 

        case HTTP_EVENT_ON_FINISH:{break;}

        case HTTP_EVENT_DISCONNECTED:{break;}

        case HTTP_EVENT_REDIRECT:{break;}

        default:{ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);}
    }

    return ESP_OK;
}   




static void post(const char* url){
    esp_http_client_config_t config_post = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = post_http_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    //char  *post_data = "test ...";
    esp_http_client_set_post_field(client, data_for_sending, 
    strlen(data_for_sending) - 10);
    //esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);

}




void app_main(void)
{
    setings();
  
    wifi_connect();
    BLOCK_TASK(WIFI, 10);
    
    if(GET_STATE(WIFI)){
        CLEAR_STATE(GET);
        get("http://worldtimeapi.org/api/timezone/Europe/Sarajevo");
        BLOCK_TASK(GET, 10);
        printf("Get\n");
    }
    
    if(GET_STATE(WIFI)){
        CLEAR_STATE(POST);
        post("http://httpbin.org/post");
        BLOCK_TASK(POST, 10);
        printf("Post\n");
    }



    printf("Rest\n");

}

```

## MQQT

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#define  LOCAL_LOG_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"


static const char *TAG = "MQTT_TCP";

//extern const uint8_t mqtt_eclipseprojects_io_pem_start[]   asm("_binary_mqtt_eclipseprojects_io_pem_start");
//extern const uint8_t mqtt_eclipseprojects_io_pem_end[]   asm("_binary_mqtt_eclipseprojects_io_pem_end");

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "Wokwi-GUEST",
            .password = ""}};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, "my_topic", 0);
        esp_mqtt_client_publish(client, "my_topic", "Hi to all from ESP32 .........", 0, 1, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("\nTOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
   // ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.eclipseprojects.io:1883",
        //.cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    nvs_flash_init();
    wifi_connection();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n");

    mqtt_app_start();
}
```
## ESP_HTTP_SERVER.H

`esp_http_server.h` is libary for building of a http sever. Server parse all incoming connections as HTTP trafic.
There are listeners for both `TCP` and `UDP` trafics. `httpd_start(httpd_handler_t, httpd_config_t)` function starts a server. This function
expect server handler(probobly a num) and server_config. `httpd_stop(httpd_handler_t)` stops a server and free all 
asociated memory. For config we mostly use `HTTPD_DEFAULT_CONFIG()`. To register events we will use
`httpd_register_uri(httpd_handler_t, httpd_uri_t)`. `httpd_uri_t` expects `uri` (links), method `HTTPD_POST`, `HTTPD_GET`, `HTTPD_PUT`, handle expects function pointer in format `esp_err_t *handler (httpd_req_t *req)`.
### START_WEBSERVER
```c
 /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);  //POST AND GET HANDLERS MUST BE IPLEMENTED
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
```
### STOP_WEBSERVER
```c
   if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
```
#### Peristent conections
Inside this component we have ability to have permanent connections(`Sesions`). For menaging of a sesions we will use a context provaided in handler. Context is always the same or bether to be said we will nead to manage memory and all other stuff with it.
```c
/* Custom function to free context */
void free_ctx_func(void *ctx)
{
    /* Could be something other than free */
    free(ctx);
}

esp_err_t adder_post_handler(httpd_req_t *req)
{
    /* Create session's context if not already available */
    if (! req->sess_ctx) {
        req->sess_ctx = malloc(sizeof(ANY_DATA_TYPE));  /*!< Pointer to context data */
        req->free_ctx = free_ctx_func;                  /*!< Function to free context data */
    }

    /* Access context data */
    ANY_DATA_TYPE *ctx_data = (ANY_DATA_TYPE *)req->sess_ctx;

    /* Respond */
    ...............
    ...............
    ...............

    return ESP_OK;
}
```
For futher studies look into a 
https://github.com/espressif/esp-idf/blob/v5.3.1/examples/protocols/http_server/persistent_sockets/main/main.c

As for all componets we coude add a event listener for handing events witch are hapening with a server.
`esp_err_t esp_event_handler_register(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void *event_handler_arg)` registers the event.
This is a list of a events witch coude hapend: 
1. `HTTP_SERVER_EVENT_ERROR`

2. `HTTP_SERVER_EVENT_START`

3. `HTTP_SERVER_EVENT_ON_CONNECTED`

4. `HTTP_SERVER_EVENT_ON_HEADER`

5. `HTTP_SERVER_EVENT_HEADERS_SENT`

6. `HTTP_SERVER_EVENT_ON_DATA`

7. `HTTP_SERVER_EVENT_SENT_DATA`

8. `HTTP_SERVER_EVENT_DISCONNECTED`

9. `HTTP_SERVER_EVENT_STOP`

### Functions
1. `esp_err_t httpd_req_async_handler_begin(httpd_req_t *r, httpd_req_t **out)` -> Start an asynchronous request.
2. `esp_err_t httpd_req_async_handler_complete(httpd_req_t *r)` -> Marks async request as finished.
3. IM `int httpd_req_recv(httpd_req_t *r, char *buf, size_t buf_len)` -> Used to parse and read req to bufer.
4. IM `esp_err_t httpd_resp_send(httpd_req_t *r, const char *buf, ssize_t buf_len)` -> Used to send bufer to req.
5. `static inline esp_err_t httpd_resp_send_404(httpd_req_t *r)` -> Used to send a special responses (404 chenge).

```C
#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "my_data.h"

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/", // Was changed .................
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/",  // Was changed ................
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();
    start_webserver();
}
```

```c
#include "esp_http_server.h"



static esp_err_t get(httpd_req_t *req){
	  char *response_message = "<!DOCTYPE HTML><html><head>\
                                <title>Static HTML page</title>\
                                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
                                </head><body>\
                                <h1>Ako ovo ne radi mali najebo si!!!</h1>\
                                </form><br>\
                                </body></html>";
    httpd_resp_send(req, response_message, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}



void server_init(){
	httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
	httpd_handle_t server_handle = NULL;
    httpd_start(&server_handle, &server_config);

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get,
        .user_ctx = NULL};
    httpd_register_uri_handler(server_handle, &uri_get);

}

```

## ESP32_ETHERNET.H

Ethernet is an asynchronous Carrier Sense Multiple Access with Collision Detect (CSMA/CD) protocol/interface. 
Normal IEEE 802.3 compliant Ethernet frames are between 64 and 1518 bytes in length. They are made up of five or six different fields: a destination MAC address (DA), a source MAC address (SA), a type/length field, a data payload, an optional padding field and a Cyclic Redundancy Check (CRC). Additionally, when transmitted on the Ethernet medium, a 7-byte preamble field and Start-of-Frame (SOF) delimiter byte are appended to the beginning of the Ethernet packet.

Thus the traffic on the twist-pair cabling.

Direction of packets is controled with a unique adress(MAC) 6 bytes.
There are two types adress (destination address or source address). 

### Ethernet driver

The Ethernet driver is composed of two parts: MAC and PHY.

The communication between MAC and PHY can have diverse choices: MII (Media Independent Interface), RMII (Reduced Media Independent Interface).
ESP-IDF only supports the RMII interface. Also external etherent chips over spi are suported.

#### Internal EMAC + External PHY
```c
eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();                      // apply default common MAC configuration
eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG(); // apply default vendor-specific MAC configuration
esp32_emac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;            // alter the GPIO used for MDC signal
esp32_emac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;          // alter the GPIO used for MDIO signal
esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config); // create MAC instance

eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();      // apply default PHY configuration
phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;           // alter the PHY address according to your board design
phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO; // alter the GPIO used for PHY reset
esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);     // create PHY instance
// ESP-IDF officially supports several different Ethernet PHY chip driver
// esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
// esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
// esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);

```

#### SPI-Ethernet Module

```c
eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();      // apply default common MAC configuration
eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();      // apply default PHY configuration
phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;           // alter the PHY address according to your board design
phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO; // alter the GPIO used for PHY reset
// Install GPIO interrupt service (as the SPI-Ethernet module is interrupt-driven)
gpio_install_isr_service(0);
// SPI bus configuration
spi_device_handle_t spi_handle = NULL;
spi_bus_config_t buscfg = {
    .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
    .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
    .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
};
ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, 1));
// Configure SPI device
spi_device_interface_config_t spi_devcfg = {
    .mode = 0,
    .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
    .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
    .queue_size = 20
};
/* dm9051 ethernet driver is based on spi driver */
eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(CONFIG_EXAMPLE_ETH_SPI_HOST, &spi_devcfg);
dm9051_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
esp_eth_mac_t *mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
esp_eth_phy_t *phy = esp_eth_phy_new_dm9051(&phy_config);


```

Ethernet component also has event handler.

### ETHERNET CONECTION USING ENC28J60
For conecting this type of ethernet chip we will use component in examples(ethernet/enc28j60).
```c
//INCLUDES
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_eth_enc28j60.h"
#include "driver/spi_master.h"
//

//ETHERNET DEFINES

//HEAR WE WILL CONFIGURE STUFF 
#define MISO_GPIO 12
#define MOSI_GPIO 13
#define CLOCK_GPIO 14
#define CHIPSELECT_GPIO 15
#define INTERUPT_GPIO 4
#define SPI_HOST_NUM 1
#define SPI_CLOCK_FREQUENCY 8
#define DUPLEX_MODE 1
static const char* TAG = "Ethernet";
//** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

static void ethernet_init(){
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    spi_bus_config_t buscfg = {
        .miso_io_num = MISO_GPIO,
        .mosi_io_num = MOSI_GPIO,
        .sclk_io_num = CLOCK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST_NUM, &buscfg, SPI_DMA_CH_AUTO));
    /* ENC28J60 ethernet driver is based on spi driver */
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = SPI_CLOCK_FREQUENCY * 1000 * 1000,
        .spics_io_num = CHIPSELECT_GPIO,
        .queue_size = 20,
        .cs_ena_posttrans = 210,
    };

    eth_enc28j60_config_t enc28j60_config = ETH_ENC28J60_DEFAULT_CONFIG(SPI_HOST_NUM, &spi_devcfg);
    enc28j60_config.int_gpio_num = INTERUPT_GPIO;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_enc28j60(&enc28j60_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.autonego_timeout_ms = 0; // ENC28J60 doesn't support auto-negotiation
    phy_config.reset_gpio_num = -1; // ENC28J60 doesn't have a pin to reset internal PHY
    esp_eth_phy_t *phy = esp_eth_phy_new_enc28j60(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

    /* ENC28J60 doesn't burn any factory MAC address, we need to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
    mac->set_addr(mac, (uint8_t[]) {
        0x02, 0x00, 0x00, 0x12, 0x34, 0x56
    });

    // ENC28J60 Errata #1 check
    if (emac_enc28j60_get_chip_info(mac) < ENC28J60_REV_B5 && SPI_CLOCK_FREQUENCY < 8) {
        ESP_LOGE(TAG, "SPI frequency must be at least 8 MHz for chip revision less than 5");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* It is recommended to use ENC28J60 in Full Duplex mode since multiple errata exist to the Half Duplex mode */
#if DUPLEX_MODE
    eth_duplex_t duplex = ETH_DUPLEX_FULL;
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex));
#endif

    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}



void app_main(void){
    ethernet_init();
    while(1){
        vTaskDelay(100);
    }
}
```

## MDNS

Multicast DNS (mDNS) is a computer networking protocol that resolves hostnames to IP addresses within small networks that do not include a local name server. It is a zero-configuration service, using essentially the same programming interfaces, packet formats and operating semantics as unicast Domain Name System (DNS). It was designed to work as either a stand-alone protocol or compatible with standard DNS servers.

#### Working princip

When an mDNS client needs to resolve a hostname, it sends an IP multicast query message that asks the host having that name to identify itself. That target machine then multicasts a message that includes its IP address. All machines in that subnet can then use that information to update their mDNS caches. Any host can relinquish its claim to a name by sending a response packet with a time to live (TTL) equal to zero.
https://en.wikipedia.org/wiki/Multicast_DNS

#### How to used it on esp32 

Esp-idf does not contain mDNS implementation. To get mDNS component we nead to clone component from espressif by using comand `idf.py add-dependency espressif/mdns` and than to reconfigure entaire config using `idf.py reconfigure`

For this compnent to be used network stack must be initialized.

This is init of a mdns 
```c
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#define HOSTNAME "nesto/"
#define MDNS_INSTANCE "mDNS instance"

static void mdns_config(){

    mdns_init();   //Initalize
    mdns_hostname_set(HOSTNAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));

}

```

After we configure a component we will nead to use a netbios interface to resolve uniqe names  
```c
void app_main(){
    //ethernet_init();
    wifi_init();
    mdns_config();
    netbiosns_init();
    netbiosns_set_name(HOSTNAME);
   
}

```
## Bluetooth Low Energy (BLE)


The Bluetooth system can be divided into two different categories: Classic Bluetooth and Bluetooth Low
Energy (BLE). ESP32 supports dual-mode Bluetooth, meaning that both Classic Bluetooth
and Bluetooth LE are supported by ESP32. Clasical bluethooth is a striming protocol hance it consumes more energy 
then it counterpart Bluetooth Low Energy(BLE). Esp32 bluethooth Controller has integrated a variety of functions, including H4 protocol, HCI, LinkManager, Link Controller, Device Manager, and HW Interface.


Bluetooth Low Energy (BLE): Bluetooth Low Energy is a wireless communication protocol designed for energy-efficient data transmission. It’s ideal for IoT devices, wearables, and applications where power consumption is critical.
On esp32 BLE stack for use is nible bluethooth stack.

### GATT

The ATT specifies the minimum data storage unit in the Bluetooth LE architecture, while the
GATT defines how to represent a data set using attribute values and descriptors, how to
aggregate similar data into a service, and how to find out what services and data a peer
device owns.
The GATT introduces the concept of Characteristics, which are about information that is
not purely numerical, as in the cases outlined below:

• The unit of a given value, for example, weight measured in kilograms (kg),
temperature measured in Celsius (℃), and so on.

• The name of a given value. For example, for characteristics with the same UUID, e.g.
temperature attribute, the name of the value informs the peer device that this value
indicates “the temperature in the master bedroom”, while the other one indicates “the
temperature in the living room”.

• The exponent of excessive data numbers, such as 230,000 and 460,000. Given that
the exponent is already specified as 10^4, transmitting only “23” and “46” is enough
to represent 230,000 and 460,000.
These are just a few examples of the many existing requirements for describing data
accurately in actual applications. In order to provide more nuanced information, a large
piece of data space should be reserved to store this additional information in each
characteristic. However, in many cases, most of the extra space reserved will not be used.
Such a design, then, will not comply with BLE's prerequisite to have as concise as possible
protocols. In cases like this, the GATT specification introduces the concept of descriptors
to outline this additional information. It must be noted that each piece of data and
descriptor do not have a one-to-one correspondence, that is, complex data can have
multiple descriptors, while simple data can have no descriptors at all.


### Generic Access Profile (Gap)

The GAP (the Generic Access Profile) defines the discovery process, device
management and the establishment of device connection between Bluetooth LE devices.
The Bluetooth LE GAP is implemented in the form of API calls and Event returns. The
processing result of API calls in the protocol stack is returned by Events. When a peer
device initiates a request, the status of that peer device is also returned by an Event.
There are four GAP roles defined for a Bluetooth LE device:

• Broadcaster: A broadcaster is a device that sends advertising packets, so it can be
discovered by the observers. This device can only advertise, but cannot be
connected.

• Observer: An observer is a device that scans for broadcasters and reports this
information to an application. This device can only send scan requests, but cannot be
connected.

• Peripheral: A peripheral is a device that advertises by using connectable advertising
packets and becomes a slave once it gets connected.

• Central: A central is a device that initiates connections to peripherals and becomes a
master once a physical link is established.

### Nimble stack
As we already sad nible stack is a ble stack used on esp32.
To get this stack to work we first nead to include next headers
```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
```
Also we nead to setup type of a ble stack(nimble stack) in menuconfig. It is under the compononent category.

This stack depends on nvs flash (like most of periferals probably some basic info are saved there) so we nead to init it ` nvs_flash_init()`.

Next step is only done on older version of a esp32. This step `esp_nimble_hci_and_controller_init()` 
is moved in nimble_port_init().
This step initilaise a comtroler on esp32 witch act kinda like a bridge betwean chip and esp32. I mean 
it setup all of them you get the point. 

Next step inits the host stack `nimble_port_init()`.

In next 3 steps we add a name for our ble device(How we will find it on a network), and also init a gat, gap service.
```c

ble_svc_gap_device_name_set("BLE_asdasd"); // 4 - Initialize NimBLE configuration - server name
ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service

```
Next steps depends on this struct `static const struct ble_gatt_svc_def `.
This struct is array of pointers to other service definitions. Example of a structure look like 
```c
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
		{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
		.characteristics = (struct ble_gatt_chr_def[]) {
				{
				.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
				.flags = BLE_GATT_CHR_F_READ,
				.access_cb = device_read
				}, {
				.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
				.flags = BLE_GATT_CHR_F_WRITE,
				.access_cb = device_write
				},
				{0}
			}
		},
		{0}
	};
```
As we can see above we have a one device whitch is setup to be a primary one `.type = BLE_GATT_SVC_TYPE_PRIMARY,`.
There are 2 types of uuid: 
	1.  16 bit one -> This is addresed by a standard `.uuid = BLE_UUID16_DECLARE(0xFEF4)`
	2.  Arbitary lenght -> Not addresed by a standard 

 Characteristics are array of ptr of a struct `struct ble_gatt_chr_def[]`
 Flags sets up are we reading or writing `BLE_GATT_CHR_F_WRITE, BLE_GATT_CHR_F_READ`.
 Access_cb is a callback for what we runing if above event (we coude call it like that) trigered.
 Examples of device_write and device_read callbacks :

 #### deveice_read: 

```c
// Read data from ESP32 defined as server
static int device_read(uint16_t con_handle, uint16_t attr_handle, 
			struct ble_gatt_access_ctxt *ctxt, void *arg) {
	os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
	return 0;
	}
```

#### device_write: 

```c
static int device_write(uint16_t conn_handle, uint16_t attr_handle,
			struct ble_gatt_access_ctxt *ctxt, void *arg) {
         printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
	return 0;
	}
```

We register our struct

```c
	ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
	ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
```

For our app to run we nead to have some way to synchronise the to devices aka to now to what to send a packets
aka data. For that easy way is to have a globaly declared `uint8_t ble_addr_type;` and use this type of
function to gets as a addres type.
```c
// Define the BLE connection
void ble_app_advertise(void) {
	// GAP - device name definition
	struct ble_hs_adv_fields fields;
	const char *device_name;
	memset(&fields, 0, sizeof(fields));
	device_name = ble_svc_gap_device_name(); // Read the BLE device name
	fields.name = (uint8_t *)device_name;
	fields.name_len = strlen(device_name);
	fields.name_is_complete = 1;
	ble_gap_adv_set_fields(&fields);

	// GAP - device connectivity definition
	struct ble_gap_adv_params adv_params;
	memset(&adv_params, 0, sizeof(adv_params));
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
	ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
	}

// The application
void ble_app_on_sync(void) {
	ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
	ble_app_advertise();                     // Define the BLE connection
	}

```
in app_main() we call: 
`ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application`

Hear we have also a advertaysing function whitch will set up gap parameters. 
And publish data. 
Last step is to run the event loop(thread):
```c
void host_task(void *param) {
	nimble_port_run(); // This function will return only when nimble_port_stop() is executed
	}
//In main
 	nimble_port_freertos_init(host_task);
```
ALL HANDLING EXAMPLE: 
```c
//TAG.H
#ifndef _BLENIMBLE_H
#define _BLENIMBLE_H
#include "defs.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"

#define TAG "BLE-Nesto";
uint8_t ble_addr_type;
void ble_app_advertise(void);

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt *ctxt, void *arg) {
	// printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);

	char * data = (char *)ctxt->om->om_data;
	printf("%d\n",strcmp(data, (char *)"LIGHT ON")==0);
	if (strcmp(data, (char *)"LIGHT ON\0")==0) {
		printf("LIGHT ON\n");
		}
	else if (strcmp(data, (char *)"LIGHT OFF\0")==0) {
		printf("LIGHT OFF\n");
		}
	else if (strcmp(data, (char *)"FAN ON\0")==0) {
		printf("FAN ON\n");
		}
	else if (strcmp(data, (char *)"FAN OFF\0")==0) {
		printf("FAN OFF\n");
		}
	else {
		printf("Data from the client: %.*s\n", ctxt->om->om_len, ctxt->om->om_data);
		}


	return 0;
	}

// Read data from ESP32 defined as server
static int device_read(uint16_t con_handle, uint16_t attr_handle, 
											 struct ble_gatt_access_ctxt *ctxt, void *arg) {
	os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
	return 0;
	}

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
		{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
		.characteristics = (struct ble_gatt_chr_def[]) {
				{
				.uuid = BLE_UUID16_DECLARE(0xFEF4),           // Define UUID for reading
				.flags = BLE_GATT_CHR_F_READ,
				.access_cb = device_read
				}, {
				.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
				.flags = BLE_GATT_CHR_F_WRITE,
				.access_cb = device_write
				},
				{0}
			}
		},
		{0}
	};

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg) {
	switch (event->type) {
		// Advertise if connected
		case BLE_GAP_EVENT_CONNECT:
			ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
			if (event->connect.status != 0) {
				ble_app_advertise();
				}
			break;
		// Advertise again after completion of the event
		case BLE_GAP_EVENT_DISCONNECT:
			ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
			break;
		case BLE_GAP_EVENT_ADV_COMPLETE:
			ESP_LOGI("GAP", "BLE GAP EVENT");
			ble_app_advertise();
			break;
		default:
			break;
		}
	return 0;
	}

// Define the BLE connection
void ble_app_advertise(void) {
	// GAP - device name definition
	struct ble_hs_adv_fields fields;
	const char *device_name;
	memset(&fields, 0, sizeof(fields));
	device_name = ble_svc_gap_device_name(); // Read the BLE device name
	fields.name = (uint8_t *)device_name;
	fields.name_len = strlen(device_name);
	fields.name_is_complete = 1;
	ble_gap_adv_set_fields(&fields);

	// GAP - device connectivity definition
	struct ble_gap_adv_params adv_params;
	memset(&adv_params, 0, sizeof(adv_params));
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
	ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
	}

// The application
void ble_app_on_sync(void) {
	ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
	ble_app_advertise();                     // Define the BLE connection
	}

// The infinite task
void host_task(void *param) {
	nimble_port_run(); // This function will return only when nimble_port_stop() is executed
	}

#endif
```
```c
//MAIN
#include "tag.h"
void app_main()
{
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}
```
### iBeacon scaner
```c

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

#define TAG "ble"


#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

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



void init_nvs(void) {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
		}
	ESP_ERROR_CHECK(ret);
	}

void init_ble(void) {
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

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
	switch (event) {
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			if (param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS) {
				ESP_LOGI(TAG, "Scan parameters set, starting scan...");
				esp_ble_gap_start_scanning(10);  // Scan for 10 seconds
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

					printf("\tMajor: 0x%04x (%d)\n", major, major);
					printf("\tMinor: 0x%04x (%d)\n", minor, minor);
					printf("\tMeasured power (RSSI at a 1m distance): %d dBm\n", ibeacon_data->ibeacon_vendor.measured_power);
					printf("\tRSSI of packet: %d dbm\n", param->scan_rst.rssi);
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


void app_main(void) {
	// Initialize NVS
	init_nvs();

	// Initialize BLE
	init_ble();

	// Register GAP callback
	esp_ble_gap_register_callback(gap_event_handler);

	// Set scan parameters
	esp_err_t ret = esp_ble_gap_set_scan_params(&ble_scan_params);
	if (ret == ESP_OK) {
		ESP_LOGI(TAG, "BLE scan parameters set successfully");
		}
	else {
		ESP_LOGE(TAG, "Failed to set scan params: %s", esp_err_to_name(ret));
		}

	while(1) {
		esp_ble_gap_start_scanning(1000);
		vTaskDelay(10000);
		esp_ble_gap_stop_scanning();
		vTaskDelay(1000);

		}
	}



```
