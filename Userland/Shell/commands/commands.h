#ifndef help_h
#define help_h
#include <inout.h>
#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"
#include <str.h>

int commandDispatcher(char ** argsv, int argsc);

int help(char ** argv, int argc);
int echo(char ** argv, int argc);
int tronBeta(char ** argv, int argc);
int tronGame(char ** argv, int argc);
int bounce(char ** argv, int argc);
int shutdown(char ** argv, int argc);
int time(char ** argv, int argc);
int fps(char ** argv, int argc);
int regs(char ** argv, int argc);
int speed(char ** argv, int argc);
int resize(char ** argv, int argc);
int show(char ** argv, int argc);
int benchfloat(char ** argv, int argc);
int benchhw(char ** argv, int argc);
int benchmem(char ** argv, int argc);
int benchkbd(char ** argv, int argc);

/* Para probar excepciones, borrarse (junto con test.c) cuando se terminen de validar */
int dividezero();
int invalidop();


#endif