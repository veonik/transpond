#include "message.h"

size_t ackPack(metrics *m, char *buf);
size_t ackUnpack(metrics *m, char *buf);

size_t ac2Pack(metrics *m, char *buf);
size_t ac2Unpack(metrics *m, char *buf);

size_t dtPack(metrics *m, char *buf);
size_t dtUnpack(metrics *m, char *buf);

size_t logPack(metrics *m, char *buf);
size_t logUnpack(metrics *m, char *buf);

struct command {
    char request[3];
    packFn fn;
};

command cmds[8] = {
        command{ .request = "he", .fn = ackPack },
        command{ .request = "lo", .fn = ac2Pack },
        command{ .request = "up", .fn = dtPack },
        command{ .request = "tl", .fn = logPack },
        command{ .request = "a1", .fn = ackUnpack },
        command{ .request = "a2", .fn = ac2Unpack },
        command{ .request = "dt", .fn = dtUnpack },
        command{ .request = "gs", .fn = logUnpack }
};

packFn getCommand(const char *prefix) {
    for (size_t i = 0; i < 8; i++) {
        if (strncmp(cmds[i].request, prefix, 2) == 0) {
            return cmds[i].fn;
        }
    }
    return nullptr;
}

size_t ackUnpack(metrics *m, char *buf) {
    char *p = buf;
    p += 2;

    char *pm = m->b;
    memcpy(pm, p, 56);

    return (p + 56) - buf;
}

size_t ackPack(metrics *m, char *buf) {
    char *p = buf;
    *p++ = 'a';
    *p++ = '1';
    
    char *pm = m->b;
    memcpy(p, pm, 56);

    return (p + 56) - buf;
}


size_t ac2Unpack(metrics *m, char *buf) {
    char *p = buf;
    p += 2;

    char *pm = m->b;
    pm += 56;
    memcpy(pm, p, 56);

    return (p + 56) - buf;
}

size_t ac2Pack(metrics *m, char *buf) {
    char *p = buf;
    *p++ = 'a';
    *p++ = '2';

    char *pm = m->b;
    pm += 56;
    memcpy(p, pm, 56);

    return (p + 56) - buf;
}

size_t dtUnpack(metrics *m, char *buf) {
    char *p = buf;
    p += 2;

    char *pm = m->b;
    pm += 112;
    memcpy(pm, p, 8);

    return (p + 8) - buf;
}

size_t dtPack(metrics *m, char *buf) {
    char *p = buf;
    *p++ = 'd';
    *p++ = 't';

    char *pm = m->b;
    pm += 112;
    memcpy(p, pm, 8);

    return (p + 8) - buf;
}

size_t logUnpack(metrics *m, char *buf) {
    char *p = buf;
    p += 2;

    m->logging = *p++;

    Serial.print("data logging: ");
    Serial.println(m->logging == MODULE_ENABLED ? "enabled" : "disabled");

    return p - buf;
}

// TODO: Not happy about the ambiguous "toggle" semantics here
size_t logPack(metrics *m, char *buf) {
    char *p = buf;
    *p++ = 'g';
    *p++ = 's';

    m->logging = m->logging == MODULE_ENABLED ? MODULE_DISABLED : MODULE_ENABLED;
    // TODO: Format the FRAM?
    Serial.print("data logging: ");
    Serial.println(m->logging == MODULE_ENABLED ? "enabled" : "disabled");

    *p++ = m->logging;

    return p - buf;
}
