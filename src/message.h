#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <Arduino.h>
#include <limits.h>

const size_t PACKED_SIZE = 42;
const int NO_READING_INT = INT_MAX;
const float NO_READING_FLOAT = NAN;

#define validReadingf(f) f != NO_READING_FLOAT
#define validReadingi(i) i != NO_READING_INT

struct metrics {
    // no unit; raw sensor data
    int lastVibration;

    // in dBm
    int lastRssi;

    // in mV
    int lastVcc;

    // in degrees
    float lastRoll;
    float lastPitch;
    float lastHeading;

    // in C
    float lastTemp;

    // in meters
    float lastAltitude;

    // in rad/s
    float lastGyroX;
    float lastGyroY;
    float lastGyroZ;

    void setNoReading() {
        lastVibration  = NO_READING_INT;
        lastRssi       = NO_READING_INT;
        lastVcc        = NO_READING_INT;
        lastRoll       = NO_READING_FLOAT;
        lastPitch      = NO_READING_FLOAT;
        lastHeading    = NO_READING_FLOAT;
        lastTemp       = NO_READING_FLOAT;
        lastAltitude   = NO_READING_FLOAT;
        lastGyroX      = NO_READING_FLOAT;
        lastGyroY      = NO_READING_FLOAT;
        lastGyroZ      = NO_READING_FLOAT;
    }

    metrics() {
        setNoReading();
    }
};

size_t pack(metrics m, char *bytes);

size_t unpack(const char *bytes, metrics &m);

#endif