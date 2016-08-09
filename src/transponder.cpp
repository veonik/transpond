#ifndef HANDSET

#include <Arduino.h>

#include <CC1101Radio.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_10DOF.h>
#include <Adafruit_BNO055.h>

#include <gSoftSerial.h>
#include <TinyGPS++.h>

#include "util.h"
#include "message.h"
#include "gps.h"
#include "fram.h"

Radio *radio;

Adafruit_10DOF                dof   = Adafruit_10DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);
Adafruit_BNO055               bno   = Adafruit_BNO055(55);

gSoftSerial gpsSerial = gSoftSerial(5, 4);
TinyGPSPlus gps;

FRAM fram = FRAM();

bool accelEnabled = true;
bool magEnabled = true;
bool bmpEnabled = true;
bool gyroEnabled = true;
bool bnoEnabled = true;
bool framEnabled = true;

uint8_t piezoSensor = A0;

unsigned long lastTick = 0;
unsigned long lastUpdate = 0;
unsigned long lastWrite = 0;
unsigned long lastReceipt = 0;
int avgTickDelay;
int avgUpdateDelay;

const long WRITE_WAIT = 500;    // in ms
const long UPDATE_WAIT = 50;    // in ms

metrics m;

void initSensors() {
    if (!accel.begin()) {
        Serial.println(F("LSM303 not found"));
        accelEnabled = false;
    }
    if (!mag.begin()) {
        Serial.println(F("LSM303 not found"));
        magEnabled = false;
    } else {
        mag.enableAutoRange(true);
    }
    if (!bmp.begin()) {
        Serial.println(F("BMP180 not found"));
        bmpEnabled = false;
    }
    // TODO: gyro.begin() never returns if the gyro is not connected.
    if ((!magEnabled && !accelEnabled && !bmpEnabled) || !gyro.begin()) {
        Serial.println(F("L3GD20 not found"));
        gyroEnabled = false;
    } else {
        gyro.enableAutoRange(true);
    }
    if (!bno.begin()) {
        Serial.println(F("BNO055 not found"));
        bnoEnabled = false;
    }
    if (!fram.begin()) {
        Serial.println(F("FRAM not found"));
        framEnabled = false;
    } else {
        fram.format();
    }

    gpsSerial.begin(9600);
    gpsSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    gpsSerial.println(PMTK_SET_NMEA_UPDATE_5HZ);
    gpsSerial.println(PMTK_API_SET_FIX_CTL_5HZ);
}

int printOrientation;
int printOrientationI;
int printGps;
int printGpsI;

void onMessageReceived(Message *msg) {
    lastReceipt = micros();
    m.rssi = msg->rssi;

    char ack[PACKED_SIZE];
    size_t s = pack(m, ack);

#ifdef DEBUGV
    Serial.print(F("sending "));
    Serial.print(s);
    Serial.println(F(" bytes"));
#endif

    Message resp(ack, s);
    radio->send(&resp);
}

void update() {
    m.vibration = analogRead(piezoSensor);
    m.vcc = readVcc();

    sensors_event_t accel_event;
    sensors_event_t mag_event;
    sensors_event_t bmp_event;
    sensors_event_t gyro_event;
    sensors_vec_t   orientation;
    if (accelEnabled
        && magEnabled
        && accel.getEvent(&accel_event)
        && mag.getEvent(&mag_event)
        && dof.fusionGetOrientation(&accel_event, &mag_event, &orientation)
    ) {
        m.accelX = accel_event.acceleration.x;
        m.accelY = accel_event.acceleration.y;
        m.accelZ = accel_event.acceleration.z;
        m.magX = mag_event.magnetic.x;
        m.magY = mag_event.magnetic.y;
        m.magZ =mag_event.magnetic.z;

        // TODO: Tilt compensation
        m.roll = orientation.roll;
        m.pitch = orientation.pitch;
        m.heading = orientation.heading;
#ifndef DEBUGV
        if (printOrientationI < printOrientation) {
            printOrientationI++;
#endif
        Serial.print(F("Orientation :\t"));
        Serial.print(orientation.roll);
        Serial.print(F("\t"));
        Serial.print(orientation.pitch);
        Serial.print(F("\t"));
        Serial.print(orientation.heading);
        Serial.println(F("\tdegs"));
#ifndef DEBUGV
        }
#endif
    } else {
        m.roll = NO_READING_FLOAT;
        m.pitch = NO_READING_FLOAT;
        m.heading = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read pitch/roll/heading"));
#endif
    }

    if (bmpEnabled && bmp.getEvent(&bmp_event) && bmp_event.pressure) {
        float temperature;
        bmp.getTemperature(&temperature);
        m.temp = temperature;
        m.altitude = bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA, bmp_event.pressure, temperature);
#ifdef DEBUGV

        Serial.print(F("Alt: "));
        Serial.print(m.altitude);
        Serial.println(F("m"));
        Serial.print(F("Temp: "));
        Serial.print(temperature);
        Serial.println(F("C"));
#endif
    } else {
        m.temp = NO_READING_FLOAT;
        m.altitude = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read temp/altitude"));
#endif
    }

    if (gyroEnabled && gyro.getEvent(&gyro_event)) {
        m.gyroX = gyro_event.gyro.x;
        m.gyroY = gyro_event.gyro.y;
        m.gyroZ = gyro_event.gyro.z;
#ifdef DEBUGV

        Serial.print(F("X: ")); Serial.print(gyro_event.gyro.x); Serial.print(F("  "));
        Serial.print(F("Y: ")); Serial.print(gyro_event.gyro.y); Serial.print(F("  "));
        Serial.print(F("Z: ")); Serial.print(gyro_event.gyro.z); Serial.print(F("  "));
        Serial.println(F("rad/s "));
#endif
    } else {
        m.gyroX = NO_READING_FLOAT;
        m.gyroY = NO_READING_FLOAT;
        m.gyroZ = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read gyro"));
#endif
    }

    if (bnoEnabled) {
        imu::Vector<3> bno_accel = bno.getVector(bno.VECTOR_ACCELEROMETER);
        imu::Vector<3> bno_gyro = bno.getVector(bno.VECTOR_GYROSCOPE);
        imu::Vector<3> bno_mag = bno.getVector(bno.VECTOR_MAGNETOMETER);
        m.accel2X = bno_accel.x();
        m.accel2Y = bno_accel.y();
        m.accel2Z = bno_accel.z();
        m.mag2X = bno_mag.x();
        m.mag2Y = bno_mag.y();
        m.mag2Z = bno_mag.z();
        m.gyro2X = bno_gyro.x();
        m.gyro2Y = bno_gyro.y();
        m.gyro2Z = bno_gyro.z();
        m.temp2 = bno.getTemp();

        accel_event.acceleration.x = m.accel2X;
        accel_event.acceleration.y = m.accel2Y;
        accel_event.acceleration.z = m.accel2Z;
        mag_event.magnetic.x = m.mag2X;
        mag_event.magnetic.y = m.mag2Y;
        mag_event.magnetic.z = m.mag2Z;

        dof.fusionGetOrientation(&accel_event, &mag_event, &orientation);
        m.roll2 = orientation.roll;
        m.pitch2 = orientation.pitch;
        m.heading2 = orientation.heading;

#ifndef DEBUGV
        if (printOrientationI < printOrientation) {
#endif
            Serial.print(F("Orientation2:\t"));
            Serial.print(orientation.roll);
            Serial.print(F("\t"));
            Serial.print(orientation.pitch);
            Serial.print(F("\t"));
            Serial.print(orientation.heading);
            Serial.println(F("\tdegs"));
#ifndef DEBUGV
        }
#endif
    }

    if (gps.location.isUpdated()) {
        m.altitudeGps  = (float) gps.altitude.meters();
        m.latitude  = (float) gps.location.lat();
        m.longitude = (float) gps.location.lng();
        m.date = gps.date.value();
        m.time = gps.time.value();
#ifndef DEBUGV
        if (printGpsI < printGps) {
            printGpsI++;
#endif
            Serial.print(F("Location: "));
            if (gps.location.isValid()) {
                Serial.print(gps.location.lat(), 6);
                Serial.print(F(","));
                Serial.print(gps.location.lng(), 6);
            } else {
                Serial.print(F("INVALID"));
            }

            Serial.print(F("  Date/Time: "));
            if (gps.date.isValid()) {
                Serial.print(gps.date.month());
                Serial.print(F("/"));
                Serial.print(gps.date.day());
                Serial.print(F("/"));
                Serial.print(gps.date.year());
            } else {
                Serial.print(F("INVALID"));
            }

            Serial.print(F(" "));
            if (gps.time.isValid()) {
                if (gps.time.hour() < 10) Serial.print(F("0"));
                Serial.print(gps.time.hour());
                Serial.print(F(":"));
                if (gps.time.minute() < 10) Serial.print(F("0"));
                Serial.print(gps.time.minute());
                Serial.print(F(":"));
                if (gps.time.second() < 10) Serial.print(F("0"));
                Serial.print(gps.time.second());
                Serial.print(F("."));
                if (gps.time.centisecond() < 10) Serial.print(F("0"));
                Serial.print(gps.time.centisecond());
            } else {
                Serial.print(F("INVALID"));
            }

            Serial.println();
#ifndef DEBUGV
        }
#endif
    }
}

void log() {
    if (!framEnabled) {
        return;
    }

    fram.write((int) millis());
    fram.write(m.vcc);
}

void setup() {
    Serial.begin(38400);
    Serial.println(F("transponder"));
    radio = new CC1101Radio();
    radio->listen(onMessageReceived);
    initSensors();
}

void loop() {
    // TODO: alpha defines how much weight each value in the exponential average
    // has, and therefore its impact in the average.
    float alpha = 0.05;
    unsigned long tick = millis();
    long diff = tick - lastTick;
    if (avgTickDelay > 0) {
        avgTickDelay = (int) round((alpha * diff) + ((1 - alpha) * avgTickDelay));
    } else {
        avgTickDelay = (int) diff;
    }
    lastTick = tick;
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    diff = tick - lastUpdate;
    if (diff >= UPDATE_WAIT) {
        if (avgUpdateDelay > 0) {
            avgUpdateDelay = (int) round((alpha * diff) + ((1 - alpha) * avgUpdateDelay));
        } else {
            avgUpdateDelay = (int) diff;
        }
        lastUpdate = tick;
        update();
    }

    radio->tick();

    diff = tick - lastWrite;
    if (diff >= WRITE_WAIT) {
        lastWrite = tick;
        log();
    }

    if (Serial.available()) {
        char cmd = (char) Serial.read();
        if (cmd == 'I') {
            Serial.print(F("Average tick delay: "));
            Serial.print(avgTickDelay);
            Serial.println(F("ms"));
            Serial.print(F("Average update delay: "));
            Serial.print(avgUpdateDelay);
            Serial.println(F("ms"));
            Serial.print(F("RAM free: "));
            Serial.print(freeRam());
            Serial.println(F(" bytes"));
        } else if (cmd == 'P') {
            printOrientation = 100;
            printOrientationI = 0;
        } else if (cmd == 'G') {
            printGps = 5;
            printGpsI = 0;
        } else if (cmd == 'F') {
            for (uint16_t pos = FRAM_DATA_START; pos < FRAM_DATA_START+40; pos += 4) {
                Serial.print("VCC at offset ");
                Serial.print(fram.readInt(pos));
                Serial.print(": ");
                Serial.println(fram.readInt(pos+2));
            }
        }
    }
}

#endif