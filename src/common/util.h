#ifndef TRANSPOND_UTIL_H
#define TRANSPOND_UTIL_H

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
#include <RamMonitor.h>
extern RamMonitor ramMonitor;
int32_t freeRam() {
    return ramMonitor.adj_free();
}

int readVcc() {
    return 0;
}
#else

// From: http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
int readVcc() {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring

    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both

    long result = (high<<8) | low;

    result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
    return (int) result; // Vcc in millivolts
}

// From: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
int freeRam() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

#define EXPAVG_POWER 256
#define EXPAVG_ALPHA 36
#define EXPAVG_DIFF  220

// From: http://playground.arduino.cc/Main/RunningAverage
long expAvg(long value, long reading) {
    return (EXPAVG_ALPHA * reading + EXPAVG_DIFF * value) / EXPAVG_POWER;
}

#endif