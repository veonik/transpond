#ifndef TRANSPONDER

#include <Arduino.h>
#include <CC1101Radio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <SPI.h>

#include "../common/util.h"
#include "../common/message.h"

#include "gui.h"
#include "controller/dashboard.h"

#define TFT_DC 6
#define TFT_CS 5
#define SD_CS 4

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Pipeline *pipe;

Radio *radio;

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile dataFile;

metrics m;
//metrics mMin; // TODO: collect these constantly, but not in bulk.
//metrics mMax;
metrics mLast;

unsigned long lastTick;
unsigned long lastUpdate;
unsigned long lastAck;
unsigned long lastSent;
int lastRssi;        // dBm
int lastRoundtrip;   // ms
int lastVcc;         // mV
int sinceLastAck;    // sec
long avgTickDelay;   // ms
long avgSendDelay;   // ms

bool disableLogging = false;
bool sendInfx = false;

const long MIN_SEND_WAIT = 50;  // in ms
const long MAX_SEND_WAIT = 1000; // in ms
const long TIMEOUT_WAIT = 1000;  // in ms
const long UPDATE_WAIT = 500;    // in ms

int logFileRotation;
unsigned long logFileSize = 0;            // in bytes
unsigned long logFileMax = 1024L * 1024L; // in bytes
char logFilename[10] = "D0.LOG";

void printCardInfo() {
    Serial.print(F("Data logging is "));
    if (disableLogging) {
        Serial.println(F("DISABLED"));
    } else {
        Serial.println(F("ENABLED"));
    }
    Serial.println();

    Serial.print("\nCard type: ");
    switch (card.type()) {
        case SD_CARD_TYPE_SD1:
            Serial.println("SD1");
            break;
        case SD_CARD_TYPE_SD2:
            Serial.println("SD2");
            break;
        case SD_CARD_TYPE_SDHC:
            Serial.println("SDHC");
            break;
        default:
            Serial.println("Unknown");
    }

    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);

    unsigned long volumesize;
    volumesize = volume.blocksPerCluster();
    volumesize *= volume.clusterCount();
    volumesize /= 2;
    volumesize /= 1024;
    Serial.print("Volume size: ");
    Serial.print(volumesize, DEC);
    Serial.println("MB");

    Serial.println("name\tdate\tsize");
    root.ls(LS_R | LS_DATE | LS_SIZE);
    Serial.println();
}

void rotateLog() {
    logFileRotation++;
    char *p = logFilename;
    *p++ = 'D';
    itoa(logFileRotation, p, 10);
    p++;
    if (logFileRotation > 99) {
        p++;
        if (logFileRotation > 999) {
            p++;
        }
    }
    *p++ = '.';
    *p++ = 'L';
    *p++ = 'O';
    *p++ = 'G';
    *p = 0;
#ifdef DEBUG
    Serial.print("rotating to new log file: ");
    Serial.println(logFilename);
#endif
    logFileSize = 0L;
}

void openLog() {
    int tries = 0;
    while (tries++ < 3 && !dataFile.open(root, logFilename, O_CREAT | O_WRITE)) {
        Serial.println(F("Unable to create log file, rotating..."));
        rotateLog();
    }
    if (!dataFile.isOpen()) {
        Serial.println(F("Unable to create log file, disabling data logger."));
        disableLogging = true;
        return;
    }
}

void setupLog() {
    if (!card.init(SPI_HALF_SPEED, SD_CS)) {
        Serial.println(F("No SD card inserted, disabling data logger."));
        disableLogging = true;
    }

    if (!disableLogging && !volume.init(card)) {
        Serial.println(F("Unable to initialize SD volume"));
        disableLogging = true;
    }

    if (!disableLogging && !root.openRoot(volume)) {
        Serial.println(F("Unable to open volume root"));
        disableLogging = true;
    }

#ifdef DEBUG
    printCardInfo();
#endif

    if (!disableLogging) {
        openLog();
    }
}

void onMessageReceived(Message *msg) {
    unsigned long ack = millis();

    const char *body = msg->getBody();
#ifdef DEBUGV
    Serial.print("received ");
    Serial.print(msg->size);
    Serial.println(" bytes");
#endif
    // expects "pre<data>"
    packFn cmd = getCommand(body);
    if (cmd == NULL) {
        Serial.print("received non-ack or malformed: ");
        Serial.print(body[0]);
        Serial.print(body[1]);
        Serial.println(body[2]);
        return;
    }
    cmd(&m, (char *)body);

    lastAck = ack;
    lastRssi = msg->rssi;
    lastRoundtrip = (int) (lastAck-lastSent);
}

void setup() {
    Serial.begin(38400);
    Serial.println("handset");

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

    setupLog();

    radio = new CC1101Radio();
    radio->listen(onMessageReceived);

    pipe = new Pipeline();
    pipe->segueTo(new DashboardViewController());
}


void writeLog() {
    if (disableLogging) {
        return;
    }

    if (!dataFile.isOpen()) {
        if (!dataFile.open(root, logFilename, O_WRITE | O_APPEND)) {
#ifdef DEBUG
            Serial.println(F("Could not open file for writing"));
#endif
            disableLogging = true;
            return;
        }
    }

    // Local sensor readings
    logFileSize += dataFile.print(lastUpdate);
    logFileSize += dataFile.print(F("\t"));
    logFileSize += dataFile.print(sinceLastAck);
    logFileSize += dataFile.print(F("\t"));
    logFileSize += dataFile.print(lastVcc);
    logFileSize += dataFile.print(F("\t"));
    if (validReadingi(lastRoundtrip)) {
        logFileSize += dataFile.print(lastRoundtrip);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingi(lastRssi)) {
        logFileSize += dataFile.print(lastRssi);
    }
    logFileSize += dataFile.print(F("\t"));

    // Remote sensor readings
    if (validReadingi(m.vcc)) {
        logFileSize += dataFile.print(m.vcc);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingi(m.rssi)) {
        logFileSize += dataFile.print(m.rssi);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingi(m.vibration)) {
        logFileSize += dataFile.print(m.vibration);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.altitudeGps)) {
        logFileSize += dataFile.print(m.altitudeGps);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.altitude)) {
        logFileSize += dataFile.print(m.altitude);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.temp)) {
        logFileSize += dataFile.print(m.temp);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.temp2)) {
        logFileSize += dataFile.print(m.temp2);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.latitude)) {
        logFileSize += dataFile.print(m.latitude, 6);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.longitude)) {
        logFileSize += dataFile.print(m.longitude, 6);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accelX)) {
        logFileSize += dataFile.print(m.accelX);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accelY)) {
        logFileSize += dataFile.print(m.accelY);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accelZ)) {
        logFileSize += dataFile.print(m.accelZ);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.magX)) {
        logFileSize += dataFile.print(m.magX);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.magY)) {
        logFileSize += dataFile.print(m.magY);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.magZ)) {
        logFileSize += dataFile.print(m.magZ);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyroX)) {
        logFileSize += dataFile.print(m.gyroX);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyroY)) {
        logFileSize += dataFile.print(m.gyroY);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyroZ)) {
        logFileSize += dataFile.print(m.gyroZ);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accel2X)) {
        logFileSize += dataFile.print(m.accel2X);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accel2Y)) {
        logFileSize += dataFile.print(m.accel2Y);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.accel2Z)) {
        logFileSize += dataFile.print(m.accel2Z);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.mag2X)) {
        logFileSize += dataFile.print(m.mag2X);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.mag2Y)) {
        logFileSize += dataFile.print(m.mag2Y);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.mag2Z)) {
        logFileSize += dataFile.print(m.mag2Z);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyro2X)) {
        logFileSize += dataFile.print(m.gyro2X);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyro2Y)) {
        logFileSize += dataFile.print(m.gyro2Y);
    }
    logFileSize += dataFile.print(F("\t"));
    if (validReadingf(m.gyro2Z)) {
        logFileSize += dataFile.print(m.gyro2Z);
    }
    logFileSize += dataFile.println(F("\t"));
    dataFile.sync();
    if (logFileSize >= logFileMax) {
        dataFile.close();
        rotateLog();
        openLog();
    }
}

int printTicks;
int printTicksI;

void loop() {
    unsigned long tick = millis();
    long diff = tick - lastTick;
    avgTickDelay = expAvg(avgTickDelay, diff);
    lastTick = tick;
    if (printTicks > printTicksI) {
        Serial.print("tick delay: ");
        Serial.print(diff);
        Serial.println("ms");
    }

    // Radio tick.
    radio->tick();

    // Send helo.
    diff = tick - lastSent;
    if ((lastAck < lastSent && diff > MAX_SEND_WAIT)
        || diff > MIN_SEND_WAIT
    ) {
        avgSendDelay = expAvg(avgSendDelay, diff);
        lastSent = tick;
        if (printTicks > printTicksI) {
            printTicksI++;
            Serial.print("send delay: ");
            Serial.print(diff);
            Serial.println("ms");
        }

        if (sendInfx) {
            Message msg("he");
            radio->send(&msg);
        } else {
            Message msg("lo");
            radio->send(&msg);
        }
        sendInfx = !sendInfx;
    }

    // Update metrics.
    tick = millis();
    if (tick - lastUpdate > UPDATE_WAIT) {
        memcpy(&mLast, &m, sizeof(metrics));
        lastUpdate = tick;
        lastVcc = readVcc();

        sinceLastAck = (int) ((lastUpdate - lastAck) / 1000L);
        if (sinceLastAck < 0) {
#ifdef DEBUG
            Serial.println("sinceLastAck less than 0");
            Serial.print("round((");
            Serial.print(lastUpdate);
            Serial.print(" - ");
            Serial.print(lastAck);
            Serial.print(") / 1000.0) = ");
            Serial.println(sinceLastAck);
#endif

            sinceLastAck = 0;
        }

        if (lastAck + TIMEOUT_WAIT < lastSent) {
            lastRoundtrip = NO_READING_INT;
            lastRssi = NO_READING_INT;
        }

        writeLog();
    }

    // Pipeline tick.
    pipe->tick();

    // Serial commands.
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        if (cmd[0] == 'D') {
            dataFile.close();
            if (!dataFile.open(root, logFilename, O_READ)) {
                Serial.println();
                Serial.println("Could not open file for reading");
                return;
            }
            uint32_t offset;
            long cmdOffset = cmd.substring(1).toInt();
            if (cmdOffset < 0) {
                offset = dataFile.fileSize() + cmdOffset;
            } else {
                offset = cmdOffset;
            }
            dataFile.seekSet(offset);
            char buf[128];
            int16_t read;
            bool firstRead = true;
            do {
                read = dataFile.read(buf, 127);
                buf[read] = 0;
                if (firstRead && read > 0) {
                    firstRead = false;
                    char *firstNewline = strchr(buf, '\n');
                    Serial.print(++firstNewline);
                } else {
                    Serial.print(buf);
                }
            } while (read > 0);
            Serial.println();
            dataFile.close();

        } else if (cmd[0] == 'L') {
            printCardInfo();

        } else if (cmd[0] == 'I') {
            Serial.print(F("Average tick delay: "));
            Serial.print(avgTickDelay);
            Serial.println(F("ms"));
            Serial.print(F("Average send delay: "));
            Serial.print(avgSendDelay);
            Serial.println(F("ms"));
            Serial.print(F("RAM free: "));
            Serial.print(freeRam());
            Serial.println(F(" bytes"));
            printTicks = (int) cmd.substring(1).toInt();
            printTicksI = 0;
        } else if (cmd[0] == 'G') {
            Serial.print(F("Location: "));
            Serial.print(m.latitude, 6);
            Serial.print(F(","));
            Serial.println(m.longitude, 6);
        } else if (cmd[0] == 'U') {
            Message msg("up");
            radio->send(&msg);
        } else if (cmd[0] == 'F') {
            if (m.logging != MODULE_ENABLED) {
                Serial.println(F("Requesting to enable logging"));
            } else {
                Serial.println(F("Requesting to disable logging"));
            }
            Message msg("tl");
            radio->send(&msg);
        } else if (cmd[0] == 'T') {
            Serial.print(F("Remote data logging is "));
            Serial.println(m.logging == MODULE_ENABLED ? "enabled" : "disabled");
        }
    }
}


#endif