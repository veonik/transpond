#ifndef TRANSMITTER

#include <Arduino.h>
#include <CC1101Radio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <SPI.h>

#include "vcc.h"

#define TFT_DC 6
#define TFT_CS 5
#define SD_CS 4

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

typedef void (*drawCallback)(void*);

class Pipeline {
private:
    static const byte SIZE = 32;

    drawCallback _callbacks[SIZE];
    void *_contexts[SIZE];

    short _pop = 0;
    short _push = 0;

public:
    void push(drawCallback fn, void *context) {
        _callbacks[_push] = fn;
        _contexts[_push] = context;
        _push++;
        if (_push == SIZE) {
            _push = 0; // wrap
        }
    }

    void tick() {
        if (!_callbacks[_pop]) {
            return;
        }
        void *context = _contexts[_pop];
        _callbacks[_pop](context);
        _callbacks[_pop] = NULL;
        _contexts[_pop] = NULL;
        _pop++;
        if (_pop == SIZE) {
            _pop = 0; // wrap
        }
    }

    void flush() {
        while (_pop != _push) {
            tick();
        }
    }
};

struct Point {
    short x;
    short y;
};

class Stat {
private:
    Point _label;
    Point _value;
    Point _chart;

    int _chartWidth = 240;
    int _controlWidth = 60;
    int _lastDrawWidth = 60;
    const char *_labelText = "";
    const char *_unitText = "";
    int _stat;
    int _historical[160];
    bool _hideLabel = false;
    bool _redrawChart = true;
    bool _invertChart = false;
    int _min = 0;
    int _max = 0;
    short _end = 0;
    int _chartColor = ILI9341_RED;


public:
    Stat(short labelX, short labelY, short valueX, short valueY, short chartX, short chartY) {
        _label = Point{x: labelX, y: labelY};
        _value = Point{x: valueX, y: valueY};
        _chart = Point{x: chartX, y: chartY};
    }

    void setChartWidth(int width) {
        _chartWidth = width;
    }

    void setLabel(const char *label) {
        _labelText = label;
    }

    void setHideLabel(bool hide) {
        _hideLabel = hide;
        _controlWidth = !_hideLabel ? 60 : 100;
    }

    void setUnit(const char *unit) {
        _unitText = unit;
    }

    void setColor(int color) {
        _chartColor = color;
    }

    void setInvert(bool invert) {
        _invertChart = invert;
    }

    void set(int stat) {
        _stat = stat;
        _end++;
        if (_end >= _chartWidth-80) {
            _end = 0;
            _redrawChart = true;
            _min = 0;
            _max = 0;
        }
        if (abs(stat) < _min || _min == 0) {
            _min = abs(stat);
            if (_min > 0) {
                _min--;
            }
        } else if (abs(stat) > _max) {
            _redrawChart = true;
            _max = abs(stat)+1;
        }
        _historical[_end] = stat;
    }

    void drawLabel() {
        if (!_hideLabel) {
            tft.setCursor(_label.x, _label.y+1);
            tft.setTextColor(ILI9341_WHITE);
            tft.print(_labelText);
        }
        tft.fillRect(_value.x, _value.y, _controlWidth, 17, 0x2104);
        _lastDrawWidth = _controlWidth;
    }

    void drawValue() {
        char val[8];
        int chars = sprintf(val, "%d", _stat);
        int drawWidth = (chars * 12);
        tft.setCursor(_value.x, _value.y+1);
        tft.setTextColor(ILI9341_WHITE, 0x2104);
        tft.print(val);
        if (drawWidth < _lastDrawWidth) {
            tft.fillRect(_value.x+drawWidth, _value.y, _lastDrawWidth-drawWidth, 17, 0x2104);
        }
        _lastDrawWidth = drawWidth;
        tft.setTextSize(1);
        tft.setCursor(tft.getCursorX(), tft.getCursorY()+7);
        tft.print(_unitText);
        tft.setTextSize(2);
    }

    void drawChart() {

        if (_redrawChart) {
            tft.fillRect(_chart.x, _chart.y, _chartWidth, 31 /* to catch the bottom of the unit on the axis */, ILI9341_BLACK);
            tft.setCursor(_chart.x+27-(strlen(_labelText)*6), _chart.y+12);
            tft.setTextColor(ILI9341_WHITE);
            tft.setTextSize(1);
            tft.print(_labelText);
            tft.setCursor(_chart.x+_chartWidth-45, _chart.y);
            if (_invertChart) {
                tft.print("-");
                tft.print(_min);
            } else {
                tft.print(_max);
            }

            if (_unitText) {
                tft.setTextColor(_chartColor);
                tft.setCursor(_chart.x + _chartWidth-45, _chart.y + 12);
                tft.print(_unitText);
                tft.setTextColor(ILI9341_WHITE);
            }
            tft.setCursor(_chart.x + _chartWidth-45, _chart.y+24);
            if (_invertChart) {
                tft.print("-");
                tft.print(_max);
            } else {
                tft.print(_min);
            }
            tft.setTextSize(2);
            _redrawChart = false;
            tft.fillRect(_chart.x+30, _chart.y, _chartWidth-80, 30, 0x2104);
            for (int i = 0; i < _end; i++) {
                float norm = ((float) abs(_historical[i]))/(_max);
                int x = _chart.x+30+i;
                int y;
                if (!_invertChart) {
                    y = _chart.y+29-(round(norm*29));
                } else {
                    y = _chart.y+(round(norm*29));
                }
                if (y < _chart.y || y > _chart.y+29) {
                    continue;
                }
                tft.fillRect(x, y, 1, 1, _chartColor);
            }
        } else {
            int i = _end;
            float norm = ((float) abs(_historical[i]))/(_max);
            int x = _chart.x+30+i;
            int y;
            if (!_invertChart) {
                y = _chart.y+29-(round(norm*29));
            } else {
                y = _chart.y+(round(norm*29));
            }
            if (y < _chart.y || y > _chart.y+29) {
                return;
            }
            tft.fillRect(x, y, 1, 1, _chartColor);
        }

    }
};

void drawLabelForwarder(void* context) {
    static_cast<Stat*>(context)->drawLabel();
}
void drawValueForwarder(void* context) {
    static_cast<Stat*>(context)->drawValue();
}
void drawChartForwarder(void* context) {
    static_cast<Stat*>(context)->drawChart();
}


Radio *radio;
Stat *Ack;
Stat *Lag;
Stat *Rssi;
Stat *Rrssi;
Stat *Vib;
Stat *Vcc;
Stat *Rvcc;
Pipeline *pipe;

SdFile dataFile;

unsigned long lastUpdate;
int lastRssi;
int lastRrssi;
unsigned long lastRvcc;
unsigned long lastAck;
long lastVibration;

Sd2Card card;
SdVolume volume;
SdFile root;


unsigned long lastSent = 0;
const unsigned long WAIT = 1000; // in ms


void onResponse(Message *msg) {
    unsigned long ack = millis();
    int rssi = msg->rssi;
    Serial.print("received '");
    String body = msg->getBody();
    Serial.print(body);
    Serial.println("'");
    int i = body.indexOf(' ');
    // expects "ack <vibration>", so we can just check for i == 3
    if (i != 3 || body.substring(0, 3) != "ack") {
        Serial.println("received non-ack or malformed");
        return;
    }
    int j = body.indexOf(' ', i+1);
    int k = body.indexOf(' ', j+1);
    lastVibration = body.substring(i+1, j).toInt();
    lastRvcc = body.substring(j+1, k).toInt();
    lastRrssi = body.substring(k+1).toInt();
    lastAck = ack;
    lastRssi = rssi;
    Serial.print("round trip ");
    Serial.print(lastAck-lastSent);
    Serial.println("ms");
}

void printCardInfo() {
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

    root.openRoot(volume);

    Serial.println("name\tdate\tsize");
    root.ls(LS_R | LS_DATE | LS_SIZE);
    Serial.println();
}

void waitForResponse(Message *msg) {
    radio->listen(onResponse);
}

void setup() {
    Serial.begin(38400);
    Serial.println("handset");

    tft.begin();
    tft.setTextSize(2);
    tft.fillScreen(ILI9341_BLACK);

    if (!card.init(SPI_HALF_SPEED, SD_CS)) {
        Serial.println("Unable to start SD");
    }

    if (!volume.init(card)) {
        Serial.println("Unable to initialize SD volume");
    }

    root.openRoot(volume);

    radio = new CC1101Radio();
    Serial.println(CC1101Interrupt);

#ifdef DEBUG
    printCardInfo();
#endif

    dataFile.open(root, "DATA.TXT", O_CREAT | O_TRUNC);
    dataFile.close();

    pipe = new Pipeline();

    Ack = new Stat(10, 10, 50, 10, 10, 90);
    Ack->setLabel("ack");
    pipe->push(drawLabelForwarder, Ack);

    Lag = new Stat(120, 10, 170, 10, 10, 90);
    Lag->setLabel("lag");
    Lag->setUnit("ms");
    Lag->setColor(ILI9341_BLUE);
    pipe->push(drawLabelForwarder, Lag);
    pipe->push(drawChartForwarder, Lag);

    Rssi = new Stat(10, 30, 10, 30, 10, 125);
    Rssi->setLabel("rssi");
    Rssi->setHideLabel(true);
    Rssi->setUnit("dBm");
    Rssi->setColor(ILI9341_PURPLE);
    Rssi->setInvert(true);
    pipe->push(drawLabelForwarder, Rssi);
    pipe->push(drawChartForwarder, Rssi);

    Rrssi = new Stat(120, 30, 120, 30, 10, 160);
    Rrssi->setLabel("rrssi");
    Rrssi->setHideLabel(true);
    Rrssi->setUnit("dBm");
    Rrssi->setColor(ILI9341_CYAN);
    Rrssi->setInvert(true);
    pipe->push(drawLabelForwarder, Rrssi);
    pipe->push(drawChartForwarder, Rrssi);

    Vcc = new Stat(10, 50, 50, 50, 10, 195);
    Vcc->setChartWidth(130);
    Vcc->setLabel("vcc");
    Vcc->setUnit("mV");
    Vcc->setColor(ILI9341_RED);
    pipe->push(drawLabelForwarder, Vcc);
    pipe->push(drawChartForwarder, Vcc);

    Rvcc = new Stat(120, 50, 170, 50, 120, 195);
    Rvcc->setChartWidth(130);
    Rvcc->setLabel("rvcc");
    Rvcc->setUnit("mV");
    Rvcc->setColor(ILI9341_ORANGE);
    pipe->push(drawLabelForwarder, Rvcc);
    pipe->push(drawChartForwarder, Rvcc);

    Vib = new Stat(10, 70, 50, 70, 10, 230);
    Vib->setLabel("vib");
    Vib->setColor(ILI9341_GREEN);
    pipe->push(drawLabelForwarder, Vib);
    pipe->push(drawChartForwarder, Vib);
}

void loop() {
    radio->tick();
    pipe->tick();
    if (millis() - lastSent > WAIT) {
        Serial.println("sending helo");
        Message msg("helo");
        msg.then(waitForResponse);
        lastSent = millis();
        radio->send(&msg);
    }
    if (millis() - lastUpdate > 500) {
        lastUpdate = millis();

        long vcc = readVcc();
        Vcc->set(vcc);

        int sinceLastAck = (int) (lastUpdate - lastAck) / 1000;
        Ack->set(sinceLastAck);

        int lag = -1;
        if (lastAck >= lastSent) {
            lag = (int) lastAck - lastSent;
        }
        Lag->set(lag);

        Rvcc->set((int) lastRvcc);
        Vib->set((int) lastVibration);
        Rssi->set(lastRssi);
        Rrssi->set(lastRrssi);

        dataFile.open(root, "DATA.TXT", O_APPEND | O_WRITE);
        dataFile.print(sinceLastAck);
        dataFile.print("\t");
        dataFile.print(lag);
        dataFile.print("\t");
        dataFile.print(lastVibration);
        dataFile.print("\t");
        dataFile.print(lastRssi);
        dataFile.print("\t");
        dataFile.print(vcc);
        dataFile.print("\t");
        dataFile.print(lastRvcc);
        dataFile.write("\n");
        dataFile.close();

        pipe->push(drawValueForwarder, Ack);
        pipe->push(drawValueForwarder, Lag);
        pipe->push(drawValueForwarder, Vib);
        pipe->push(drawValueForwarder, Rssi);
        pipe->push(drawValueForwarder, Vcc);
        pipe->push(drawValueForwarder, Rvcc);
        pipe->push(drawValueForwarder, Rrssi);

        pipe->push(drawChartForwarder, Lag);
        pipe->push(drawChartForwarder, Vib);
        pipe->push(drawChartForwarder, Rssi);
        pipe->push(drawChartForwarder, Vcc);
        pipe->push(drawChartForwarder, Rvcc);
        pipe->push(drawChartForwarder, Rrssi);
    }
}


#endif