#include "../commands.h"
#include "bench_base.h"

int benchfloat(char **argv, int argc) {
    const char *uso = "Uso: benchfloat [duracion_s]\n  duracion_s opcional entre 1 y 60 (por defecto 1).\n";
    uint64_t duracion_s;
    if (!bench_parse_duration(argc, argv, &duracion_s, uso)) {
        return 1;
    }

    volatile double numerador = 1234567.0;
    volatile double denominador = 1.000001;
    volatile double acumulador = 0.0;

    uint64_t iteraciones = 0;
    uint64_t inicio = bench_wait_for_next_second();
    uint64_t objetivo = inicio + duracion_s;
    uint64_t actual = inicio;

    do {
        acumulador += numerador / denominador;
        numerador += 1.0;
        denominador += 0.000001;
        if (denominador > 2.0) {
            denominador = 1.000001;
        }
        iteraciones++;
        actual = bench_now_seconds();
    } while (actual < objetivo);

    uint64_t fin = (actual < objetivo) ? objetivo : actual;
    uint64_t transcurrido = (fin > inicio) ? (fin - inicio) : duracion_s;

    uint64_t ops_por_segundo = bench_ops_per_second(iteraciones, transcurrido);

    bench_print_summary("Benchmark de division en coma flotante", "Divisiones ejecutadas", transcurrido, iteraciones, ops_por_segundo);
    bench_print_footer();

    (void)acumulador;

    return 0;
}
