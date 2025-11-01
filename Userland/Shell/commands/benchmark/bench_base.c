#include "bench_base.h"
#include "../commands.h"

bool bench_parse_duration(int argc, char **argv, uint64_t *duration_ms, const char *usage_message) {
    if (duration_ms == 0) {
        return false;
    }

    if (argc < 2) {
        *duration_ms = BENCH_DEFAULT_DURATION_MS;
        return true;
    }

    const char *arg = argv[1];
    if (arg == 0 || *arg == '\0') {
        if (usage_message) {
            print(usage_message);
        }
        return false;
    }

    uint64_t value = 0;
    for (int i = 0; arg[i] != '\0'; i++) {
        char c = arg[i];
        if (c < '0' || c > '9') {
            if (usage_message) {
                print(usage_message);
            }
            return false;
        }
        uint64_t digit = (uint64_t)(c - '0');
        if (value > (UINT64_MAX - digit) / 10) {
            value = BENCH_MAX_DURATION_MS;
            break;
        }
        value = value * 10 + digit;
        if (value > BENCH_MAX_DURATION_MS) {
            value = BENCH_MAX_DURATION_MS;
            break;
        }
    }

    if (value == 0) {
        value = BENCH_DEFAULT_DURATION_MS;
    }

    *duration_ms = value;
    return true;
}

uint64_t bench_ops_per_second(uint64_t operations, uint64_t elapsed_ms) {
    if (elapsed_ms == 0) {
        elapsed_ms = 1;
    }
    return (operations * 1000) / elapsed_ms;
}

static void bench_print_separator(char ch) {
    for (int i = 0; i < 58; i++) {
        printChar(ch);
    }
    printChar('\n');
}

void bench_print_arrow_value(const char *label, uint64_t value) {
    print("    -> ");
    if (label) {
        print(label);
    } else {
        print("Valor");
    }
    print(": ");
    printDec(value);
    print("\n");
}

void bench_print_summary(const char *titulo, const char *detalle_operaciones, uint64_t elapsed_ms, uint64_t operations, uint64_t ops_per_sec) {
    print("\n");
    bench_print_separator('=');

    print("  ");
    if (titulo && *titulo) {
        print(titulo);
    } else {
        print("Resultado del benchmark");
    }
    print("\n");

    bench_print_separator('-');

    print("Duracion (ms): ");
    printDec(elapsed_ms);
    print("\n");

    if (detalle_operaciones && *detalle_operaciones) {
        bench_print_arrow_value(detalle_operaciones, operations);
    } else {
        bench_print_arrow_value("Operaciones totales", operations);
    }

    bench_print_arrow_value("Operaciones por segundo", ops_per_sec);
}

void bench_print_footer(void) {
    bench_print_separator('=');
    print("\n");
}
