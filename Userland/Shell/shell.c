#include <inout.h>

char * v = (char*)0xB8000 + 79 * 2;

int main() {
    *v = 'X';
	*(v+1) = 0x74;
    print("Hola", STDERR);
    while(1) {}
	return 0;
}