#ifndef tronarena_h
#define tronarena_h

    #include "tronConfig.h"
    #include <stdlib.h>
    
    uint8_t arenaGet(int col, int row);
    void arenaSet(int col, int row, uint8_t owner);
    void arenaClear(void);
    bool insideArena(int col, int row);


#endif