#include "message.h"

union {
    float f;
    int i;
    char bytes[4];
} mval;

size_t pack(metrics m, char *bytes) {
    bytes[0] = 'a';
    bytes[1] = 'c';
    bytes[2] = 'k';
    
    mval.i = m.lastVibration;
    bytes[3] = mval.bytes[0];
    bytes[4] = mval.bytes[1];

    mval.i = m.lastRssi;
    bytes[5] = mval.bytes[0];
    bytes[6] = mval.bytes[1];

    mval.i = m.lastVcc;
    bytes[7] = mval.bytes[0];
    bytes[8] = mval.bytes[1];

    mval.f = m.lastRoll;
    bytes[9] = mval.bytes[0];
    bytes[10] = mval.bytes[1];
    bytes[11] = mval.bytes[2];
    bytes[12] = mval.bytes[3];

    mval.f = m.lastPitch;
    bytes[13] = mval.bytes[0];
    bytes[14] = mval.bytes[1];
    bytes[15] = mval.bytes[2];
    bytes[16] = mval.bytes[3];

    mval.f = m.lastHeading;
    bytes[17] = mval.bytes[0];
    bytes[18] = mval.bytes[1];
    bytes[19] = mval.bytes[2];
    bytes[20] = mval.bytes[3];

    mval.f = m.lastTemp;
    bytes[21] = mval.bytes[0];
    bytes[22] = mval.bytes[1];
    bytes[23] = mval.bytes[2];
    bytes[24] = mval.bytes[3];

    mval.f = m.lastAltitude;
    bytes[25] = mval.bytes[0];
    bytes[26] = mval.bytes[1];
    bytes[27] = mval.bytes[2];
    bytes[28] = mval.bytes[3];

    mval.f = m.lastGyroX;
    bytes[29] = mval.bytes[0];
    bytes[30] = mval.bytes[1];
    bytes[31] = mval.bytes[2];
    bytes[32] = mval.bytes[3];

    mval.f = m.lastGyroY;
    bytes[33] = mval.bytes[0];
    bytes[34] = mval.bytes[1];
    bytes[35] = mval.bytes[2];
    bytes[36] = mval.bytes[3];

    mval.f = m.lastGyroZ;
    bytes[37] = mval.bytes[0];
    bytes[38] = mval.bytes[1];
    bytes[39] = mval.bytes[2];
    bytes[40] = mval.bytes[3];

    bytes[41] = 0; // For good measure?
    return PACKED_SIZE;
}

size_t unpack(const char *bytes, metrics &m) {
    mval.bytes[0] = bytes[3];
    mval.bytes[1] = bytes[4];
    m.lastVibration = mval.i;

    mval.bytes[0] = bytes[5];
    mval.bytes[1] = bytes[6];
    m.lastRssi = mval.i;

    mval.bytes[0] = bytes[7];
    mval.bytes[1] = bytes[8];
    m.lastVcc = mval.i;

    mval.bytes[0] = bytes[9];
    mval.bytes[1] = bytes[10];
    mval.bytes[2] = bytes[11];
    mval.bytes[3] = bytes[12];
    m.lastRoll = mval.f;

    mval.bytes[0] = bytes[13];
    mval.bytes[1] = bytes[14];
    mval.bytes[2] = bytes[15];
    mval.bytes[3] = bytes[16];
    m.lastPitch = mval.f;

    mval.bytes[0] = bytes[17];
    mval.bytes[1] = bytes[18];
    mval.bytes[2] = bytes[19];
    mval.bytes[3] = bytes[20];
    m.lastHeading = mval.f;

    mval.bytes[0] = bytes[21];
    mval.bytes[1] = bytes[22];
    mval.bytes[2] = bytes[23];
    mval.bytes[3] = bytes[24];
    m.lastTemp = mval.f;

    mval.bytes[0] = bytes[25];
    mval.bytes[1] = bytes[26];
    mval.bytes[2] = bytes[27];
    mval.bytes[3] = bytes[28];
    m.lastAltitude = mval.f;

    mval.bytes[0] = bytes[29];
    mval.bytes[1] = bytes[30];
    mval.bytes[2] = bytes[31];
    mval.bytes[3] = bytes[32];
    m.lastGyroX = mval.f;

    mval.bytes[0] = bytes[33];
    mval.bytes[1] = bytes[34];
    mval.bytes[2] = bytes[35];
    mval.bytes[3] = bytes[36];
    m.lastGyroY = mval.f;

    mval.bytes[0] = bytes[37];
    mval.bytes[1] = bytes[38];
    mval.bytes[2] = bytes[39];
    mval.bytes[3] = bytes[40];
    m.lastGyroZ = mval.f;

    return PACKED_SIZE;
}