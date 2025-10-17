#ifndef help_h
#define help_h
#include <inout.h>

int help(char ** argv, int argc);
int echo(char ** argv, int argc);
int tron(char ** argv, int argc);
int shutdown();
int getTime();                          //ver

/* Para probar excepciones, borrarse (junto con test.c) cuando se terminen de validar */
int dividezero();
int invalidop();

#endif