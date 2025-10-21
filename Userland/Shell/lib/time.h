#ifndef time_h
#define time_h
    #include <stdint.h>

    void getDateTime(uint8_t* buffer);
    uint64_t getMilisFromBoot();
    void sleep(uint64_t ms);

#endif