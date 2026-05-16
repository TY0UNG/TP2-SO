/* _loader.c */
#include <stdint.h>

extern char bss;
extern char endOfShell;

int main(int argc, char **argv);
extern void sys_exit(int status);

void * memset(void * destiny, int32_t c, uint64_t length);

void _start(int argc, char **argv) {
	//Clean BSS
	memset(&bss, 0, &endOfShell - &bss);
	int ret = main(argc, argv);
	sys_exit(ret);
	while(1);   // por las dudas; sys_exit no deberia retornar
}

void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}