#ifndef BENCH_BASE_H
#define BENCH_BASE_H

#include <stdint.h>
#include <stdbool.h>

#define BENCH_DEFAULT_DURATION_MS 1000
#define BENCH_MAX_DURATION_MS 60000

bool bench_parse_duration(int argc, char **argv, uint64_t *duration_ms, const char *usage_message);
uint64_t bench_ops_per_second(uint64_t operations, uint64_t elapsed_ms);
void bench_print_summary(const char *titulo, const char *detalle_operaciones, uint64_t elapsed_ms, uint64_t operations, uint64_t ops_per_sec);
void bench_print_arrow_value(const char *label, uint64_t value);
void bench_print_footer(void);

#endif
