#ifndef TRANSPOND_COMMAND_H
#define TRANSPOND_COMMAND_H

#include <Arduino.h>

class Command {
private:
    union {
        int i;
        float f;
        long l;
        char b[4];
    } val;

protected:
    char *packChar(char *buf, char b);
    char *packInt(char *buf, int i);
    char *packFloat(char *buf, float f);
    char *packLong(char *buf, long l);

    char *unpackChar(char *b, char *buf);
    char *unpackInt(int *i, char *buf);
    char *unpackFloat(float *f, char *buf);
    char *unpackLong(long *l, char *buf);

public:
    Command() {}
    virtual size_t pack(char *buf) = 0;
    virtual size_t unpack(char *buf) = 0;
};


#endif //TRANSPOND_COMMAND_H
