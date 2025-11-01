#ifndef time_h
#define time_h
    #include <stdint.h>

    typedef struct date_time {
        uint8_t year, month, daymonth, hour, minutes, seconds
    } DateTime;

    void getDateTime(DateTime * output);
    uint64_t getMilisFromBoot();
    void sleep(uint64_t ms);
    void printTime(DateTime * datetime);
    void printHR_M_S(uint8_t * datetime);

#endif