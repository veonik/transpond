#ifndef RECEIVER

#include <Arduino.h>
#include <CC1101Radio.h>

#include "vcc.h"

Radio *radio;
int piezoSensor = A0;
int lastReading = 0;

unsigned long lastMessage;
long lastRssi;

void onMsg(Message *msg) {
    lastMessage = micros();
    lastRssi = msg->rssi;
    String ack = "ack ";
    ack += lastReading;
    ack += " ";
    ack += readVcc();
    ack += " ";
    ack += lastRssi;
    Message resp(ack);
    radio->send(&resp);
}

void setup() {
    Serial.begin(38400);
    Serial.println("transmitter");
    radio = new CC1101Radio();
    radio->listen(onMsg);
    Serial.println(CC1101Interrupt);
}

void loop() {
    radio->tick();
    lastReading = analogRead(piezoSensor);
}

#endif