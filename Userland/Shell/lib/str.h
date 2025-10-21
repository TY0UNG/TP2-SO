#ifndef str_h
#define str_h
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

int strcmp(const char* str1, const char* str2);
int strlen(const char* str);
void strcpy(char* dest, const char* src);
void strcat(char* dest, const char* src);
char * strchr(const char *s, char c);
int strparse(char * input, char ** output, char * splitter);

char *parseInt(int n, char *buffer, uint64_t max);

#endif