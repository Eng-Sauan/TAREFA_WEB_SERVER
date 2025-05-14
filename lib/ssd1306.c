#include "ssd1306.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static i2c_inst_t *ssd1306_i2c;

static void ssd1306_send_command(uint8_t cmd) {
    uint8_t data[] = {0x00, cmd};
    i2c_write_blocking(ssd1306_i2c, SSD1306_ADDR, data, 2, false);
}

static void ssd1306_send_data(uint8_t *data, size_t len) {
    uint8_t buf[len + 1];
    buf[0] = 0x40;
    memcpy(&buf[1], data, len);
    i2c_write_blocking(ssd1306_i2c, SSD1306_ADDR, buf, len + 1, false);
}

void ssd1306_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin) {
    ssd1306_i2c = i2c;
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    sleep_ms(100);

    uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F,
        0xD3, 0x00, 0x40, 0x8D, 0x14,
        0x20, 0x00, 0xA1, 0xC8, 0xDA,
        0x12, 0x81, 0xCF, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };

    for (int i = 0; i < sizeof(init_cmds); i++)
        ssd1306_send_command(init_cmds[i]);

    ssd1306_clear();
    ssd1306_show();
}

void ssd1306_clear(void) {
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_show(void) {
    ssd1306_send_command(0x21); // set column range
    ssd1306_send_command(0);
    ssd1306_send_command(SSD1306_WIDTH - 1);
    ssd1306_send_command(0x22); // set page range
    ssd1306_send_command(0);
    ssd1306_send_command((SSD1306_HEIGHT / 8) - 1);

    for (int i = 0; i < sizeof(buffer); i += 16)
        ssd1306_send_data(&buffer[i], 16);
}

void ssd1306_draw_pixel(uint8_t x, uint8_t y, bool color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    if (color)
        buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

// Fonte básica (5x7)
static const uint8_t font5x7[][5] = {
    // código ASCII 32 até 127
    // exemplo: caractere 'A'
    [ 'A' - 32 ] = { 0x7C, 0x12, 0x11, 0x12, 0x7C },
    // você pode preencher todos os caracteres aqui
};

void ssd1306_draw_char(uint8_t x, uint8_t y, char c) {
    if (c < 32 || c > 126) return;

    const uint8_t *glyph = font5x7[c - 32];
    for (int i = 0; i < 5; i++) {
        uint8_t line = glyph[i];
        for (int j = 0; j < 7; j++) {
            ssd1306_draw_pixel(x + i, y + j, (line >> j) & 0x01);
        }
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str) {
    while (*str) {
        ssd1306_draw_char(x, y, *str++);
        x += 6;
    }
}
