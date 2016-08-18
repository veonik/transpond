#ifndef TRANSPOND_MESSAGE_H
#define TRANSPOND_MESSAGE_H

#include <Arduino.h>
#include <limits.h>
#include "command.h"

const int NO_READING_INT = INT_MAX;
const float NO_READING_FLOAT = NAN;
const unsigned long NO_READING_ULONG = ULONG_MAX;

#define validReadingf(f) !isnan(f)
#define validReadingi(i) i != NO_READING_INT
#define invalidReadingf(f) isnan(f)
#define invalidReadingi(i) i == NO_READING_INT

struct metrics {
    // no unit; raw sensor data
    int vibration;

    // in dBm
    int rssi;

    // in mV
    int vcc;

    // format: DDMMYY
    unsigned long date;

    // format: HHMMSSDD
    unsigned long time;

    // in C
    float temp;
    float temp2;

    // in meters
    float altitude;
    float altitudeGps;

    // in m/s
    float speed;

    // deg
    float course;

    // deg
    float latitude;
    float longitude;

    // in m/s^2
    float accelX;
    float accelY;
    float accelZ;

    float accel2X;
    float accel2Y;
    float accel2Z;

    // in micro Teslas (uT)
    float magX;
    float magY;
    float magZ;

    float mag2X;
    float mag2Y;
    float mag2Z;

    // in rad/s
    float gyroX;
    float gyroY;
    float gyroZ;

    float gyro2X;
    float gyro2Y;
    float gyro2Z;

    void setNoReading() {
        vibration   =
        rssi        =
        vcc         = NO_READING_INT;

        date        =
        time        = NO_READING_ULONG;

        temp        =
        temp2       =

        altitude    =
        altitudeGps =

        latitude    =
        longitude   =
        speed       =
        course      =

        accelX      =
        accelY      =
        accelZ      =

        accel2X     =
        accel2Y     =
        accel2Z     =

        magX        =
        magY        =
        magZ        =

        mag2X       =
        mag2Y       =
        mag2Z       =

        gyroX       =
        gyroY       =
        gyroZ       =

        gyro2X      =
        gyro2Y      =
        gyro2Z      = NO_READING_FLOAT;
    }

    metrics() {
        setNoReading();
    }
};

class AckCommand : public Command {
private:
    metrics *m;

public:
    AckCommand(metrics *m) : Command() {
        this->m = m;
    }

    size_t pack(char *buf);

    size_t unpack(char *buf);
};

class Ack2Command : public Command {
private:
    metrics *m;

public:
    Ack2Command(metrics *m) : Command() {
        this->m = m;
    }

    size_t pack(char *buf);

    size_t unpack(char *buf);
};

#endif