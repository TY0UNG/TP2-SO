#include "tronAudio.h"

void playMenuTheme(void) {
    clear_audio_buffer();
    play_sound(392, 160);
    play_sound(0, 40);
    play_sound(523, 160);
    play_sound(0, 40);
    play_sound(659, 220);
    play_sound(0, 40);
    play_sound(784, 260);
    play_sound(0, 120);
}

void playRoundTheme(void) {
    play_sound(440, 110);
    play_sound(587, 110);
    play_sound(784, 110);
    play_sound(0, 55);
    play_sound(659, 110);
    play_sound(523, 110);
    play_sound(0, 55);
}

void playCrashSound(void) {
    clear_audio_buffer();
    play_sound(120, 120);
    play_sound(80, 120);
    play_sound(40, 180);
}