#ifndef str_h
#define str_h

int strcmp(const char* str1, const char* str2);
int strlen(const char* str);
void strcpy(char* dest, const char* src);
void strcat(char* dest, const char* src);
char * strchr(const char *s, char c);
int strparse(char * input, char ** output, char * splitter);

#endif