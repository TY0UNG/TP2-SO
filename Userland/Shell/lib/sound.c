#include <sound.h>
#include <stdint.h>

extern void sys_audio_handler(void);
extern void sys_play_sound(uint16_t frequency, uint16_t duration);
extern int sys_is_audio_buffer_empty(void);
extern void sys_clear_audio_buffer(void);

// Wrappers para facilitar el uso
void audio_handler(void) {
    sys_audio_handler();
}

void play_sound(uint16_t frequency, uint16_t duration) {
    sys_play_sound(frequency, duration);
}

int is_audio_buffer_empty(void) {
    return sys_is_audio_buffer_empty();
}

 void clear_audio_buffer(void) {
    sys_clear_audio_buffer();
}
