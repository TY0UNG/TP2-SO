#include <str.h>

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}

int strlen(const char* str) {
    const char *p = str;
    while (*p) p++;
    return (unsigned long)(p - str);
}

void strcpy(char* dest, const char* src) {

}

void strcat(char* dest, const char* src) {

}
