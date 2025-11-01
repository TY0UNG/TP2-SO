#ifndef BENCH_BASE_H
#define BENCH_BASE_H

#include <stdint.h>
#include <stdbool.h>
#include "../../lib/time.h"

#define BENCH_DEFAULT_DURATION_S 1
#define BENCH_MAX_DURATION_S 60

bool bench_parse_duration(int argc, char **argv, uint64_t *duration_s, const char *usage_message);
uint64_t bench_ops_per_second(uint64_t operations, uint64_t elapsed_s);
uint64_t bench_now_seconds(void);
uint64_t bench_seconds_from_datetime(const DateTime *dt);
uint64_t bench_wait_for_next_second(void);
void bench_print_summary(const char *titulo, const char *detalle_operaciones, uint64_t elapsed_s, uint64_t operations, uint64_t ops_per_sec);
void bench_print_arrow_value(const char *label, uint64_t value);
void bench_print_footer(void);

#endif
