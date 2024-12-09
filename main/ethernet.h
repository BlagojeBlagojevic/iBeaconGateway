#ifndef ETHERNET_H
#define ETHERNET_H


//
#include "defs.h"

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

//DEFAULT in ns 
#define DELAY_TIME_WIRE 210 

//Change this for prod default is not factory
#define MAC_ADDR 0x02, 0x00, 0x00, 0x12, 0x34, 0x56


static const char* TAG = "Ethernet";
static esp_netif_t *eth_netif;

//CONFIGURATION OF A STATIC IP HEAR OR IN MAIN
static char* STATIC_IP = "10.119.006.002";
static char* STATIC_MASK =  "255.255.255.240";
static char* STATIC_GATWAY =  "10.119.006.001";
static char* STATIC_DNS =  "000.000.000.000";
#define STATIC_IPa
//EXTERN FUNCTIONS
extern void ethernet_init();


#ifdef ETHERNET_IMPLEMENTATNION

#ifdef STATIC_IPa
static void _set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE("asd", "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(STATIC_IP);
    ip.netmask.addr = ipaddr_addr(STATIC_MASK);
    ip.gw.addr = ipaddr_addr(STATIC_GATWAY);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE("asd", "Failed to set ip info");
        return;
    }
    //dali treba
    esp_netif_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = ipaddr_addr(STATIC_DNS);
    dns.ip.type = IPADDR_TYPE_V4;
    esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns);
    
}
#endif



static void _eth_event_handler(void *arg, esp_event_base_t event_base,
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
        #ifdef STATIC_IPa
          _set_static_ip(eth_netif);
        #endif
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        CLEAR_STATE(ETHERNET);
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        CLEAR_STATE(ETHERNET);
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        CLEAR_STATE(ETHERNET);
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void _got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    //esp_netif_dhcpc_start(eth_netif);
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    SET_STATE(ETHERNET);
}

extern void ethernet_init(){
    
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);

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
        .cs_ena_posttrans = DELAY_TIME_WIRE,
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
        MAC_ADDR
    });

    // ENC28J60 Errata #1 check
    if (emac_enc28j60_get_chip_info(mac) < ENC28J60_REV_B5 && SPI_CLOCK_FREQUENCY < 8) {
        ESP_LOGE(TAG, "SPI frequency must be at least 8 MHz for chip revision less than 5");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &_eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &_got_ip_event_handler, NULL));

    /* It is recommended to use ENC28J60 in Full Duplex mode since multiple errata exist to the Half Duplex mode */
#if DUPLEX_MODE
    eth_duplex_t duplex = ETH_DUPLEX_FULL;
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex));
#endif

    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

#endif
#endif
