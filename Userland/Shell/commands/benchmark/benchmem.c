#include "../commands.h"
#include "bench_base.h"

#define BENCH_MEM_BLOCK_SIZE 4096

static volatile uint8_t origen[BENCH_MEM_BLOCK_SIZE];
static volatile uint8_t destino[BENCH_MEM_BLOCK_SIZE];

int benchmem(char **argv, int argc) {
    const char *uso = "Uso: benchmem [duracion_s]\n  duracion_s opcional entre 1 y 60 (por defecto 1).\n";
    uint64_t duracion_s;
    if (!bench_parse_duration(argc, argv, &duracion_s, uso)) {
        return 1;
    }

    // Inicializar el buffer de origen una sola vez.
    for (int i = 0; i < BENCH_MEM_BLOCK_SIZE; i++) {
        origen[i] = (uint8_t)(i & 0xFF);
    }

    uint64_t iteraciones = 0;
    uint64_t inicio = bench_wait_for_next_second();
    uint64_t objetivo = inicio + duracion_s;
    uint64_t actual = inicio;

    do {
        for (int i = 0; i < BENCH_MEM_BLOCK_SIZE; i++) {
            destino[i] = origen[i] + (uint8_t)iteraciones;
        }
        iteraciones++;
        actual = bench_now_seconds();
    } while (actual < objetivo);

    uint64_t fin = (actual < objetivo) ? objetivo : actual;
    uint64_t transcurrido = (fin > inicio) ? (fin - inicio) : duracion_s;

    uint64_t ops_por_segundo = bench_ops_per_second(iteraciones, transcurrido);
    uint64_t bytes_por_segundo = ops_por_segundo * BENCH_MEM_BLOCK_SIZE;

    bench_print_summary("Benchmark de copia de memoria", "Copias de bloque", transcurrido, iteraciones, ops_por_segundo);
    bench_print_arrow_value("Bytes procesados por segundo", bytes_por_segundo);
    bench_print_footer();

    return 0;
}
