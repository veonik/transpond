#ifndef TRANSPONDER

#include <Arduino.h>
#include <CC1101Radio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <SPI.h>

#include "gui.h"
#include "util.h"
#include "message.h"

#include "dashboard.h"

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
AckCommand ackCommand = AckCommand(&m);
Ack2Command ac2Command = Ack2Command(&m);

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
long ticks;
long sends;

bool disableLogging = false;
bool sendInfx = false;

const long MIN_SEND_WAIT = 100;  // in ms
const long MAX_SEND_WAIT = 1000; // in ms
const long TIMEOUT_WAIT = 1000;  // in ms
const long UPDATE_WAIT = 500;    // in ms

const char *LOG_FILE = "DATA.TXT";

void onMessageReceived(Message *msg) {
    unsigned long ack = millis();

    const char *body = msg->getBody();
#ifdef DEBUGV
    Serial.print("received ");
    Serial.print(msg->size);
    Serial.println(" bytes");
#endif
    // expects "ack<data>"
    if (body[0] != 'a' || body[1] != 'c') {
        Serial.println("received non-ack or malformed");
        return;
    }
    if (body[2] == 'k') {
        ackCommand.unpack((char *)body);
    } else if (body[2] == '2') {
        ac2Command.unpack((char *)body);
    }

    lastAck = ack;
    lastRssi = msg->rssi;
    lastRoundtrip = (int) (lastAck-lastSent);

}

#ifdef DEBUG
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
#endif

void setup() {
    Serial.begin(38400);
    Serial.println("handset");

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

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
        if (!dataFile.open(root, LOG_FILE, O_CREAT | O_WRITE)) {
            dataFile.open(root, LOG_FILE, O_TRUNC | O_WRITE);
        }
        if (!dataFile.truncate(0)) {
            Serial.println(F("Unable to create or truncate log file, disabling data logger."));
            disableLogging = true;
        }
        dataFile.sync();
    }

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
        if (!dataFile.open(root, LOG_FILE, O_WRITE | O_APPEND)) {
#ifdef DEBUG
            Serial.println(F("Could not open file for writing"));
#endif
            return;
        }
    }

    // Local sensor readings
    dataFile.print(lastUpdate);
    dataFile.print(F("\t"));
    dataFile.print(sinceLastAck);
    dataFile.print(F("\t"));
    dataFile.print(lastVcc);
    dataFile.print(F("\t"));
    if (validReadingi(lastRoundtrip)) {
        dataFile.print(lastRoundtrip);
    }
    dataFile.print(F("\t"));
    if (validReadingi(lastRssi)) {
        dataFile.print(lastRssi);
    }
    dataFile.print(F("\t"));

    // Remote sensor readings
    if (validReadingi(m.vcc)) {
        dataFile.print(m.vcc);
    }
    dataFile.print(F("\t"));
    if (validReadingi(m.rssi)) {
        dataFile.print(m.rssi);
    }
    dataFile.print(F("\t"));
    if (validReadingi(m.vibration)) {
        dataFile.print(m.vibration);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.altitudeGps)) {
        dataFile.print(m.altitudeGps);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.altitude)) {
        dataFile.print(m.altitude);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.temp)) {
        dataFile.print(m.temp);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.temp2)) {
        dataFile.print(m.temp2);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.latitude)) {
        dataFile.print(m.latitude, 6);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.longitude)) {
        dataFile.print(m.longitude, 6);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accelX)) {
        dataFile.print(m.accelX);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accelY)) {
        dataFile.print(m.accelY);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accelZ)) {
        dataFile.print(m.accelZ);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.magX)) {
        dataFile.print(m.magX);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.magY)) {
        dataFile.print(m.magY);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.magZ)) {
        dataFile.print(m.magZ);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyroX)) {
        dataFile.print(m.gyroX);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyroY)) {
        dataFile.print(m.gyroY);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyroZ)) {
        dataFile.print(m.gyroZ);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accel2X)) {
        dataFile.print(m.accel2X);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accel2Y)) {
        dataFile.print(m.accel2Y);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.accel2Z)) {
        dataFile.print(m.accel2Z);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.mag2X)) {
        dataFile.print(m.mag2X);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.mag2Y)) {
        dataFile.print(m.mag2Y);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.mag2Z)) {
        dataFile.print(m.mag2Z);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyro2X)) {
        dataFile.print(m.gyro2X);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyro2Y)) {
        dataFile.print(m.gyro2Y);
    }
    dataFile.print(F("\t"));
    if (validReadingf(m.gyro2Z)) {
        dataFile.print(m.gyro2Z);
    }
    dataFile.println(F("\t"));
    dataFile.sync();
}

int printTicks;
int printTicksI;

void loop() {
    unsigned long tick = millis();
    long diff = tick - lastTick;
    ticks++;
    avgTickDelay = avgTickDelay + ((diff - avgTickDelay) / ticks);
    lastTick = tick;
    if (printTicks > printTicksI) {
        Serial.print("tick delay: ");
        Serial.print(diff);
        Serial.println("ms");
    }

    // Tick.
    radio->tick();
    pipe->tick();

    // Send helo.
    diff = tick - lastSent;
    if ((lastAck < lastSent && diff > MAX_SEND_WAIT)
        || diff > MIN_SEND_WAIT
    ) {
        sends++;
        avgSendDelay = avgSendDelay + ((diff - avgSendDelay) / sends);
        lastSent = tick;
        if (printTicks > printTicksI) {
            printTicksI++;
            Serial.print("send delay: ");
            Serial.print(diff);
            Serial.println("ms");
        }

        if (sendInfx) {
            sendInfx = false;
            Message msg("infx");
            radio->send(&msg);
        } else {
            Message msg("helo");
            radio->send(&msg);
            sendInfx = true;
        }
    }

    // Update metrics.
    // TODO: This should probably happen after radio->tick but before pipe->tick.
    if (tick - lastUpdate > UPDATE_WAIT) {
        lastUpdate = tick;
        lastVcc = readVcc();

        sinceLastAck = (int) floor((lastUpdate - lastAck) / 1000.0);
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
            m.setNoReading();
            lastRoundtrip = NO_READING_INT;
            lastRssi = NO_READING_INT;
        }

        writeLog();
    }

    // Serial commands.
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        if (cmd[0] == 'D') {
            dataFile.close();
            if (!dataFile.open(root, LOG_FILE, O_READ)) {
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

        } else if (cmd[0] == 'I') {
            Serial.print(F("Average tick delay: "));
            Serial.print(avgTickDelay);
            Serial.println(F("ms"));
            Serial.print(F("Average send delay: "));
            Serial.print(avgSendDelay);
            Serial.println(F("ms"));
            printTicks = (int) cmd.substring(1).toInt();
            printTicksI = 0;
        } else if (cmd[0] == 'G') {
            Serial.print(F("Location: "));
            Serial.print(m.latitude, 6);
            Serial.print(F(","));
            Serial.println(m.longitude, 6);
        }
    }
}


#endif