#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>

// Los semaforos se identifican por un string (su nombre). Todas las
// operaciones reciben ese nombre como id.

// Crea e inicializa el semaforo con el nombre dado y el valor inicial
// indicado. Devuelve 0 en exito, -1 si el nombre es invalido, ya existe un
// semaforo con ese nombre, o no hay lugar.
int sem_init(const char * name, int initialValue);

// Abre el semaforo con el nombre dado. Si no existe, lo crea con initialValue;
// si ya existe, simplemente lo abre (ignora initialValue). Devuelve 0 en
// exito, -1 si el nombre es invalido / no hay lugar.
int sem_open(const char * name, int initialValue);

// Incrementa (V / signal) el semaforo, despertando a un proceso en espera.
// Devuelve 0 en exito, -1 si no existe un semaforo con ese nombre.
int sem_post(const char * name);

// Decrementa (P / wait) el semaforo, bloqueando al proceso actual si el valor
// no es positivo. Devuelve 0 en exito, -1 si no existe ese semaforo.
int sem_wait(const char * name);

// Cierra el semaforo para el proceso actual. Cuando ningun proceso lo tiene
// abierto, se libera la entrada. Devuelve 0 en exito, -1 si no existe.
int sem_close(const char * name);

#endif
