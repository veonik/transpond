#ifndef TRANSPOND_COMMAND_H
#define TRANSPOND_COMMAND_H

#include <Arduino.h>
#include "message.h"

char *packChar(char *buf, char b);
char *packInt(char *buf, int i);
char *packFloat(char *buf, float f);
char *packLong(char *buf, long l);
char *packULong(char *buf, unsigned long l);

char *unpackChar(char *b, char *buf);
char *unpackInt(int *i, char *buf);
char *unpackFloat(float *f, char *buf);
char *unpackLong(long *l, char *buf);
char *unpackULong(unsigned long *l, char *buf);

#endif //TRANSPOND_COMMAND_H
