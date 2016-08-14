#include "command.h"

char *Command::packChar(char *buf, char b) {
    *buf++ = b;
    return buf;
}

char *Command::packInt(char *buf, int i) {
    val.i = i;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    return buf;
}

char *Command::packFloat(char *buf, float f) {
    val.f = f;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    *buf++ = val.b[2];
    *buf++ = val.b[3];
    return buf;
}

char *Command::packLong(char *buf, long l) {
    val.l = l;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    *buf++ = val.b[2];
    *buf++ = val.b[3];
    return buf;
}

char *Command::unpackChar(char *b, char *buf) {
    *b = *buf++;
    return buf;
}

char *Command::unpackInt(int *i, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    *i = val.i;
    return buf;
}

char *Command::unpackFloat(float *f, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    val.b[2] = *buf++;
    val.b[3] = *buf++;
    *f = val.f;
    return buf;
}


char *Command::unpackLong(long *l, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    val.b[2] = *buf++;
    val.b[3] = *buf++;
    *l = val.l;
    return buf;
}
