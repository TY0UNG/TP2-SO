#ifndef time_h
#define time_h
    #include <stdint.h>
    #include <tick.h>

    void getTime(uint8_t *time_buffer);
    uint64_t getMilisFromBoot();
    void calibrateMilis();

#endif