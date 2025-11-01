#include "../commands.h"
#include "bench_base.h"

int benchhw(char **argv, int argc) {
    const char *uso = "Uso: benchhw [duracion_ms]\n  duracion_ms opcional entre 1 y 60000 (por defecto 1000).\n";
    uint64_t duracion_ms;
    if (!bench_parse_duration(argc, argv, &duracion_ms, uso)) {
        return 1;
    }

    DateTime datetime;
    uint64_t iteraciones = 0;
    uint64_t inicio = getMilisFromBoot();
    uint64_t ahora = inicio;

    do {
        getDateTime(&datetime);
        iteraciones++;
        ahora = getMilisFromBoot();
    } while (ahora - inicio < duracion_ms);

    uint64_t transcurrido = ahora - inicio;
    uint64_t ops_por_segundo = bench_ops_per_second(iteraciones, transcurrido);

    bench_print_summary("Benchmark de acceso al RTC", "Lecturas realizadas", transcurrido, iteraciones, ops_por_segundo);
    bench_print_footer();

    return 0;
}
