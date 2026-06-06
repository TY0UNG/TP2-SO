#include <stdint.h>

int createProcess(const char* name, int (*entry)(char**, int), int argc , char ** argv);