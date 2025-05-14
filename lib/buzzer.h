#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

#define BUZZER_GPIO 21

void buzzer_init(void);
void buzzer_play_tone(uint32_t freq_hz, uint32_t duration_ms);
void buzzer_stop(void);

#endif
