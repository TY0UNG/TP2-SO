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
    while (*dest != '\0') dest++;
    while ((*dest++ = *src++));
}

char * strchr(const char *s, char c) {
    while (*s) {
        if (*s == c) return (char *)s;
        s++;
    }
    return 0;
}

int strparse(char * input, char ** output, char * splitter) {
    if (!input || !output || !splitter) return 0;

    int count = 0;
    char *token = input;

    while (*token) {
        while (*token && strchr(splitter, *token)) token++;

        if (!*token) break;

        output[count++] = token;

        while (*token && !strchr(splitter, *token)) token++;

        if (*token) {
            *token = '\0';
            token++;
        }
    }

    return count;
}
