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
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
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

    output[count] = NULL; // terminar el array: el kernel recorre argv hasta NULL
    return count;
}

static void invertir(char *cadena, int longitud) {
    int i = 0;
    int j = longitud - 1;
    char temp;
    while (i < j) {
        temp = cadena[i];
        cadena[i] = cadena[j];
        cadena[j] = temp;
        i++;
        j--;
    }
}

char *parseInt(int n, char *buffer, uint64_t max) {
    long long num = n;
    int i = 0;
    bool es_negativo = false;

    if (buffer == NULL || max == 0) {
        return NULL;
    }
    
    if (num == 0) {
        if (max < 2) return NULL;
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    if (num < 0) {
        es_negativo = true;
        num = -num;
    }

    while (num != 0) {
        if (i >= max - 1) {
            buffer[0] = '\0';
            return NULL; 
        }
        buffer[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (es_negativo) {
        if (i >= max - 1) {
            buffer[0] = '\0';
            return NULL;
        }
        buffer[i++] = '-';
    }

    buffer[i] = '\0';
    invertir(buffer, i);

    return buffer;
}