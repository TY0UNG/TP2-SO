#include <sound.h>
#include <stdint.h>

extern void sys_play_sound(uint16_t frequency, uint16_t duration);
extern int sys_is_audio_buffer_empty(void);
extern void sys_clear_audio_buffer(void);

void play_sound(uint16_t frequency, uint16_t duration) {
    sys_play_sound(frequency, duration);
}

int is_audio_buffer_empty(void) {
    return sys_is_audio_buffer_empty();
}

 void clear_audio_buffer(void) {
    sys_clear_audio_buffer();
}
