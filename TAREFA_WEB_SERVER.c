#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "lib/buzzer.h"

// DEFINIÇÕES DE PINOS E WI-FI
#define LED_PIN 12           // LED da "casa" (luz a ser controlada)
#define BUTTON1_PIN 5        // Botão A
#define BUTTON2_PIN 6        // Botão B

#define WIFI_SSID "Ferreira-1"
#define WIFI_PASS "vetoresespacoxyz"

// Mensagens que mostram o estado dos botões
char button1_message[50] = "Nenhum evento no Botão 1";
char button2_message[50] = "Nenhum evento no Botão 2";

// Buffer para a resposta da página HTML
char http_response[1024];

// Função para criar o conteúdo da página HTML
void create_http_response() {
    snprintf(http_response, sizeof(http_response),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>Casa Inteligente</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; background-color: #f0f8ff; text-align: center; padding: 40px; color: #333; }"
        "h1 { color: #0044cc; }"
        "a { display: inline-block; margin: 10px; padding: 10px 20px; background-color: #008CBA; color: white; text-decoration: none; border-radius: 5px; font-size: 16px; }"
        "a:hover { background-color: #005f73; }"
        "p { font-size: 18px; }"
        "</style>"
        "</head>"
        "<body>"
        "<h1>Controle de Iluminação</h1>"
        "<a href=\"/led/on\">Ligar Luz</a>"
        "<a href=\"/led/off\">Desligar Luz</a>"
        "<a href=\"/update\">Atualizar Estado</a>"
        "<h2>Status dos Botões:</h2>"
        "<p>Botão A: %s</p>"
        "<p>Botão B: %s</p>"
        "</body>"
        "</html>\r\n",
        button1_message, button2_message);
}

// Callback que processa a requisição HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *request = (char *)p->payload;

    // Processa comandos recebidos
    if (strstr(request, "GET /led/on")) {
        gpio_put(LED_PIN, 1);                 // Liga LED
        buzzer_play_tone(700, 500);          // Bip curto
    } else if (strstr(request, "GET /led/off")) {
        gpio_put(LED_PIN, 0);                 // Desliga LED
        buzzer_play_tone(700, 500);          // Bip curto
    }

    // Atualiza a página com o estado atual
    create_http_response();
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);

    pbuf_free(p);
    return ERR_OK;
}

// Callback quando uma conexão chega
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);
    return ERR_OK;
}

// Inicia o servidor Web (HTTP na porta 80)
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);

    printf("Servidor HTTP rodando na porta 80...\n");
}

// Função para monitorar o estado dos botões
void monitor_buttons() {
    static bool button1_last_state = false;
    static bool button2_last_state = false;

    bool button1_state = !gpio_get(BUTTON1_PIN); // Botão A pressionado = LOW
    bool button2_state = !gpio_get(BUTTON2_PIN); // Botão B pressionado = LOW

    if (button1_state != button1_last_state) {
        button1_last_state = button1_state;
        if (button1_state) {
            snprintf(button1_message, sizeof(button1_message), "Botão A pressionado!");
        } else {
            snprintf(button1_message, sizeof(button1_message), "Botão A solto!");
        }
    }

    if (button2_state != button2_last_state) {
        button2_last_state = button2_state;
        if (button2_state) {
            snprintf(button2_message, sizeof(button2_message), "Botão B pressionado!");
        } else {
            snprintf(button2_message, sizeof(button2_message), "Botão B solto!");
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(5000);  // Pequeno atraso para garantir que a USB estabilize
    printf("Iniciando sistema Smart Home...\n");

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar Wi-Fi\n");
        return 1;
    } else {
        printf("Wi-Fi conectado!\n");
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP: %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    // Configura LED, Botões e Buzzer
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(BUTTON1_PIN);
    gpio_set_dir(BUTTON1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON1_PIN);

    gpio_init(BUTTON2_PIN);
    gpio_set_dir(BUTTON2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON2_PIN);

    buzzer_init(); // Inicializa buzzer

    printf("LED, Botões e Buzzer configurados\n");

    // Inicia o Webserver
    start_http_server();

    // Loop principal
    while (true) {
        cyw43_arch_poll();   // Mantém Wi-Fi estável
        monitor_buttons();   // Atualiza estado dos botões
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}
