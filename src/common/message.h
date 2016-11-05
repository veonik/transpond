#ifndef TRANSPOND_MESSAGE_H
#define TRANSPOND_MESSAGE_H

#include <Arduino.h>
#include <limits.h>

const int NO_READING_INT = INT_MAX;
const float NO_READING_FLOAT = NAN;
const unsigned long NO_READING_ULONG = ULONG_MAX;
const char MODULE_ENABLED = 'i';
const char MODULE_DISABLED = 'o';

#define validReadingf(f) !isnan(f)
#define validReadingi(i) i != NO_READING_INT
#define invalidReadingf(f) isnan(f)
#define invalidReadingi(i) i == NO_READING_INT

struct metrics {
    union {
        struct {
            // in dBm
            int rssi;

            // in mV
            int vcc;

            // in C
            float temp2;

            // in meters
            float altitudeGps;

            // deg
            float latitude;
            float longitude;

            // in m/s^2
            float accelX;
            float accelY;
            float accelZ;

            // in micro Teslas (uT)
            float magX;
            float magY;
            float magZ;

            // in rad/s
            float gyroX;
            float gyroY;
            float gyroZ;

            // --- 56 bytes

            // in m/s
            float speed;

            // deg
            float course;

            // in meters
            float altitude;

            // in C
            float temp;

            // in m/s^2
            float accel2X;
            float accel2Y;
            float accel2Z;

            // in micro Teslas (uT)
            float mag2X;
            float mag2Y;
            float mag2Z;

            // in rad/s
            float gyro2X;
            float gyro2Y;
            float gyro2Z;

            char logging = MODULE_DISABLED;
            char unused;

            // no unit; raw sensor data
            int vibration;

            // -- 56 bytes

            // format: DDMMYY
            unsigned long date;

            // format: HHMMSSDD
            unsigned long time;

        };
        char b[120];
    };

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

typedef size_t (*packFn)(metrics*, char*);

packFn getCommand(const char *prefix);


#endif