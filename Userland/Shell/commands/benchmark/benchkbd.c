#include "../commands.h"
#include "bench_base.h"

int benchkbd(char **argv, int argc) {
    const char *uso = "Uso: benchkbd [duracion_ms]\n  duracion_ms opcional entre 1 y 60000 (por defecto 1000).\n";
    uint64_t duracion_ms;
    if (!bench_parse_duration(argc, argv, &duracion_ms, uso)) {
        return 1;
    }

    uint64_t lecturas = 0;

    uint64_t inicio = getMilisFromBoot();
    uint64_t ahora = inicio;

    do {
        (void)getKey();
        lecturas++;
        ahora = getMilisFromBoot();
    } while (ahora - inicio < duracion_ms);

    uint64_t transcurrido = ahora - inicio;
    uint64_t lecturas_por_segundo = bench_ops_per_second(lecturas, transcurrido);

    bench_print_summary("Benchmark de lectura del teclado", "Lecturas totales", transcurrido, lecturas, lecturas_por_segundo);
    bench_print_footer();

    return 0;
}
