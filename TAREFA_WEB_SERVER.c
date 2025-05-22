#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "lib/buzzer.h"
#include "lib/led_matrix.h"
#include "lib/ssd1306.h"

#define BUTTON1_PIN 5
#define BUTTON2_PIN 6
#define WS2812_PIN 7
#define WIFI_SSID "Ferreira-1"
#define WIFI_PASS "vetoresespacoxyz"

// Configurações do display OLED
#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_ADDR      0x3C
#define OLED_I2C_PORT  i2c1
#define OLED_SDA_PIN   14
#define OLED_SCL_PIN   15
#define OLED_I2C_FREQ  400000

char button1_message[50] = "Nenhum evento no Botão 1";
char button2_message[50] = "Nenhum evento no Botão 2";
char http_response[2048];

bool luz_ligada = false;
int intensidade_luz = 100; // 100% ou 50%
volatile bool atualizar_display = true;
bool matriz_inicializada = false;

void atualizar_oled(bool wifi_conectado) {
    ssd1306_clear();
    ssd1306_draw_string(0, 0, luz_ligada ? "Luz: Ligada" : "Luz: Desligada");
    ssd1306_draw_string(0, 16, wifi_conectado ? "Wi-Fi: Conectado" : "Wi-Fi: Erro");
    ssd1306_show();
}

void matriz_led_ligar(int intensidade) {
    if (!matriz_inicializada) {
        npInit(WS2812_PIN);
        npClear();
        npWrite();
        matriz_inicializada = true;
    }

    FRAME1(); // Pode ajustar conforme necessidade para simular intensidade
    npWrite();
}

void matriz_led_desligar() {
    if (matriz_inicializada) {
        FRAME2(); // LEDs apagados
        npWrite();
    }
}

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
        "<a href=\"/intensidade\">Alternar Intensidade</a>"
        "<a href=\"/update\">Atualizar Estado</a>"
        "<h2>Status dos Botões:</h2>"
        "<p>Botão A: %s</p>"
        "<p>Botão B: %s</p>"
        "<p>Intensidade atual: %d%%</p>"
        "</body>"
        "</html>\r\n",
        button1_message, button2_message, intensidade_luz);
}

static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *request = (char *)p->payload;

    if (strstr(request, "GET /led/on")) {
        matriz_led_ligar(intensidade_luz);
        luz_ligada = true;
        buzzer_play_tone(700, 500);
        atualizar_oled(true);
    } else if (strstr(request, "GET /led/off")) {
        matriz_led_desligar();
        luz_ligada = false;
        buzzer_play_tone(700, 500);
        atualizar_oled(true);
    } else if (strstr(request, "GET /intensidade")) {
        intensidade_luz = (intensidade_luz == 100) ? 50 : 100;
        if (luz_ligada) matriz_led_ligar(intensidade_luz);
        snprintf(button2_message, sizeof(button2_message), "Intensidade ajustada via Web: %d%%", intensidade_luz);
        buzzer_play_tone(900, 200);
    }

    create_http_response();
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
    pbuf_free(p);
    return ERR_OK;
}

static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);
    return ERR_OK;
}

static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) return;
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) return;
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

void monitor_buttons() {
    static bool button1_last_state = false;
    static bool button2_last_state = false;

    bool button1_state = !gpio_get(BUTTON1_PIN);
    bool button2_state = !gpio_get(BUTTON2_PIN);

    if (button1_state != button1_last_state) {
        button1_last_state = button1_state;
        if (button1_state) {
            luz_ligada = !luz_ligada;
            if (luz_ligada) matriz_led_ligar(intensidade_luz);
            else matriz_led_desligar();
            buzzer_play_tone(700, 500);
            snprintf(button1_message, sizeof(button1_message), "Botão A pressionado!");
            atualizar_oled(true);
        } else {
            snprintf(button1_message, sizeof(button1_message), "Botão A solto!");
        }
    }

    if (button2_state != button2_last_state) {
        button2_last_state = button2_state;
        if (button2_state) {
            intensidade_luz = (intensidade_luz == 100) ? 50 : 100;
            if (luz_ligada) matriz_led_ligar(intensidade_luz);
            snprintf(button2_message, sizeof(button2_message), "Botão B pressionado! Intensidade: %d%%", intensidade_luz);
        } else {
            snprintf(button2_message, sizeof(button2_message), "Botão B solto!");
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(5000);
    printf("Iniciando sistema Smart Home...\n");

    if (cyw43_arch_init()) return 1;
    cyw43_arch_enable_sta_mode();
    bool wifi_conectado = (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000) == 0);

    struct netif *netif = netif_list;
    if (wifi_conectado) {
        printf("Conectado ao Wi-Fi!\nIP local: %s\n", ipaddr_ntoa(&netif->ip_addr));
    } else {
        printf("Erro ao conectar no Wi-Fi\n");
    }

    gpio_init(BUTTON1_PIN); gpio_set_dir(BUTTON1_PIN, GPIO_IN); gpio_pull_up(BUTTON1_PIN);
    gpio_init(BUTTON2_PIN); gpio_set_dir(BUTTON2_PIN, GPIO_IN); gpio_pull_up(BUTTON2_PIN);
    buzzer_init();

    start_http_server();

    // Inicializa comunicação I2C com o display
    i2c_init(OLED_I2C_PORT, OLED_I2C_FREQ);
    gpio_set_function(OLED_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA_PIN);
    gpio_pull_up(OLED_SCL_PIN);

    // Inicializa o display OLED
    ssd1306_init(OLED_I2C_PORT, OLED_SDA_PIN, OLED_SCL_PIN);
    atualizar_oled(wifi_conectado);

    while (true) {
        cyw43_arch_poll();
        monitor_buttons();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}
