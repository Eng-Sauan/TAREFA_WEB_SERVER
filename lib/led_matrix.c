#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"


// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

int matriz0[5][5][3] = {
  {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
  {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
  {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
  {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
  {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};

int matriz1[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};


int linha;

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
    };
    typedef struct pixel_t pixel_t;
    typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

    // Declaração do buffer de pixels que formam a matriz.
    npLED_t leds[LED_COUNT];

    // Variáveis para uso da máquina PIO.
    PIO np_pio;
    uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// Função para converter a posição do matriz para uma posição do vetor.
int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return y * 5 + x; // Linha par (esquerda para direita).
    } else {
        return y * 5 + (4 - x); // Linha ímpar (direita para esquerda).
    }
}

// Função para definir a intensidade das cores do LED
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 10;
    G = g * 10;
    B = b * 10;
    return (G << 24) | (R << 16) | (B << 8);
}

void FRAME1(){
      for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
          int index = getIndex(x, y);
          npSetLED(index, matriz0[y][x][0], matriz0[y][x][1], matriz0[y][x][2]);
      }
    }
}

void FRAME2(){
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
        int index = getIndex(x, y);
        npSetLED(index, matriz1[y][x][0], matriz1[y][x][1], matriz1[y][x][2]);
    }
  }
}

void LEDS() {
  static absolute_time_t ultimo_piscar = {0};
  static int frame_index = 0;
  static bool iniciado = false;

  if (!iniciado) {
      npInit(LED_PIN);
      npClear();
      npWrite();
      iniciado = true;
  }

  // Piscar a cada 200ms
  if (absolute_time_diff_us(ultimo_piscar, get_absolute_time()) > 200000) {
      if (frame_index % 2 == 0) {
          FRAME1();
      } else {
          FRAME2();
      }

      npWrite();
      frame_index++;
      if (frame_index >= 10) frame_index = 0; // Alterna entre 0 a 9
      ultimo_piscar = get_absolute_time();
  }
}

