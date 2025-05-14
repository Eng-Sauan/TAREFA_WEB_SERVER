#include "input.h"
#include "hardware/gpio.h"

void input_init(void) {
    gpio_init(BUTTON_A_GPIO);
    gpio_set_dir(BUTTON_A_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_A_GPIO);

    gpio_init(BUTTON_B_GPIO);
    gpio_set_dir(BUTTON_B_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_B_GPIO);
}

bool button_a_pressed(void) {
    return !gpio_get(BUTTON_A_GPIO); // Pressionado = LOW
}

bool button_b_pressed(void) {
    return !gpio_get(BUTTON_B_GPIO); // Pressionado = LOW
}
