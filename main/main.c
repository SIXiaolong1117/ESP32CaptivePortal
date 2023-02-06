#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <netdb.h>
#include <sys/socket.h>
#include "driver/uart.h"

#include "dns_server.h"
#include "web_server.h"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static const char *TAG = "Main";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        // esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
        auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        ESP_LOGI(TAG, "a phone connected!");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "A Phone disconnect  ...");

    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = "Captive Portal",
            .ssid_len = strlen("Captive Portal"),
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 3,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_LOGI(TAG, "Setting WiFi softAP SSID %s...", wifi_ap_config.ap.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();  // 初始化WiFi
    dns_server_start(); // 开启DNS服务器
    web_server_start(); // 开启TCP服务器
}