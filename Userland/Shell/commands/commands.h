#ifndef help_h
#define help_h
#include <inout.h>
#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"

int help(char ** argv, int argc);
int echo(char ** argv, int argc);
int tron(char ** argv, int argc);
int bounce(char ** argv, int argc);
int shutdown(char ** argv, int argc);
int time(char ** argv, int argc);
int fps(char ** argv, int argc);
int regs(char ** argv, int argc);

/* Para probar excepciones, borrarse (junto con test.c) cuando se terminen de validar */
int dividezero();
int invalidop();


#endif