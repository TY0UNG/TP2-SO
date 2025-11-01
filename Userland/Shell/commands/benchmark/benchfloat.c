#include "../commands.h"
#include "bench_base.h"

int benchfloat(char **argv, int argc) {
    const char *uso = "Uso: benchfloat [duración_ms]\n  duración_ms opcional entre 1 y 60000 (por defecto 1000).\n";
    uint64_t duración_ms;
    if (!bench_parse_duration(argc, argv, &duración_ms, uso)) {
        return 1;
    }

    volatile double numerador = 1234567.0;
    volatile double denominador = 1.000001;
    volatile double acumulador = 0.0;

    uint64_t iteraciones = 0;
    uint64_t inicio = getMilisFromBoot();
    uint64_t ahora = inicio;

    do {
        acumulador += numerador / denominador;
        numerador += 1.0;
        denominador += 0.000001;
        if (denominador > 2.0) {
            denominador = 1.000001;
        }
        iteraciones++;
        ahora = getMilisFromBoot();
    } while (ahora - inicio < duración_ms);

    uint64_t transcurrido = ahora - inicio;
    uint64_t ops_por_segundo = bench_ops_per_second(iteraciones, transcurrido);

    bench_print_summary("Benchmark de división en coma flotante", "Divisiones ejecutadas", transcurrido, iteraciones, ops_por_segundo);
    bench_print_footer();

    (void)acumulador;

    return 0;
}
