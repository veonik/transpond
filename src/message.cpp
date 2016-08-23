#include "message.h"

size_t AckCommand::unpack(char *buf) {
    char *p = buf;
    p += 3;

    char c;
    p = unpackChar(&c, p);
    m->vibration = (int) c;
    p = unpackChar(&c, p);
    m->rssi = (int) c;

    p = unpackInt(&m->vcc, p);

    p = unpackFloat(&m->latitude, p);
    p = unpackFloat(&m->longitude, p);
    int i;
    p = unpackInt(&i, p);
    m->temp = i / 100.0f;

    p = unpackInt(&i, p);
    m->altitude = i / 10.0f;

    p = unpackFloat(&m->altitudeGps, p);

    p = unpackFloat(&m->accelX, p);
    p = unpackFloat(&m->accelY, p);
    p = unpackFloat(&m->accelZ, p);

    p = unpackFloat(&m->magX, p);
    p = unpackFloat(&m->magY, p);
    p = unpackFloat(&m->magZ, p);

    p = unpackFloat(&m->gyroX, p);
    p = unpackFloat(&m->gyroY, p);
    p = unpackFloat(&m->gyroZ, p);

    return p - buf;
}

size_t AckCommand::pack(char *buf) {
    char *p = buf;
    p = packChar(p, 'a');
    p = packChar(p, 'c');
    p = packChar(p, 'k');
    p = packChar(p, (char) m->vibration);
    p = packChar(p, (char) m->rssi);
    p = packInt(p, m->vcc);
    p = packFloat(p, m->latitude);
    p = packFloat(p, m->longitude);
    p = packInt(p, (int) m->temp * 100);
    p = packInt(p, (int) round(m->altitude * 10));
    p = packFloat(p, m->altitudeGps);

    p = packFloat(p, m->accelX);
    p = packFloat(p, m->accelY);
    p = packFloat(p, m->accelZ);

    p = packFloat(p, m->magX);
    p = packFloat(p, m->magY);
    p = packFloat(p, m->magZ);

    p = packFloat(p, m->gyroX);
    p = packFloat(p, m->gyroY);
    p = packFloat(p, m->gyroZ);

    return p - buf;
}


size_t Ack2Command::unpack(char *buf) {
    char *p = buf;
    p += 3;

    int i;
    p = unpackInt(&i, p);
    m->temp2 = i / 100.0f;

    p = unpackFloat(&m->speed, p);
    p = unpackFloat(&m->course, p);

    p = unpackFloat(&m->accel2X, p);
    p = unpackFloat(&m->accel2Y, p);
    p = unpackFloat(&m->accel2Z, p);

    p = unpackFloat(&m->mag2X, p);
    p = unpackFloat(&m->mag2Y, p);
    p = unpackFloat(&m->mag2Z, p);

    p = unpackFloat(&m->gyro2X, p);
    p = unpackFloat(&m->gyro2Y, p);
    p = unpackFloat(&m->gyro2Z, p);

    return p - buf;
}

size_t Ack2Command::pack(char *buf) {
    char *p = buf;
    p = packChar(p, 'a');
    p = packChar(p, 'c');
    p = packChar(p, '2');

    p = packInt(p, (int) m->temp2 * 100);

    p = packFloat(p, m->speed);
    p = packFloat(p, m->course);

    p = packFloat(p, m->accel2X);
    p = packFloat(p, m->accel2Y);
    p = packFloat(p, m->accel2Z);

    p = packFloat(p, m->mag2X);
    p = packFloat(p, m->mag2Y);
    p = packFloat(p, m->mag2Z);

    p = packFloat(p, m->gyro2X);
    p = packFloat(p, m->gyro2Y);
    p = packFloat(p, m->gyro2Z);

    return p - buf;
}

