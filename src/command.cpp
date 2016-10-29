#include "command.h"

union {
    int i;
    float f;
    long l;
    unsigned long ul;
    char b[4];
} val;

char *packChar(char *buf, char b) {
    *buf++ = b;
    return buf;
}

char *packInt(char *buf, int i) {
    val.i = i;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    return buf;
}

char *packFloat(char *buf, float f) {
    val.f = f;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    *buf++ = val.b[2];
    *buf++ = val.b[3];
    return buf;
}

char *packLong(char *buf, long l) {
    val.l = l;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    *buf++ = val.b[2];
    *buf++ = val.b[3];
    return buf;
}

char *packULong(char *buf, unsigned long ul) {
    val.ul = ul;
    *buf++ = val.b[0];
    *buf++ = val.b[1];
    *buf++ = val.b[2];
    *buf++ = val.b[3];
    return buf;
}

char *unpackChar(char *b, char *buf) {
    *b = *buf++;
    return buf;
}

char *unpackInt(int *i, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    *i = val.i;
    return buf;
}

char *unpackFloat(float *f, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    val.b[2] = *buf++;
    val.b[3] = *buf++;
    *f = val.f;
    return buf;
}

char *unpackLong(long *l, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    val.b[2] = *buf++;
    val.b[3] = *buf++;
    *l = val.l;
    return buf;
}

char *unpackULong(unsigned long *ul, char *buf) {
    val.b[0] = *buf++;
    val.b[1] = *buf++;
    val.b[2] = *buf++;
    val.b[3] = *buf++;
    *ul = val.ul;
    return buf;
}