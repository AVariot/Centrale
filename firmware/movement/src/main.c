#include "driver/gpio.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sockets.h"

#include "network.h"
#include <lwip/def.h>

#define PIN_CAPTEUR GPIO_NUM_4

void init_wifi() {
    // Non-Volatile Storage
    esp_err_t ret = nvs_flash_init();
    // If corrupted or full, we reset the storage
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Start of the sub system LwIP network
    /*
        LwIP = Lightweight IP, open source networking protocol stack for embedded system
    */
    ESP_ERROR_CHECK(esp_netif_init());
    // Event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // sta for station
    /*
        Sta = the module will act like a smartphone ; listening around it
        AP = Access point = act like a modem
    */
    esp_netif_create_default_wifi_sta();
    // Initiate the wifi hardware
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
        },
    };
    // Start Wifi    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Connexion
    ESP_ERROR_CHECK(esp_wifi_connect()); 
}

int init_socket() {
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(9000);
    
    // Convert the ip in binary
    inet_pton(AF_INET, IP_SERVEUR, &dest_addr.sin_addr);

    // Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("TCP", "Impossible d'allouer la socket");
        return -1;
    }

    // Connect to the server
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE("TCP", "Serveur injoignable (Erreur %d)", errno);
        close(sock);
        return -1;
    }

    return sock;
}

void send_tcp(int sock, char *message) {
    int bytes_send = send(sock, message, strlen(message) + 1, 0);
    if (bytes_send < 0) {
        ESP_LOGE("TCP", "Erreur lors de l'envoi");
    }
}

void init_capteur() {
    // Remise à zéro de la broche
    gpio_reset_pin(PIN_CAPTEUR);
    // Configuration en entrée (Input)
    gpio_set_direction(PIN_CAPTEUR, GPIO_MODE_INPUT);
    // Activation de la résistance pull-down pour fixer l'état à 0 au repos
    // gpio_set_pull_mode(PIN_CAPTEUR, GPIO_PULLDOWN_ONLY);
    
    ESP_LOGI("CAPTEUR", "Pin 4 initialise en mode numerique avec succes !");
}

void app_main(void) {
    init_wifi();
    init_capteur(); // Pin 4
    

    ESP_LOGI("MAIN", "Attente IP...");
    vTaskDelay(5000 / portTICK_PERIOD_MS); 

    int sock = init_socket();
    
    if (sock >= 0) {
        ESP_LOGI("MAIN", "Tuyau ouvert avec le serveur !");
        send_tcp(sock, "MOVEMENT 001\n");
    }

    int last_state = 0;

    while (1) {
        // --- Handle TCP reconnexion ---
        if (sock < 0) {
            sock = init_socket();
            if (sock < 0) {
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                continue;
            }
        }

        int current_state = gpio_get_level(PIN_CAPTEUR);

        if (current_state == 1 && last_state == 0) {
            ESP_LOGW("CAPTEUR", "Mouvement detecte sur le Pin 4 !");
            send_tcp(sock, "MOVEMENT 001\n");
        }
        
        last_state = current_state;

        static int ping = 0;
        ping++;
        if (ping >= 30) {
            int envoye = send(sock, "PING\n", 5, 0);
            if (envoye < 0) {
                ESP_LOGE("MAIN", "Le serveur a coupe la connexion !");
                close(sock);
                sock = -1;
            } else {
                ESP_LOGI("MAIN", "Ping envoye, le tuyau est toujours ouvert.");
            }
            ping = 0;
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
