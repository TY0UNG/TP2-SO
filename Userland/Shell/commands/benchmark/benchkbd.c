#include "../commands.h"
#include "bench_base.h"

int benchkbd(char **argv, int argc) {
    const char *uso = "Uso: benchkbd [duracion_s]\n  duracion_s opcional entre 1 y 60 (por defecto 1).\n";
    uint64_t duracion_s;
    if (!bench_parse_duration(argc, argv, &duracion_s, uso)) {
        return 1;
    }

    uint64_t lecturas = 0;

    uint64_t inicio = bench_wait_for_next_second();
    uint64_t objetivo = inicio + duracion_s;
    uint64_t actual = inicio;

    do {
        (void)getKey();
        lecturas++;
        actual = bench_now_seconds();
    } while (actual < objetivo);

    uint64_t fin = (actual < objetivo) ? objetivo : actual;
    uint64_t transcurrido = (fin > inicio) ? (fin - inicio) : duracion_s;

    uint64_t lecturas_por_segundo = bench_ops_per_second(lecturas, transcurrido);

    bench_print_summary("Benchmark de lectura del teclado", "Lecturas totales", transcurrido, lecturas, lecturas_por_segundo);
    bench_print_footer();

    return 0;
}
