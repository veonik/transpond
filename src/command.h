#ifndef TRANSPOND_COMMAND_H
#define TRANSPOND_COMMAND_H

#include <Arduino.h>

class Command {
private:
    static union {
        int i;
        float f;
        long l;
        char b[4];
    } val;

protected:
    static char *packChar(char *buf, char b);
    static char *packInt(char *buf, int i);
    static char *packFloat(char *buf, float f);
    static char *packLong(char *buf, long l);

    static char *unpackChar(char *b, char *buf);
    static char *unpackInt(int *i, char *buf);
    static char *unpackFloat(float *f, char *buf);
    static char *unpackLong(long *l, char *buf);

public:
    Command() {}
    virtual size_t pack(char *buf) = 0;
    virtual size_t unpack(char *buf) = 0;
};


#endif //TRANSPOND_COMMAND_H
