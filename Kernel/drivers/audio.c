#include <audio.h>
#include <video.h>


extern void output_audio(uint16_t freq, uint16_t duration);
extern void stop_audio(void);

static AudioEvent buffer[AUDIO_BUFFER_LENGTH];
static uint8_t head = 0;
static uint8_t tail = 0;
static uint8_t size = 0;

static bool isFull() { 
    return size == AUDIO_BUFFER_LENGTH; 
}

bool isAudioBufferEmpty() { 
    return size == 0; 
}

void clearAudioBuffer() {
    size = head = tail = 0;
    stop_audio(); 
}

static void queue(AudioEvent a) {
    if (isFull()) getNextAudioEvent();  
    buffer[head] = a;
    head = (head + 1) % AUDIO_BUFFER_LENGTH;
    size++;
}

AudioEvent getNextAudioEvent() {
    AudioEvent aux = buffer[tail];
    tail = (tail + 1) % AUDIO_BUFFER_LENGTH;
    size--;
    return aux;
}

void audio_handler() {
    if (isAudioBufferEmpty()) return;

    AudioEvent aud = getNextAudioEvent();
    output_audio(aud.frequency, aud.duration);
}

void play_sound(uint16_t freq, uint16_t duration) {
    AudioEvent e = {freq, duration};
    queue(e);
}

void stop_current_sound() {
    stop_audio();
}