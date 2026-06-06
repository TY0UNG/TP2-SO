#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef MM_OWN

#define ALIGNMENT 16

/*  el header de cada bloque. los punteros next y prev pasan a ser 
    payload del usuario cuando se ocupa el bloque */
typedef struct BlockHeader {
    size_t size;                 // bytes de payload
    size_t is_free;              // 1 = libre, 0 = ocupado
    struct BlockHeader *next;    // siguiente bloque libre en la free list
    struct BlockHeader *prev;    // anterior bloque libre en la free list
} BlockHeader;

/*  el footer con el size para hacer coalescing */
typedef struct BlockFooter {
    size_t size;
} BlockFooter;

/* --- estado global --- */
static BlockHeader *free_list_head = NULL;
static void *arena_base     = NULL;         // inicio de la zona gestionada
static size_t arena_size     = 0;           // bytes totales gestionados
static size_t used_bytes     = 0;           // ocupados (para getUsedMemory)
static bool initialized    = false;

/* --- Helpers --- */

// Puntero al payload (data) de un header. Esto es lo que devuelve malloc.
static void *payload_of(BlockHeader *h) {
    return (char*)h + sizeof(BlockHeader);
}

// Puntero al footer de un header
static BlockFooter *footer_of(BlockHeader *h) {
    return (BlockFooter*)((char*)payload_of(h) + h->size);
}

// Mantiene el footer sincronizado con el size del header.
static void sync_footer(BlockHeader *h) {
    footer_of(h)->size = h->size;
}

static size_t align_up(size_t n, size_t a) {
    return (n + a - 1) & ~(a - 1);
}

// Puntero al header de un payload. Esto es lo que recibe free.
static BlockHeader *header_of(void *p) {
    return (BlockHeader*)((char*)p - sizeof(BlockHeader));
}

// Header del bloque fisico siguiente en la cinta
static BlockHeader *next_block(BlockHeader *h) {
    return (BlockHeader*)((char*)footer_of(h) + sizeof(BlockFooter));
}

// Header del bloque fisico anterior en la cinta
static BlockHeader *prev_block(BlockHeader *h) {
    BlockFooter *pf = (BlockFooter*)((char*)h - sizeof(BlockFooter));
    return (BlockHeader*)((char*)h - sizeof(BlockFooter) - pf->size - sizeof(BlockHeader));
}

// Chequear next en la cinta
static bool has_next(BlockHeader *h) {
    return (char*)next_block(h) < (char*)arena_base + arena_size;
}

// Chequear prev en la cinta
static bool has_prev(BlockHeader *h) {
    return (char*)h > (char*)arena_base;
}

// Inserta un bloque de la free list
static void list_insert(BlockHeader *h) {
    h->prev = NULL;
    h->next = free_list_head;
    if (free_list_head) free_list_head->prev = h;
    free_list_head = h;
}

// Saca un bloque de la free list
static void list_remove(BlockHeader *h) {
    if (h->prev) h->prev->next = h->next;
    else         free_list_head = h->next;
    if (h->next) h->next->prev = h->prev;
}

/* --- Inicializacion del Manager --- */

void initializeMemoryManager(void *memoryStart, size_t memorySize) {
    // Alínea el comienzo de la arena a ALIGNMENT bytes, redondeando para arriba.
    uintptr_t raw = (uintptr_t) memoryStart;
    uintptr_t aligned = (raw + ALIGNMENT - 1) & ~((uintptr_t)ALIGNMENT - 1);
    size_t adjust  = (size_t)(aligned - raw);

    if (memorySize < adjust) { initialized = false; return; }
    memorySize -= adjust;

    // El bloque inicial cubre toda la arena, aca trunca hacia abajo lo que sobre (para no excederse de la memoria).
    size_t total = memorySize & ~((size_t)ALIGNMENT - 1);

    // Si no entra un header y footer, no alcanza el espacio.
    if (total < sizeof(BlockHeader) + sizeof(BlockFooter) + ALIGNMENT) {
        initialized = false;
        return;
    }
    arena_base = (void*) aligned;
    arena_size = total;
    used_bytes = 0;

    // Setea un unico bloque libre que cubre todo y la free list apunta a el.
    BlockHeader *h = (BlockHeader*) arena_base;
    h->size    = total - sizeof(BlockHeader) - sizeof(BlockFooter);
    h->is_free = 1;
    h->next    = NULL;
    h->prev    = NULL;
    sync_footer(h);

    free_list_head = h;
    initialized = true;
}

/* --- malloc / free --- */

void *malloc(size_t size) {
    if (!initialized || size == 0) return NULL;

    // Tamaño total del bloque (header + payload + footer) alineado a ALIGNMENT.
    size_t total_need = align_up(sizeof(BlockHeader) + size + sizeof(BlockFooter), ALIGNMENT);
    size_t need = total_need - sizeof(BlockHeader) - sizeof(BlockFooter);

    // first-fit
    BlockHeader *h = free_list_head;
    while (h != NULL && h->size < need) h = h->next;
    if (h == NULL) return NULL;

    list_remove(h);

    // Si sobra para otro bloque util, split
    size_t leftover = h->size - need;
    if (leftover >= sizeof(BlockHeader) + sizeof(BlockFooter) + ALIGNMENT) {
        h->size = need;
        sync_footer(h);

        BlockHeader *rest = next_block(h);
        rest->size = leftover - sizeof(BlockHeader) - sizeof(BlockFooter);
        rest->is_free = 1;
        sync_footer(rest);
        list_insert(rest);
    }

    h->is_free = 0;
    sync_footer(h);
    used_bytes += sizeof(BlockHeader) + h->size + sizeof(BlockFooter);
    return payload_of(h);
}

void free(void *ptr) {
    if (!initialized || ptr == NULL) return;

    BlockHeader *h = header_of(ptr);
    if (h->is_free) return;

    used_bytes -= sizeof(BlockHeader) + h->size + sizeof(BlockFooter);
    h->is_free = 1;

    // Coalescing hacia adelante.
    if (has_next(h)) {
        BlockHeader *n = next_block(h);
        if (n->is_free) {
            list_remove(n);
            h->size += sizeof(BlockFooter) + sizeof(BlockHeader) + n->size;
            sync_footer(h);
        }
    }

    // Coalescing hacia atras.
    if (has_prev(h)) {
        BlockHeader *p = prev_block(h);
        if (p->is_free) {
            list_remove(p);
            p->size += sizeof(BlockFooter) + sizeof(BlockHeader) + h->size;
            sync_footer(p);
            h = p;
        }
    }

    list_insert(h);
}

/* --- Estado de memoria --- */

size_t getTotalMemory(void) {
    return arena_size;
}

size_t getUsedMemory(void) {
    return used_bytes;
}

#endif