#include <inout.h>
#include <stdint.h>

extern int sys_write(int fd, const char* str);
extern size_t sys_read(const char* buffer);
extern void sys_clear();

int print(const char* str) {
    return sys_write(STDOUT, str);
}

int println(const char* str) {
    return print(str) && print("\n");
}

int printerr(const char* str) {
    return sys_write(STDERR, str);
}

size_t read(char* buffer) {
    return sys_read(buffer);
}

void clear() {  
    sys_clear();
}