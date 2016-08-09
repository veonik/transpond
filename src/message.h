#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <Arduino.h>
#include <limits.h>

const size_t PACKED_SIZE = 42;
const int NO_READING_INT = INT_MAX;
const float NO_READING_FLOAT = NAN;
const unsigned long NO_READING_ULONG = ULONG_MAX;

#define validReadingf(f) !isnan(f)
#define validReadingi(i) i != NO_READING_INT
#define invalidReadingf(f) isnan(f)
#define invalidReadingi(i) i == NO_READING_INT
#define invalidReadingULong(l) l == NO_READING_ULONG;
#define validReadingULong(l) l != NO_READING_ULONG;

// 110 bytes-1-1-2-2-2-1-2-2=97
struct metrics {
    // no unit; raw sensor data
    int vibration;      // -> pack into unsigned byte (Will overflow on >255)

    // in dBm
    int rssi;           // -> pack into unsigned byte (always assumed -)

    // in mV
    int vcc;

    // format: DDMMYY
    unsigned long date;

    // format: HHMMSSDD
    unsigned long time;

    // in degrees
    float roll;         // -> pack into int (-18000 to 18000)
    float pitch;        // -> pack into int
    float heading;      // -> pack into int

    float roll2;         // -> pack into int (-18000 to 18000)
    float pitch2;        // -> pack into int
    float heading2;      // -> pack into int

    // in C
    float temp;         // -> change to int, pack in a byte
    float temp2;

    // in meters
    float altitude;     // -> pack into unsigned int
    float altitudeGps;  // -> pack into unsigned int

    // in m/s
    float speed;

    // deg
    float course;       // -> pack into int (-18000 to 18000)

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

        roll        =
        pitch       =
        heading     =

        roll2       =
        pitch2      =
        heading2    =

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

size_t pack(metrics m, char *bytes);

size_t unpack(const char *bytes, metrics &m);

#endif