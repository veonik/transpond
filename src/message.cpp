#include "message.h"
#include "command.h"

size_t ackPack(metrics *m, char *buf);
size_t ackUnpack(metrics *m, char *buf);

size_t ac2Pack(metrics *m, char *buf);
size_t ac2Unpack(metrics *m, char *buf);

size_t dtPack(metrics *m, char *buf);
size_t dtUnpack(metrics *m, char *buf);

size_t logPack(metrics *m, char *buf);
size_t logUnpack(metrics *m, char *buf);

struct command {
    char request[5];
    char response[4];
    packFn pack;
    packFn unpack;
};

command cmds[4] = {
        command{ .request = "helo", .response = "ack", .pack = ackPack, .unpack = ackUnpack },
        command{ .request = "infx", .response = "ac2", .pack = ac2Pack, .unpack = ac2Unpack },
        command{ .request = "dtup", .response = "dt ", .pack = dtPack,  .unpack = dtUnpack },
        command{ .request = "tlog", .response = "log", .pack = logPack,  .unpack = logUnpack }
};

packFn getCommand(const char *prefix) {
    for (size_t i = 0; i < sizeof(cmds); i++) {
        if (strncmp(cmds[i].request, prefix, 4) == 0) {
            return cmds[i].pack;
        } else if (strncmp(cmds[i].response, prefix, 3) == 0) {
            return cmds[i].unpack;
        }
    }
    return NULL;
}

size_t ackUnpack(metrics *m, char *buf) {
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

size_t ackPack(metrics *m, char *buf) {
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


size_t ac2Unpack(metrics *m, char *buf) {
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

    p = unpackChar(&m->logging, p);

    return p - buf;
}

size_t ac2Pack(metrics *m, char *buf) {
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

    p = packChar(p, m->logging);

    return p - buf;
}


size_t dtUnpack(metrics *m, char *buf) {
    char *p = buf;
    p += 3;

    unsigned long ul;
    p = unpackULong(&ul, p);
    m->date = ul;

    p = unpackULong(&ul, p);
    m->time = ul;

    Serial.print("updated date and time: ");
    Serial.print(m->date);
    Serial.print(" ");
    Serial.println(m->time);

    return p - buf;
}

size_t dtPack(metrics *m, char *buf) {
    char *p = buf;
    p = packChar(p, 'd');
    p = packChar(p, 't');
    p = packChar(p, ' ');

    p = packULong(p, m->date);
    p = packULong(p, m->time);

    return p - buf;
}

size_t logUnpack(metrics *m, char *buf) {
    char *p = buf;
    p += 3;

    p = unpackChar(&m->logging, p);

    Serial.print("data logging: ");
    Serial.println(m->logging == 'i' ? "enabled" : "disabled");

    return p - buf;
}

// TODO: Not happy about the ambiguous "toggle" semantics here
size_t logPack(metrics *m, char *buf) {
    char *p = buf;
    p = packChar(p, 'l');
    p = packChar(p, 'o');
    p = packChar(p, 'g');

    m->logging = m->logging == 'i' ? 'o' : 'i';
    // TODO: Format the FRAM?
    Serial.print("data logging: ");
    Serial.println(m->logging == 'i' ? "enabled" : "disabled");

    p = packChar(p, m->logging);

    return p - buf;
}