#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

#define MS_PER_TICK 55
#define AUDIO_BUFFER_LENGTH 256

typedef struct {
    uint16_t frequency;
    uint16_t duration;
} AudioEvent;

bool isAudioBufferEmpty(void);
void clearAudioBuffer(void);
AudioEvent getNextAudioEvent();
void play_sound(uint16_t freq, uint16_t duration);
void stop_current_sound(void);

void  audio_timer_handler();

extern void start_T();

#endif