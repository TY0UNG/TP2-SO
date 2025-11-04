#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void clear_audio_buffer(void);

int is_audio_buffer_empty(void);

void play_sound(uint16_t frequency, uint16_t duration);

#endif