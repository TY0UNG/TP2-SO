#ifndef time_h
#define time_h

#include <stdint.h>
#include <tick.h>

    typedef struct date_time {
        uint8_t year, month, daymonth, hour, minutes, seconds;
    } DateTime;

    void getTime(DateTime * output);
    uint64_t getMilisFromBoot();
    void calibrateMilis();

#endif