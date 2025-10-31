#include <audio.h>
#include <video.h>

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
/*
extern void output_audio(uint16_t freq, uint16_t duration);
extern void stop_audio(void);

static AudioEvent buffer[AUDIO_BUFFER_LENGTH];
static uint8_t head = 0;
static uint8_t tail = 0;
static uint8_t size = 0;


static uint64_t audio_start_time = 0;
static uint16_t current_duration = 0;
static bool is_playing = false;

static bool isFull() { 
    return size == AUDIO_BUFFER_LENGTH; 
}

bool isAudioBufferEmpty() { 
    return size == 0; 
}

void clearAudioBuffer() {
    size = head = tail = 0;
    is_playing = false;  
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

void audio_timer_tick() {
    uint64_t current_time = getMilisFromBoot();
    
    // Si hay algo reproduciéndose, verificar si terminó
    if (is_playing) {
        if (current_time - audio_start_time >= current_duration) {
            is_playing = false;
            stop_audio();
        } else {
            return; // Todavía está reproduciendo
        }
    }
    
    // Si no hay nada reproduciéndose, tomar el siguiente del buffer
    if (!is_playing && !isAudioBufferEmpty()) {
        AudioEvent aud = getNextAudioEvent();
        output_audio(aud.frequency, aud.duration);
        audio_start_time = current_time;
        current_duration = aud.duration;
        is_playing = true;
    }
}

void play_sound(uint16_t freq, uint16_t duration) {
    AudioEvent e = {freq, duration};
    queue(e);
}

void stop_current_sound() {
    stop_audio();
}*/






/*el de asm 

section .text
GLOBAL output_audio
GLOBAL stop_audio

output_audio:
    push rbp
    mov rbp, rsp
    push rax
    push rdx
    push rcx
    push rbx
    
    ; rdi = frec, rsi = dura
    test rdi, rdi
    jz .end
    
    ; Calcular divisor = 1193180 / frecuencia
    mov rax, 1193180
    xor rdx, rdx
    div rdi
    mov rbx, rax            
    

    mov al, 0xB6
    out 0x43, al
    mov al, bl
    out 0x42, al
    mov al, bh
    out 0x42, al
    
    ; Activar speaker
    in al, 0x61
    or al, 0x03
    out 0x61, al
    
    ; (rsi = ms)
    mov rcx, rsi
    imul rcx, 100000        ; Ajustar según necesites
.delay:
    pause
    dec rcx
    jnz .delay
    
    ; Desactivar speaker
    in al, 0x61
    and al, 0xFC
    out 0x61, al
    
.end:
    pop rbx
    pop rcx
    pop rdx
    pop rax
    pop rbp
    ret

stop_audio:
    push rax
    in al, 0x61
    and al, 0xFC
    out 0x61, al
    pop rax
    ret
    
*/