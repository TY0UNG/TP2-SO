#ifndef time_h
#define time_h
    #include <stdint.h>

    void getDateTime(uint8_t* buffer);
    uint64_t getMilisFromBoot();
    void sleep(uint64_t ms);
    void printTime(uint8_t * datetime);
    uint8_t * difTime(uint8_t * Time_0,uint8_t * Time_1);

#endif