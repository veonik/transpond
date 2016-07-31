#ifndef HANDSET

#include <Arduino.h>
#include <CC1101Radio.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_10DOF.h>

#include "vcc.h"
#include "message.h"

Radio *radio;

Adafruit_10DOF                dof   = Adafruit_10DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);

bool accelEnabled = true;
bool magEnabled = true;
bool bmpEnabled = true;
bool gyroEnabled = true;

uint8_t piezoSensor = A0;

unsigned long lastTick = 0;
unsigned long lastUpdate = 0;
unsigned long lastReceipt = 0;
int avgTickDelay;
int avgUpdateDelay;

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
}

int printOrientation;
int printOrientationI;

void onMessageReceived(Message *msg) {
    lastReceipt = micros();
    m.lastRssi = msg->rssi;

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
    m.lastVibration = analogRead(piezoSensor);
    m.lastVcc = readVcc();

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
        // TODO: Tilt compensation
        m.lastRoll = orientation.roll;
        m.lastPitch = orientation.pitch;
        m.lastHeading = orientation.heading;
#ifndef DEBUGV
        if (printOrientationI < printOrientation) {
            printOrientationI++;
#endif
        Serial.print(F("Orientation: "));
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
        m.lastRoll = NO_READING_FLOAT;
        m.lastPitch = NO_READING_FLOAT;
        m.lastHeading = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read pitch/roll/heading"));
#endif
    }

    if (bmpEnabled && bmp.getEvent(&bmp_event) && bmp_event.pressure) {
        float temperature;
        bmp.getTemperature(&temperature);
        m.lastTemp = temperature;
        m.lastAltitude = bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA, bmp_event.pressure, temperature);
#ifdef DEBUGV

        Serial.print(F("Alt: "));
        Serial.print(m.lastAltitude);
        Serial.println(F("m"));
        Serial.print(F("Temp: "));
        Serial.print(temperature);
        Serial.println(F("C"));
#endif
    } else {
        m.lastTemp = NO_READING_FLOAT;
        m.lastAltitude = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read temp/altitude"));
#endif
    }

    if (gyroEnabled && gyro.getEvent(&gyro_event)) {
        m.lastGyroX = gyro_event.gyro.x;
        m.lastGyroY = gyro_event.gyro.y;
        m.lastGyroZ = gyro_event.gyro.z;
#ifdef DEBUGV

        Serial.print(F("X: ")); Serial.print(gyro_event.gyro.x); Serial.print(F("  "));
        Serial.print(F("Y: ")); Serial.print(gyro_event.gyro.y); Serial.print(F("  "));
        Serial.print(F("Z: ")); Serial.print(gyro_event.gyro.z); Serial.print(F("  "));
        Serial.println(F("rad/s "));
#endif
    } else {
        m.lastGyroX = NO_READING_FLOAT;
        m.lastGyroY = NO_READING_FLOAT;
        m.lastGyroZ = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read gyro"));
#endif
    }
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
    float alpha = 0.01;
    unsigned long tick = millis();
    long diff = tick - lastTick;
    if (avgTickDelay > 0) {
        avgTickDelay = (int) round((alpha * diff) + ((1 - alpha) * avgTickDelay));
    } else {
        avgTickDelay = (int) diff;
    }
    lastTick = tick;

    radio->tick();

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

    if (Serial.available()) {
        char cmd = (char) Serial.read();
        if (cmd == 'I') {
            Serial.print(F("Average tick delay: "));
            Serial.print(avgTickDelay);
            Serial.println(F("ms"));
            Serial.print(F("Average update delay: "));
            Serial.print(avgUpdateDelay);
            Serial.println(F("ms"));
        } else if (cmd == 'P') {
            printOrientation = 100;
            printOrientationI = 0;
        }
    }
}

#endif