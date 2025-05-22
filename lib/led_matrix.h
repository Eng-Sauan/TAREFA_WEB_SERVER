#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include "hardware/pio.h"     // Para o tipo PIO
#include "pico/stdlib.h"      // Para uint e outras funções do SDK

#define LED_COUNT 25
#define LED_PIN 7

typedef struct {
    uint8_t G, R, B;
} pixel_t;

typedef pixel_t npLED_t;

extern npLED_t leds[LED_COUNT];

extern PIO np_pio;
extern uint sm;

void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
int getIndex(int x, int y);
uint32_t matrix_rgb(double b, double r, double g);
void FRAME1();
void FRAME2();
void LEDS();

#endif // LED_MATRIX_H
