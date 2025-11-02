#include <audio.h>
#include <video.h>
#include <interrupts.h>

extern void output_audio_start(uint16_t freq);
extern void stop_audio(void);

#define MS_PER_TICK 55
#define AUDIO_BUFFER_LENGTH 2000


static AudioEvent buffer[AUDIO_BUFFER_LENGTH];
static  uint16_t head = 0;  
static  uint16_t tail = 0;
static  uint16_t size = 0;
static  int ticks_restantes = 0;  

static bool isFull() { 
    return size == AUDIO_BUFFER_LENGTH; 
}

bool isAudioBufferEmpty() { 
    return size == 0; 
}

static void queue(AudioEvent a) {
    _cli();  //
    if (isFull()) {
        tail = (tail + 1) % AUDIO_BUFFER_LENGTH;
        size--;
    }
    buffer[head] = a;
    head = (head + 1) % AUDIO_BUFFER_LENGTH;
    size++;
    _sti();
}

AudioEvent getNextAudioEvent() {
    AudioEvent aux = buffer[tail];
    tail = (tail + 1) % AUDIO_BUFFER_LENGTH;
    size--;
    return aux;
}

void audio_timer_handler() {
    if (ticks_restantes > 0) {
        ticks_restantes--;
        if (ticks_restantes == 0) {
            stop_audio(); 
        }
        return; 
    }
    
    if (isAudioBufferEmpty()) return; 
    
    AudioEvent aud = getNextAudioEvent();

    if (aud.frequency == 0) {
        
        stop_audio();
        if (aud.duration > 0) {
            ticks_restantes = aud.duration / MS_PER_TICK;
            if (ticks_restantes == 0) {
                ticks_restantes = 1;
            }
        }
    } else if (aud.duration > 0) {
        ticks_restantes = aud.duration / MS_PER_TICK;
        if (ticks_restantes == 0) {
            ticks_restantes = 1;
        }
        output_audio_start(aud.frequency);
    } else {
        ticks_restantes = 1;
        output_audio_start(aud.frequency);
    }
}

void play_sound(uint16_t freq, uint16_t duration) {
    AudioEvent e = {freq, duration};
    queue(e);
}

void clearAudioBuffer() {
    _cli(); 
    stop_audio();
    ticks_restantes = 0;
    head = tail = size = 0;
    _sti();
}

/*
#include <audio.h>

extern void output_audio_start(uint16_t freq);
extern void stop_audio(void);



#define MS_PER_TICK 10
#define AUDIO_BUFFER_LENGTH 1000 // Tamaño del buffer (puedes ajustarlo)

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

static void queue(AudioEvent a) {
    if (isFull()) {
        // Si está lleno, descarta el sonido más viejo para
        // dejar espacio al nuevo.
        tail = (tail + 1) % AUDIO_BUFFER_LENGTH;
        size--;
    }
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


static int ticks_restantes = 0;


void audio_timer_handler() {
    
    if (ticks_restantes > 0) {
        ticks_restantes--;
        
        
        if (ticks_restantes == 0) {
            stop_audio(); 
        }
        return; 
    }
    
    if (isAudioBufferEmpty()) return; 
    
    AudioEvent aud = getNextAudioEvent();
    
    if (aud.duration > 0) {
        ticks_restantes = aud.duration / MS_PER_TICK;
        
        if (ticks_restantes == 0) {
            ticks_restantes = 1; 
        }
    } else {
        ticks_restantes = 0; // Duración 0
    }

    output_audio_start(aud.frequency); 
}


void play_sound(uint16_t freq, uint16_t duration) {
    AudioEvent e = {freq, duration};
    queue(e);
}
void clearAudioBuffer() {
    stop_audio();
    ticks_restantes = 0;
    head =tail = size = 0;
}
*/