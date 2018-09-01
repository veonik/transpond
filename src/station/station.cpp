#ifdef STATION

#include <Arduino.h>
#include <CC1101Radio.h>
#include <SPI.h>
#ifdef __MK64FX512__
#include <RamMonitor.h>
#endif

#include "../common/util.h"
#include "../common/message.h"

Radio *radio;

metrics m;
//metrics mMin; // TODO: collect these constantly, but not in bulk.
//metrics mMax;
metrics mLast;

#ifdef __MK64FX512__
RamMonitor ramMonitor{};
#endif

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

char lastReceived[64]{};

const long MIN_SEND_WAIT = 50;  // in ms
const long MAX_SEND_WAIT = 1000; // in ms
const long TIMEOUT_WAIT = 1000;  // in ms
const long UPDATE_WAIT = 500;    // in ms

void onMessageReceived(Message *msg) {
    unsigned long ack = millis();

    const char *body = msg->getBody();
#ifdef DEBUG
    Serial.print("received ");
    Serial.print(msg->size);
    Serial.println(" bytes");
#endif
    if (lastReceived[0] != 0) {
        Serial.println("received packet but not done processing previous packet!");
        return;
    }
    memcpy(lastReceived, body, 64);

    lastAck = ack;
    lastRssi = msg->rssi;
    lastRoundtrip = (int) (lastAck-lastSent);
}

int printTicks;
int printTicksI;

void handleMessage() {
    // expects "pre<data>"
    Serial.print("command: ");
    Serial.print(lastReceived[0]);
    Serial.print(lastReceived[1]);
    packFn cmd = getCommand(lastReceived);
    if (cmd == nullptr) {
        Serial.println(F(" (invalid command)"));
    } else {
        Serial.println();
        if (printTicks > printTicksI) {
            Serial.print("contents: ");
            Serial.println(lastReceived);
        }
    }
    memset(lastReceived, 0, 64);
}

void setup() {
#ifdef __MK64FX512__
    ramMonitor.initialize();
#endif

    Serial.begin(38400);
    Serial.println("base station");

    radio = new CC1101Radio();
    radio->listen(onMessageReceived);
}


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
    if (lastReceived[0] != 0) {
        handleMessage();
    }

    // Send helo.
    diff = tick - lastSent;
    if ((lastAck < lastSent && diff > MAX_SEND_WAIT)
        || diff > MIN_SEND_WAIT
    ) {
        //TODO: Add configurable ability for station to act as a handset
//        avgSendDelay = expAvg(avgSendDelay, diff);
//        lastSent = tick;
//        if (printTicks > printTicksI) {
//            Serial.print("send delay: ");
//            Serial.print(diff);
//            Serial.println("ms");
//        }
//
//        if (sendInfx) {
//            Message msg("he");
//            radio->send(&msg);
//        } else {
//            Message msg("lo");
//            radio->send(&msg);
//        }
//        sendInfx = !sendInfx;
        delay(1);
    }

    // Update metrics.
    tick = millis();
    if (tick - lastUpdate > UPDATE_WAIT) {
        memcpy(&mLast, &m, sizeof(metrics));
        lastUpdate = tick;
        lastVcc = readVcc();
#ifdef __MK64FX512__
        ramMonitor.run();
#endif

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
    }

    if (printTicks > printTicksI) {
        printTicksI++;
    }

    // Serial commands.
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        if (cmd[0] == 'I') {
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