#include "joystick.h"
#include "hardware/adc.h"

#define JOY_X_CHANNEL 0 // ADC0 (GPIO26)
#define JOY_Y_CHANNEL 1 // ADC1 (GPIO27)

#define ADC_MID 2048
#define ADC_MAX 4095

void joystick_init(void) {
    adc_init();
    adc_gpio_init(26); // X
    adc_gpio_init(27); // Y
}

static uint16_t read_adc_channel(uint channel) {
    adc_select_input(channel);
    return adc_read();
}

int16_t joystick_read_x(void) {
    return read_adc_channel(JOY_X_CHANNEL);
}

int16_t joystick_read_y(void) {
    return read_adc_channel(JOY_Y_CHANNEL);
}

int8_t joystick_get_x_percent(void) {
    int16_t raw = joystick_read_x();
    return (int8_t)(((int32_t)raw - ADC_MID) * 100 / ADC_MID);
}

int8_t joystick_get_y_percent(void) {
    int16_t raw = joystick_read_y();
    return (int8_t)(((int32_t)raw - ADC_MID) * 100 / ADC_MID);
}
