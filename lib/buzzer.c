#include "buzzer.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

static uint buzzer_slice;

void buzzer_init(void) {
    gpio_set_function(BUZZER_GPIO, GPIO_FUNC_PWM);
    buzzer_slice = pwm_gpio_to_slice_num(BUZZER_GPIO);
    pwm_set_enabled(buzzer_slice, false);
}

void buzzer_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    uint32_t clock = 125000000;
    uint32_t divider = 1;
    uint32_t wrap = clock / (freq_hz - divider);

    pwm_set_clkdiv(buzzer_slice, divider);
    pwm_set_wrap(buzzer_slice, wrap);
    pwm_set_gpio_level(BUZZER_GPIO, wrap / 2); // 50% duty cycle
    pwm_set_enabled(buzzer_slice, true);

    sleep_ms(duration_ms);

    buzzer_stop();
}

void buzzer_stop(void) {
    pwm_set_enabled(buzzer_slice, false);
    pwm_set_gpio_level(BUZZER_GPIO, 0);
}
