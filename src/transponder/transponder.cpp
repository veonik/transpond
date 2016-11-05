#ifndef HANDSET

#include <Arduino.h>

// TODO: improved i2c library for Teensy
//#ifdef __MK64FX512__
//#include <i2c_t3.h>
//#define Wire TwoWire
//#endif

#include <CC1101Radio.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_BNO055.h>

#ifndef __MK64FX512__
#include <gSoftSerial.h>
#endif

#include <TinyGPS++.h>

#include "common/util.h"
#include "common/message.h"
#include "gps.h"
#include "fram.h"

Radio *radio;

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_BMP085_Unified       bmp   = Adafruit_BMP085_Unified(18001);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);
Adafruit_BNO055               bno   = Adafruit_BNO055(55);

#ifndef __MK64FX512__
gSoftSerial gpsSerial = gSoftSerial(5, 4);
#else
HardwareSerial gpsSerial = Serial1;
#endif
TinyGPSPlus gps;

FRAM fram = FRAM();

bool accelEnabled = true;
bool magEnabled = true;
bool bmpEnabled = true;
bool gyroEnabled = true;
bool bnoEnabled = true;
bool framStarted = false;
bool radioEnabled = true;

uint8_t piezoSensor = A0;

unsigned long lastTick = 0;
unsigned long lastUpdate = 0;
unsigned long lastGpsUpdate = 0;
unsigned long lastWrite = 0;
unsigned long lastReceipt = 0;
long avgTickDelay;
long avgUpdateDelay;

const long WRITE_WAIT = 250;    // in ms
const long UPDATE_WAIT = 64;    // in ms
const long GPS_WAIT = 100;    // in ms

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
    if (!bno.begin(Adafruit_BNO055::OPERATION_MODE_AMG)) {
        Serial.println(F("BNO055 not found"));
        bnoEnabled = false;
    }
    if (!fram.begin()) {
        Serial.println(F("FRAM not found"));
    } else {
        framStarted = true;
    }

    gpsSerial.begin(9600);
    gpsSerial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    gpsSerial.println(PMTK_SET_NMEA_UPDATE_5HZ);
    gpsSerial.println(PMTK_API_SET_FIX_CTL_5HZ);
}

int printGps;
int printGpsI;

void onMessageReceived(Message *msg) {
    lastReceipt = micros();
    m.rssi = msg->rssi;
    size_t s;
    char ack[61];
    packFn cmd = getCommand(msg->getBody());
    if (cmd == NULL) {
        Serial.print(F("unknown command received: "));
        Serial.println(msg->getBody());
        return;
    }

    s = cmd(&m, ack);

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
    if (accelEnabled && accel.getEvent(&accel_event)) {
        m.accelX = accel_event.acceleration.x;
        m.accelY = accel_event.acceleration.y;
        m.accelZ = accel_event.acceleration.z;

    } else {
        m.accelX = NO_READING_FLOAT;
        m.accelY = NO_READING_FLOAT;
        m.accelZ = NO_READING_FLOAT;
    }

    if (magEnabled && mag.getEvent(&mag_event)) {
        m.magX = mag_event.magnetic.x;
        m.magY = mag_event.magnetic.y;
        m.magZ = mag_event.magnetic.z;
    } else {
        m.magX = NO_READING_FLOAT;
        m.magY = NO_READING_FLOAT;
        m.magZ = NO_READING_FLOAT;
    }

    if (bmpEnabled && bmp.getEvent(&bmp_event)) {
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
        m.accel2X = (float) bno_accel.x();
        m.accel2Y = (float) bno_accel.y();
        m.accel2Z = (float) bno_accel.z();
        m.mag2X = (float) bno_mag.x();
        m.mag2Y = (float) bno_mag.y();
        m.mag2Z = (float) bno_mag.z();
        m.gyro2X = (float) bno_gyro.x();
        m.gyro2Y = (float) bno_gyro.y();
        m.gyro2Z = (float) bno_gyro.z();
        m.temp2 = bno.getTemp();
    }
}

void updateGps() {
    if (gps.time.isUpdated() && gps.time.isValid()) {
        m.time = gps.time.value();
    }
    if (gps.date.isUpdated() && gps.date.isValid()) {
        m.date = gps.date.value();
    }
    float alt;
    if (gps.altitude.isUpdated() && gps.altitude.isValid()
        // TODO: Sometimes the reading is still garbage, even if isValid()
        // TODO: This assumes altitude is higher than 10 meters.
        && (alt = (float) gps.altitude.meters()) > 10.0
            ) {
        m.altitudeGps  = alt;
    }
    if (gps.speed.isUpdated() && gps.speed.isValid()) {
        m.speed = (float) gps.speed.mps();
    }
    if (gps.course.isUpdated() && gps.course.isValid()) {
        m.course = (float) gps.course.deg();
    }
    float lat, lng;
    if (gps.location.isUpdated() && gps.location.isValid()
        // TODO: Sometimes the reading is still garbage, even if isValid()
        && abs(lat = (float) gps.location.lat()) > 1.0
        && abs(lng = (float) gps.location.lng()) > 1.0
            ) {
        if (lng > 0) {
            // TODO: Sometimes the reading is signed oppositely, even if isValid()
            // TODO: This assumes I'm in the northern hemisphere.
            lng *= -1;
        }
        if (lat < 0) {
            // TODO: Sometimes the reading is signed oppositely, even if isValid()
            // TODO: This assumes I'm in the western hemisphere.
            lat *= -1;
        }
        m.latitude  = lat;
        m.longitude = lng;

#ifndef DEBUGV
        if (printGpsI < printGps) {
            printGpsI++;
#endif
            Serial.print(F("Location: "));
            Serial.print(m.latitude, 6);
            Serial.print(F(","));
            Serial.println(m.longitude, 6);

            Serial.print(F("Date: "));
            Serial.println(m.date);
            Serial.print(F("Time: "));
            Serial.println(m.time);

            Serial.println();
#ifndef DEBUGV
        }
#endif
    }
}

void log() {
    if (m.logging != MODULE_ENABLED) {
        return;
    }

    if (! (fram.write(millis()) && fram.write(m.latitude)
          && fram.write(m.longitude) && fram.write(m.altitude)
          && fram.write(m.altitudeGps) && fram.write(m.speed))
    ) {
        m.logging = MODULE_DISABLED;
        Serial.println(F("FRAM disabled, storage full."));
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
    unsigned long tick = millis();
    long diff = tick - lastTick;
    avgTickDelay = expAvg(avgTickDelay, diff);
    lastTick = tick;

    diff = tick - lastUpdate;
    if (diff >= UPDATE_WAIT) {
        avgUpdateDelay = expAvg(avgUpdateDelay, diff);
        lastUpdate = tick;
        update();
    }

    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    diff = tick - lastGpsUpdate;
    if (diff >= GPS_WAIT) {
        lastGpsUpdate = tick;
        updateGps();
    }

    if (radioEnabled) {
        radio->tick();
    }

    diff = tick - lastWrite;
    if (diff >= WRITE_WAIT) {
        lastWrite = tick;
        log();
    }

    if (Serial.available()) {
        char cmd = (char) Serial.read();
        if (cmd == 'I') {
            Serial.print(F("Radio is "));
            Serial.println(radioEnabled ? "enabled" : "disabled");
            Serial.print(F("Logging is "));
            Serial.println(m.logging == MODULE_ENABLED ? "enabled" : "disabled");
            Serial.print(F("Average tick delay: "));
            Serial.print(avgTickDelay);
            Serial.println(F("ms"));
            Serial.print(F("Average update delay: "));
            Serial.print(avgUpdateDelay);
            Serial.println(F("ms"));
            Serial.print(F("RAM free: "));
            Serial.print(freeRam());
            Serial.println(F(" bytes"));
        } else if (cmd == 'G') {
            printGps = 5;
            printGpsI = 0;
        } else if (cmd == 'F') {
            for (uint16_t pos = FRAM_DATA_START; pos < fram.pos(); pos += 24) {
                char tab = '\t';
                Serial.print(fram.readULong(pos));
                Serial.print(tab);
                Serial.print(fram.readFloat(pos+4), 6);
                Serial.print(tab);
                Serial.print(fram.readFloat(pos+8), 6);
                Serial.print(tab);
                Serial.print(fram.readFloat(pos+12), 6);
                Serial.print(tab);
                Serial.print(fram.readFloat(pos+16), 6);
                Serial.print(tab);
                Serial.println(fram.readFloat(pos+20), 6);
            }
        } else if (cmd == 'S') {
            if (m.logging != MODULE_ENABLED) {
                if (framStarted) {
                    m.logging = MODULE_ENABLED;
                    Serial.println(F("FRAM logging enabled."));
                } else {
                    Serial.println(F("FRAM not started; unable to enable logging."));
                }
            } else {
                m.logging = MODULE_DISABLED;
                Serial.println(F("FRAM logging disabled."));
            }
        } else if (cmd == 'D') {
            if (framStarted) {
                if (fram.format()) {
                    Serial.println(F("FRAM formatted."));
                } else {
                    Serial.println(F("Unable to format FRAM."));
                }
            } else {
                Serial.println(F("FRAM not started; unable to format."));
            }
        } else if (cmd == 'R') {
            radioEnabled = !radioEnabled;
            if (radioEnabled) {
                Serial.println(F("Radio enabled."));
            } else {
                Serial.println(F("Radio disabled."));
            }
        }
    }
}

#endif