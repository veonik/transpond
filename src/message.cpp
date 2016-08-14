#include "message.h"

extern metrics m;

union {
    float f;
    int i;
    unsigned int ui;
    char b[4];
} mval;

size_t AckCommand::unpack(char *buf) {
    int i = 3;

    m.vibration = (int) buf[i++];

    m.rssi = -1 * (int) buf[i++];

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    m.vcc = mval.i;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.latitude = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.longitude = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    m.temp = mval.i / 100.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    m.temp2 = mval.i / 100.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    m.altitude = mval.ui / 10.0f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.altitudeGps = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyroX = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyroY = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyroZ = mval.f;

    return i;
}

size_t AckCommand::pack(char *buf) {
    int i = 0;
    buf[i++] = 'a';
    buf[i++] = 'c';
    buf[i++] = 'k';

    buf[i++] = (char) m.vibration;

    buf[i++] = (char) abs(m.rssi);

    mval.i = m.vcc;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];

    mval.f = m.latitude;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.longitude;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.i = (int) round(m.temp * 100);
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];

    mval.i = (int) round(m.temp2 * 100);
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];

    mval.ui = (unsigned int) round(m.altitude * 10);
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];

    mval.f = m.altitudeGps;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accelX;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accelY;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accelZ;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.magX;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.magY;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.magZ;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyroX;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyroY;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyroZ;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3]; // 58+3

    return i;
}


size_t Ack2Command::unpack(char *buf) {
    int i = 3;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.speed = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.course = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.accel2X = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.accel2Y = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.accel2Z = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.mag2X = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.mag2Y = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.mag2Z = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyro2X = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyro2Y = mval.f;

    mval.b[0] = buf[i++];
    mval.b[1] = buf[i++];
    mval.b[2] = buf[i++];
    mval.b[3] = buf[i++];
    m.gyro2Z = mval.f;

    return i;
}

size_t Ack2Command::pack(char *buf) {
    int i = 0;
    buf[i++] = 'a';
    buf[i++] = 'c';
    buf[i++] = '2';

    mval.f = m.speed;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.course;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accel2X;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accel2Y;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.accel2Z;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.mag2X;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.mag2Y;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.mag2Z;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyro2X;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyro2Y;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3];

    mval.f = m.gyro2Z;
    buf[i++] = mval.b[0];
    buf[i++] = mval.b[1];
    buf[i++] = mval.b[2];
    buf[i++] = mval.b[3]; // 44

    return i;
}

