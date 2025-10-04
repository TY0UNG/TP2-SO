#include <inout.h>
#include <stdint.h>

extern int sys_write(int fd, const char* str);
extern size_t sys_read(const char* buffer);

int print(const char* str, int fd) {
    return sys_write(fd, str);
}

size_t read(char* buffer) {
    return sys_read(buffer);
}