#include <stdint.h>
extern uint8_t endOfKernel[];
static uintptr_t next_free_address = 0;

void * memset(void * destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	// Patron para escribir de a 8 bytes
    uint64_t pattern = 0x0101010101010101ULL * chr;

    uint64_t *d64 = (uint64_t *)destination;
    uint64_t i;

    // Bucle principal rápido: escribe de a 8 bytes
    for (i = 0; i < length / sizeof(uint64_t); i++)
    {
        d64[i] = pattern;
    }

    // Maneja los bytes restantes al final
    uint8_t *d8 = (uint8_t *)(d64 + i);
    uint64_t remainder = length % sizeof(uint64_t);
    while (remainder--)
    {
        *d8++ = chr;
    }
    
    return destination;
}

void * memcpy(void * destination, const void * source, uint64_t length)
{
	/*
	* memcpy does not support overlapping buffers, so always do it
	* forwards. (Don't change this without adjusting memmove.)
	*
	* For speedy copying, optimize the common case where both pointers
	* and the length are word-aligned, and copy word-at-a-time instead
	* of byte-at-a-time. Otherwise, copy by bytes.
	*
	* The alignment logic below should be portable. We rely on
	* the compiler to be reasonably intelligent about optimizing
	* the divides and modulos out. Fortunately, it is.
	*/
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *) destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t * d = (uint8_t*)destination;
		const uint8_t * s = (const uint8_t*)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

void* alloc(uint64_t size) {
    // Si es la primera vez que se llama, inicializa el puntero
    // con la dirección real del final del kernel.
    if (next_free_address == 0) {
        next_free_address = (uintptr_t)endOfKernel;
    }

    // Opcional pero recomendado: alinear la dirección a 8 o 16 bytes.
    if (next_free_address % 16 != 0) {
        next_free_address += 16 - (next_free_address % 16);
    }

    // Guarda la dirección actual para devolverla
    uintptr_t allocated_block_ptr = next_free_address;

    // "Empuja" el puntero para la siguiente asignación
    next_free_address += size;

    // Devuelve el puntero al bloque que acabas de reservar
    return (void*)allocated_block_ptr;
}


uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base) {
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	buffer[digits] = '\0';

	return digits;
}
