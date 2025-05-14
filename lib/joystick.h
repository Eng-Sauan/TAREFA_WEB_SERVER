#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

void joystick_init(void);
int16_t joystick_read_x(void);
int16_t joystick_read_y(void);

// Valores normalizados: -100 a 100
int8_t joystick_get_x_percent(void);
int8_t joystick_get_y_percent(void);

#endif
