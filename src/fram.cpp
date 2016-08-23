#include "fram.h"

bool FRAM::begin() {
    if (_fram.begin()) {
        _pos = word(_fram.read8(FRAM_CURRENT_POSITION_HIGH), _fram.read8(FRAM_CURRENT_POSITION_LOW));
        if (_pos >= FRAM_DATA_START && _pos <= FRAM_DATA_END) {
            return true;
        }
        // Invalid position, clear everything!
        return format();
    }
}

/**
 * @return The previous position; useful after writing.
 */
uint16_t FRAM::_increment(int i) {
    uint16_t pos = _pos;
    _pos += i;
    if (_pos > FRAM_DATA_END) {
        _pos = FRAM_DATA_START;
    }
    _fram.write8(FRAM_CURRENT_POSITION_LOW, lowByte(_pos));
    _fram.write8(FRAM_CURRENT_POSITION_HIGH, highByte(_pos));
    return pos;
}

uint16_t FRAM::_boundsCheck(uint16_t pos, uint16_t size) {
    if (pos >= FRAM_DATA_START && pos+size <= FRAM_DATA_END) {
        return pos;
    }
    return FRAM_DATA_START;
}

void FRAM::_ensureFits(uint16_t pos, uint16_t size) {
    if (pos >= FRAM_DATA_START && pos+size <= FRAM_DATA_END) {
        return;
    }
    _pos = FRAM_DATA_START;
    _fram.write8(FRAM_CURRENT_POSITION_LOW, lowByte(_pos));
    _fram.write8(FRAM_CURRENT_POSITION_HIGH, highByte(_pos));
}

bool FRAM::format() {
    _pos = FRAM_DATA_START;
    _fram.write8(FRAM_CURRENT_POSITION_LOW, lowByte(_pos));
    _fram.write8(FRAM_CURRENT_POSITION_HIGH, highByte(_pos));
    return true;
}

uint16_t FRAM::write(char c) {
    _ensureFits(_pos, 1);
    uint16_t pos = _pos;
    _fram.write8(_pos, c);
    return _increment(1);
}

uint16_t FRAM::write(int i) {
    _ensureFits(_pos, 2);
    _fram.write8(_pos, lowByte(i));
    _fram.write8(_pos+1, highByte(i));
    return _increment(2);
}

uint16_t FRAM::write(float f) {
    _ensureFits(_pos, 4);
    _buffer.f = f;
    _fram.write8(_pos,   _buffer.b[0]);
    _fram.write8(_pos+1, _buffer.b[1]);
    _fram.write8(_pos+2, _buffer.b[2]);
    _fram.write8(_pos+3, _buffer.b[3]);
    return _increment(4);
}

uint16_t FRAM::write(unsigned long ul) {
    _ensureFits(_pos, 4);
    _buffer.ul = ul;
    _fram.write8(_pos,   _buffer.b[0]);
    _fram.write8(_pos+1, _buffer.b[1]);
    _fram.write8(_pos+2, _buffer.b[2]);
    _fram.write8(_pos+3, _buffer.b[3]);
    return _increment(4);
}

uint16_t FRAM::read(uint16_t start, char *buf, uint16_t size) {
    if (_boundsCheck(start, size) != start) {
        return 0;
    }
    for (uint16_t i = 0; i < size; i++) {
        *buf++ = _fram.read8(start++);
    }
    return size;
}

int FRAM::readInt(uint16_t pos) {
    read(pos, (char *)_buffer.b, 2);
    return _buffer.i;
}

char FRAM::readChar(uint16_t pos) {
    read(pos, (char *)_buffer.b, 1);
    return _buffer.c;
}

float FRAM::readFloat(uint16_t pos) {
    read(pos, (char *)_buffer.b, 4);
    return _buffer.f;
}

unsigned long FRAM::readULong(uint16_t pos) {
    read(pos, (char *)_buffer.b, 4);
    return _buffer.ul;
}