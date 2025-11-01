#include "bench_base.h"
#include "../commands.h"

static uint8_t bench_rtc_to_binary(uint8_t value) {
    uint8_t low = value & 0x0F;
    uint8_t high = (value >> 4) & 0x0F;

    if (low > 9u || high > 9u) {
        return value;
    }

    return (uint8_t)(high * 10u + low);
}

static uint64_t bench_days_from_civil(uint16_t year, uint8_t month, uint8_t day) {
    if (month <= 2) {
        year -= 1;
        month += 12;
    }

    uint64_t era = year / 400u;
    uint64_t yoe = (uint64_t)(year - (uint16_t)(era * 400u));
    uint64_t doy = (uint64_t)((153u * (month - 3u) + 2u) / 5u) + (uint64_t)day - 1u;
    uint64_t doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;

    return era * 146097u + doe;
}

uint64_t bench_seconds_from_datetime(const DateTime *dt) {
    if (dt == 0) {
        return 0;
    }

    uint16_t year = (uint16_t)(2000u + bench_rtc_to_binary(dt->year));
    uint8_t month = bench_rtc_to_binary(dt->month);
    uint8_t day = bench_rtc_to_binary(dt->daymonth);
    uint8_t hour = bench_rtc_to_binary(dt->hour);
    uint8_t minute = bench_rtc_to_binary(dt->minutes);
    uint8_t second = bench_rtc_to_binary(dt->seconds);

    if (month < 1u) {
        month = 1u;
    } else if (month > 12u) {
        month = 12u;
    }

    if (day < 1u) {
        day = 1u;
    } else if (day > 31u) {
        day = 31u;
    }

    uint64_t days = bench_days_from_civil(year, month, day);
    return days * 86400u + (uint64_t)hour * 3600u + (uint64_t)minute * 60u + (uint64_t)second;
}

uint64_t bench_now_seconds(void) {
    DateTime now;
    getDateTime(&now);
    return bench_seconds_from_datetime(&now);
}

uint64_t bench_wait_for_next_second(void) {
    uint64_t current = bench_now_seconds();
    uint64_t next = current;

    do {
        next = bench_now_seconds();
    } while (next == current);

    return next;
}

bool bench_parse_duration(int argc, char **argv, uint64_t *duration_s, const char *usage_message) {
    if (duration_s == 0) {
        return false;
    }

    if (argc < 2) {
        *duration_s = BENCH_DEFAULT_DURATION_S;
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
            value = BENCH_MAX_DURATION_S;
            break;
        }
        value = value * 10 + digit;
        if (value > BENCH_MAX_DURATION_S) {
            value = BENCH_MAX_DURATION_S;
            break;
        }
    }

    if (value == 0) {
        value = BENCH_DEFAULT_DURATION_S;
    }

    *duration_s = value;
    return true;
}

uint64_t bench_ops_per_second(uint64_t operations, uint64_t elapsed_s) {
    if (elapsed_s == 0) {
        elapsed_s = 1;
    }
    return (operations + elapsed_s / 2u) / elapsed_s;
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

void bench_print_summary(const char *titulo, const char *detalle_operaciones, uint64_t elapsed_s, uint64_t operations, uint64_t ops_per_sec) {
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

    print("Duracion (s): ");
    printDec(elapsed_s);
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
