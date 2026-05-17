#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef MM_BUDDY

#define MIN_BLOCK_SHIFT 12 // 4 KB (2^12)
#define MAX_BLOCK_SHIFT 22 // 4 MB (2^22)
#define NUM_ORDERS (MAX_BLOCK_SHIFT - MIN_BLOCK_SHIFT + 1) // 11 niveles (0 al 10)

// Nodos intrusivos para las listas de bloques libres
typedef struct FreeBlock {
    struct FreeBlock* prev;
    struct FreeBlock* next;
} FreeBlock;

// --- Estructuras Globales del Memory Manager ---
bool initialized = false;
void* managed_memory_base;
size_t managed_memory_size;

FreeBlock* free_lists[NUM_ORDERS];
uint8_t* bitmaps[NUM_ORDERS];
uint8_t* page_orders;

// --- Funciones Auxiliares Privadas ---

// Invierte el bit correspondiente a un par de buddies
static void flip_bit(int order, size_t page_idx) {
    size_t bit_idx = page_idx >> (order + 1);
    bitmaps[order][bit_idx / 8] ^= (1 << (bit_idx % 8));
}

// Lee el valor del bit
static bool get_bit(int order, size_t page_idx) {
    size_t bit_idx = page_idx >> (order + 1);
    return (bitmaps[order][bit_idx / 8] & (1 << (bit_idx % 8))) != 0;
}

static void list_add(int order, FreeBlock* block) {
    block->prev = NULL;
    block->next = free_lists[order];
    if (free_lists[order]) free_lists[order]->prev = block;
    free_lists[order] = block;
}

static void list_remove(int order, FreeBlock* block) {
    if (block->prev) block->prev->next = block->next;
    else free_lists[order] = block->next;
    if (block->next) block->next->prev = block->prev;
}

void initializeMemoryManager(void * memoryStart, size_t memorySize) {
    if (memorySize < (1ULL << MIN_BLOCK_SHIFT)) return; // Falla por falta de RAM

    // Calcular el espacio máximo necesario para la metadata
    size_t total_pages_max = memorySize >> MIN_BLOCK_SHIFT;
    size_t order_array_size = total_pages_max; // 1 byte por página
    size_t bitmaps_size = 0;
    
    for (int i = 0; i < NUM_ORDERS; i++) {
        size_t num_pairs = total_pages_max >> (i + 1);
        bitmaps_size += (num_pairs + 7) / 8; // Redondeo a bytes
    }

    size_t metadata_size = order_array_size + bitmaps_size;

    // Acomodar la metadata al principio de la memoria
    uint8_t* meta_ptr = (uint8_t*)memoryStart;
    
    page_orders = meta_ptr;
    meta_ptr += order_array_size;

    for (int i = 0; i < NUM_ORDERS; i++) {
        bitmaps[i] = meta_ptr;
        size_t num_pairs = total_pages_max >> (i + 1);
        size_t bytes = (num_pairs + 7) / 8;
        
        // Inicializar bitmaps en 0
        for (size_t j = 0; j < bytes; j++) bitmaps[i][j] = 0;
        meta_ptr += bytes;
    }

    // Alinear el inicio de la zona de bloques a 4 KB
    uintptr_t start_addr = (uintptr_t)memoryStart + metadata_size;
    size_t min_block = 1ULL << MIN_BLOCK_SHIFT;
    
    if (start_addr % min_block != 0) {
        start_addr += min_block - (start_addr % min_block);
    }
    
    managed_memory_base = (void*)start_addr;

    // Calcular RAM útil y limpiar listas
    if (start_addr >= (uintptr_t)memoryStart + memorySize) return; // No sobró nada
    
    size_t usable_size = ((uintptr_t)memoryStart + memorySize) - start_addr;
    usable_size &= ~(min_block - 1); // Truncar bytes sueltos que no formen 4 KB
    managed_memory_size = usable_size;

    for (int i = 0; i < NUM_ORDERS; i++) free_lists[i] = NULL;

    // Particionar la RAM residual en las listas de bloques libres
    size_t current_offset = 0;
    while (usable_size >= min_block) {
        int order = NUM_ORDERS - 1;
        while (order >= 0) {
            size_t block_size = 1ULL << (MIN_BLOCK_SHIFT + order);
            
            // Si el bloque cabe Y el offset actual es múltiplo del bloque (Alineación natural)
            if (block_size <= usable_size && (current_offset % block_size) == 0) {
                FreeBlock* block = (FreeBlock*)((uintptr_t)managed_memory_base + current_offset);

                // Inicializamos el bitmap de este bloque para que no intente fusionarse hacia afuera.
                size_t page_idx = current_offset >> MIN_BLOCK_SHIFT;
                flip_bit(order, page_idx);

                list_add(order, block);
                
                current_offset += block_size;
                usable_size -= block_size;
                break;
            }
            order--;
        }
    }

    initialized = true;
}

void * malloc(size_t size) {
    if (!initialized || size == 0) return NULL;

    // Buscar qué "orden" satisface el tamaño pedido
    int order = 0;
    while (order < NUM_ORDERS && (1ULL << (MIN_BLOCK_SHIFT + order)) < size) {
        order++;
    }
    if (order >= NUM_ORDERS) return NULL; // Excede el máximo de 4 MB

    // Encontrar el nivel más bajo que tenga bloques disponibles
    int current_order = order;
    while (current_order < NUM_ORDERS && free_lists[current_order] == NULL) {
        current_order++;
    }

    if (current_order == NUM_ORDERS) return NULL; // Out of memory

    // Extraer el bloque grande
    FreeBlock* block = free_lists[current_order];
    list_remove(current_order, block);

    // Volteamos el bit en este nivel porque el bloque acaba de cambiar de "Libre" a "Ocupado/Dividido".
    size_t extracted_page_idx = ((uintptr_t)block - (uintptr_t)managed_memory_base) >> MIN_BLOCK_SHIFT;
    flip_bit(current_order, extracted_page_idx);

    // Cortarlo por la mitad hasta llegar al orden deseado
    while (current_order > order) {
        current_order--;
        size_t block_size = 1ULL << (MIN_BLOCK_SHIFT + current_order);
        
        // El buddy siempre es la mitad derecha del bloque recién cortado
        FreeBlock* buddy = (FreeBlock*)((uintptr_t)block + block_size);
        
        // Ponemos al gemelo derecho en la lista de libres
        list_add(current_order, buddy);
        
        // Volteamos el bit de este nivel porque ahora son diferentes (Uno ocupado, uno libre)
        size_t page_idx = ((uintptr_t)block - (uintptr_t)managed_memory_base) >> MIN_BLOCK_SHIFT;
        page_orders[page_idx] = 0xFF; // Nace siendo libre
        flip_bit(current_order, page_idx);
    }

    // Guardar la metadata oculta para que free() funcione sin tamaño
    size_t final_page_idx = ((uintptr_t)block - (uintptr_t)managed_memory_base) >> MIN_BLOCK_SHIFT;
    page_orders[final_page_idx] = order;

    return (void*)block;
}

void free(void * ptr) {
    if (!initialized || ptr == NULL) return;

    // Identificar matemáticamente el bloque mediante su offset
    size_t offset = (uintptr_t)ptr - (uintptr_t)managed_memory_base;
    size_t page_idx = offset >> MIN_BLOCK_SHIFT;

    if (page_orders[page_idx] == 0xFF) {
        return; 
    }

    // Leer qué tamaño tenía este bloque cuando se asignó
    int order = page_orders[page_idx];
    
    // Lo marcamos como libre inmediatamente para evitar futuros double frees
    page_orders[page_idx] = 0xFF;

    // Fusión recursiva (Merge) con los bitmaps
    while (order < NUM_ORDERS - 1) {
        
        // Al liberar, invertimos el bit
        flip_bit(order, page_idx);

        if (get_bit(order, page_idx)) {
            // Si el bit quedó en 1, significa que antes era 0 (ambos estaban ocupados)
            // Como su gemelo sigue ocupado, NO podemos fusionarlos.
            break; 
        }

        // Se pueden fusionar pues ambos estan libres
        
        // Calculamos la posicion del buddy
        size_t buddy_page_idx = page_idx ^ (1ULL << order);
        FreeBlock* buddy = (FreeBlock*)((uintptr_t)managed_memory_base + (buddy_page_idx << MIN_BLOCK_SHIFT));
        
        // Sacamos al buddy de la lista porque se va a fusionar
        list_remove(order, buddy);

        // Limpiamos el bit diferenciador para obtener el inicio del nuevo bloque gigante
        page_idx &= ~(1ULL << order);
        
        // Subimos un nivel de tamaño para revisar si podemos seguir fusionando
        order++;
        ptr = (void*)((uintptr_t)managed_memory_base + (page_idx << MIN_BLOCK_SHIFT));
    }

    // Añadimos el bloque consolidado final a su lista correspondiente
    list_add(order, (FreeBlock*)ptr);
}

// Devuelve la cantidad total de RAM útil gestionada por el Buddy System
size_t getTotalMemory() {
    if (!initialized) return 0;
    
    // Devolvemos el tamaño calculado durante la fase de Bootstrap
    return managed_memory_size;
}

// Devuelve la cantidad de memoria que actualmente está asignada a los programas
size_t getUsedMemory() {
    if (!initialized) return 0;

    size_t free_memory = 0;

    // Recorremos cada nivel de tamaño
    for (int i = 0; i < NUM_ORDERS; i++) {
        
        // Calculamos cuánto pesa un bloque en este nivel (ej. Order 0 = 4096)
        size_t block_size = 1ULL << (MIN_BLOCK_SHIFT + i);
        
        // Tomamos el primer bloque libre de esta lista
        FreeBlock* current = free_lists[i];

        // Recorremos la lista enlazada sumando el tamaño por cada bloque que encontremos
        while (current != NULL) {
            free_memory += block_size;
            current = current->next;
        }
    }

    // La memoria en uso es simplemente la memoria total menos la que está libre
    return managed_memory_size - free_memory;
}
#endif
