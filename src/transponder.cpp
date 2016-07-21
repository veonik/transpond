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

uint8_t piezoSensor = A0;

unsigned long lastMessage;

metrics m;

void initSensors() {
    if(!accel.begin()) {
        Serial.println(F("LSM303 not found"));
    }
    if(!mag.begin()) {
        Serial.println(F("LSM303 not found"));
    }
    if(!bmp.begin()) {
        Serial.println(F("BMP180 not found"));
    }
    if(!gyro.begin()) {
        Serial.println(F("L3GD20 not found"));
    }

    gyro.enableAutoRange(true);
    mag.enableAutoRange(true);
}

void onMessageReceived(Message *msg) {
    lastMessage = micros();
    m.lastRssi = msg->rssi;

    char ack[PACKED_SIZE];
    size_t s = pack(m, ack);

    Serial.print(F("sending "));
    Serial.print(s);
    Serial.println(F(" bytes"));
    Message resp(ack, s);
    radio->send(&resp);
}

void setup() {
    Serial.begin(38400);
    Serial.println(F("transponder"));
    radio = new CC1101Radio();
    radio->listen(onMessageReceived);
    initSensors();
}

void loop() {
    radio->tick();

    m.lastVibration = analogRead(piezoSensor);
    m.lastVcc = readVcc();

    sensors_event_t accel_event;
    sensors_event_t mag_event;
    sensors_event_t bmp_event;
    sensors_event_t gyro_event;
    sensors_vec_t   orientation;

    accel.getEvent(&accel_event);
    mag.getEvent(&mag_event);

    if (dof.fusionGetOrientation(&accel_event, &mag_event, &orientation)) {
        m.lastRoll = orientation.roll;
        m.lastPitch = orientation.pitch;
        m.lastHeading = orientation.heading;
#ifdef DEBUGV

        Serial.print(F("Orientation: "));
        Serial.print(orientation.roll);
        Serial.print(F(" "));
        Serial.print(orientation.pitch);
        Serial.print(F(" "));
        Serial.print(orientation.heading);
        Serial.println(F("   degs"));
#endif
    } else {
        m.lastRoll = NO_READING_FLOAT;
        m.lastPitch = NO_READING_FLOAT;
        m.lastHeading = NO_READING_FLOAT;
#ifdef DEBUGV

        Serial.println(F("Unable to read pitch/roll/heading"));
#endif
    }

    bmp.getEvent(&bmp_event);
    if (bmp_event.pressure) {
        float temperature;
        bmp.getTemperature(&temperature);
        m.lastTemp = temperature;
        m.lastAltitude = bmp.pressureToAltitude(SENSORS_PRESSURE_SEALEVELHPA, bmp_event.pressure, temperature);
#ifdef DEBUGV

        Serial.print(F("Alt: "));
        Serial.print(lastAltitude);
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

    gyro.getEvent(&gyro_event);
    if (gyro_event.gyro.status) {
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

#endif