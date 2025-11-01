#include <commands.h>

extern void dividezero_test();
extern void invalidopcode_test();

int dividezero() {
    print("Dividiendo por cero...\n");
    dividezero_test();
    return 0;
}

int invalidop() {
    print("Ejecutando operacion incorrecta");
    invalidopcode_test();
    return 0;
}