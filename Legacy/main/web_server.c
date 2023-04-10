#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "web_server.h"

#define BUFF_SIZE 1024

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
static const char *TAG = "WEB_SERVER";
static const char *HTTP_200 = "HTTP/1.1 200 OK\r\n"
                              "Server: lwIP/1.4.0\r\n"
                              "Content-Type: text/html\r\n"
                              "Connection: Keep-Alive\r\n"
                              "Content-Length: %d \r\n\r\n";
static const char *HTTP_400 = "HTTP/1.0 400 BadRequest\r\n"
                              "Content-Length: 0\r\n"
                              "Connection: Close\r\n"
                              "Server: lwIP/1.4.0\r\n\n";

int my_write(int fd, void *buffer, int length)
{
    int bytes_left;
    int written_bytes;
    char *ptr;

    ptr = buffer;
    bytes_left = length;
    while (bytes_left > 0)
    {
        written_bytes = send(fd, ptr, bytes_left, 0);
        if (written_bytes <= 0)
        {
            if (errno == EINTR)
                written_bytes = 0;
            else
                return (-1);
        }
        bytes_left -= written_bytes;
        ptr += written_bytes;
        vTaskDelay(10);
    }
    return (0);
}

void handle_http_request(void *pvParameters)
{
    char buff[BUFF_SIZE] = {0};
    int length = 0;

    int fd = *(int *)pvParameters;
    int bytes_recvd = 0;
    char *uri = NULL;

    ESP_LOGI(TAG, "Http Sub Task Run with socket: %d", fd);

    vTaskDelay(30);

    // 读取HTTP请求头
    bytes_recvd = recv(fd, buff, BUFF_SIZE - 1, 0);

    if (bytes_recvd <= 0)
    {
        ESP_LOGE(TAG, "Recv requst header error!");
        goto requst_error;
    }

    // 解析请求类型及请求URI
    uri = strstr(buff, "HTTP");
    if (uri == NULL)
    {
        ESP_LOGE(TAG, "Parase requst header error!");
        goto requst_error;
    }
    uri[0] = 0;
    uri = NULL;

    uri = strstr(buff, " ");
    if (uri == NULL)
    {
        ESP_LOGE(TAG, "Parase requst uri error!");
        goto requst_error;
    }
    uri[0] = 0;
    uri++;

    ESP_LOGI(TAG, "the reqqust type is %s, uri is: %s", buff, uri);

    // 响应GET请求
    if (strcmp(buff, "GET") == 0)
    {
        length = sprintf(buff, HTTP_200, index_html_end - index_html_start);
        my_write(fd, buff, length);
        my_write(fd, index_html_start, index_html_end - index_html_start);
    }
    else
    {
        my_write(fd, HTTP_400, strlen(HTTP_400));
    }

    vTaskDelay(30);

requst_error:
    ESP_LOGI(TAG, "close socket %d", fd);
    close(fd);
    vTaskDelete(NULL);
}

void web_server(void *pvParameters)
{
    // socket句柄和建立连接后的句柄
    int sockfd, new_fd;
    // 本方地址信息结构体，下面有具体的属性赋值
    struct sockaddr_in my_addr;
    // 对方地址信息
    struct sockaddr_in their_addr;
    socklen_t sin_size;

    // 发送接收超时时间
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    sin_size = sizeof(struct sockaddr_in);
    // 建立socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        ESP_LOGE(TAG, "socket failed:%d", errno);
        goto web_err;
        ;
    }
    // 本方地址信息结构体具体属性赋值
    // 该属性表示接收本机或其他机器传输
    my_addr.sin_family = AF_INET;
    // 端口号
    my_addr.sin_port = htons(80);
    // IP，括号内容表示本机IP
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 将其他属性置0
    bzero(&(my_addr.sin_zero), 8);

    // 绑定地址结构体和socket
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)
    {
        ESP_LOGE(TAG, "bind error");
        goto web_err;
    }

    // 开启监听 ，第二个参数是最大监听数
    listen(sockfd, 8);
    ESP_LOGI(TAG, "web_server start...");
    while (1)
    {
        // 在这里阻塞知道接收到消息，参数分别是socket句柄，接收到的地址信息以及大小
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            ESP_LOGE(TAG, "accept failed");
        }
        else
        {
            ESP_LOGI(TAG, "Accept new socket: %d", new_fd);

            setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
            setsockopt(new_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

            int *para_fd = malloc(sizeof(int));
            *para_fd = new_fd;
            xTaskCreate(&handle_http_request, "socket_task", 1024 * 3, para_fd, 6, NULL);
        }
        vTaskDelay(10);
    }

web_err:
    vTaskDelete(NULL);
}

void web_server_start(void)
{
    xTaskCreate(&web_server, "web_server_task", 2048, NULL, 5, NULL);
}