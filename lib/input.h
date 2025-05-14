#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#define BUTTON_A_GPIO 5
#define BUTTON_B_GPIO 6

void input_init(void);

bool button_a_pressed(void);
bool button_b_pressed(void);

#endif
