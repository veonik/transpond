#ifndef TRANSPOND_FRAM_H
#define TRANSPOND_FRAM_H

#include <Adafruit_FRAM_I2C.h>

// Reading these tells us where we currently are positioned.
#define FRAM_CURRENT_POSITION_LOW 0x00
#define FRAM_CURRENT_POSITION_HIGH 0x01

#define FRAM_DATA_START 0x10   // Data starts at byte 16
#define FRAM_DATA_END   0x8000 // Data ends   at byte 32768

class FRAM {
private:
    int _pos = 0;
    Adafruit_FRAM_I2C _fram;

    uint16_t _increment(int);
    uint16_t _boundsCheck(uint16_t pos, uint16_t size);
    void _ensureFits(uint16_t pos, uint16_t size);

    union {
        float f;
        int i;
        char c;
        byte b[4];
    } _buffer;

    // Create a map and you can make a key-value memory

public:
    FRAM() {
        _fram = Adafruit_FRAM_I2C();
    };

    bool begin();

    bool format();

    uint16_t write(char);
    uint16_t write(int);
    uint16_t write(float);

    uint16_t read(uint16_t start, char *buf, uint16_t size);
    int readInt(uint16_t pos);
    char readChar(uint16_t pos);
    float readFloat(uint16_t pos);
};

#endif