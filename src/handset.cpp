#ifndef TRANSPONDER

#include <Arduino.h>
#include <CC1101Radio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <SPI.h>

#include "gui.h"
#include "vcc.h"
#include "message.h"

#define TFT_DC 6
#define TFT_CS 5
#define SD_CS 4

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

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

    void set(float stat) {
        if (stat == NO_READING_FLOAT) {
            return set(NO_READING_INT);
        }
        set((int) stat);
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
Stat *Altitude;
Stat *Heading;
Pipeline *pipe;
Button *Settings;

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile dataFile;

metrics remote;

unsigned long lastUpdate;
unsigned long lastAck;
unsigned long lastSent = 0;
int lastRssi;      // dBm
int lastRoundtrip; // ms
int lastVcc;       // mV
int sinceLastAck;  // sec

bool disableLogging = false;

const unsigned long SEND_WAIT = 1000;  // in ms
const unsigned long UPDATE_WAIT = 500; // in ms

void onMessageReceived(Message *msg) {
    lastAck = millis();
    lastRssi = msg->rssi;
    lastRoundtrip = (int) (lastAck-lastSent);
    Serial.print("received ");
    const char *body = msg->getBody();
    Serial.print(msg->size);
    Serial.println(" bytes");
    // expects "ack<data>"
    if (body[0] != 'a' || body[1] != 'c' || body[2] != 'k') {
        Serial.println("received non-ack or malformed");
        return;
    }

    unpack((char *)body, remote);

    Serial.print("round trip ");
    Serial.print(lastRoundtrip);
    Serial.println("ms");
}

#ifdef DEBUG
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
#endif

void setup() {
    Serial.begin(38400);
    Serial.println("handset");

    tft.begin();
    tft.setTextSize(2);
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

    radio = new CC1101Radio();
    radio->listen(onMessageReceived);

#ifdef DEBUG
    if (disableLogging) {
        Serial.println(F("Cannot print SD card info, logging disabled"));
    } else {
        printCardInfo();
    }
#endif

    if (!disableLogging) {
        if (dataFile.open(root, "DATA.TXT", O_CREAT | O_TRUNC)) {
            dataFile.close();
        } else {
            Serial.println(F("Unable to create log file, disabling data logger."));
            disableLogging = true;
        }
    }

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

    Altitude = new Stat(120, 70, 170, 70, 10, 230);
    Altitude->setLabel("alt");
    Altitude->setUnit("m");
    pipe->push(drawLabelForwarder, Altitude);

    Settings = new Button(Point{x: 10, y: 280}, Size{w: 150, h: 30});
    Settings->setLabel("Settings");
    pipe->push(drawButtonForwarder, Settings);
}

void scheduleDraw() {
    pipe->push(drawValueForwarder, Ack);
    pipe->push(drawValueForwarder, Lag);
    pipe->push(drawValueForwarder, Vib);
    pipe->push(drawValueForwarder, Rssi);
    pipe->push(drawValueForwarder, Vcc);
    pipe->push(drawValueForwarder, Rvcc);
    pipe->push(drawValueForwarder, Rrssi);
    pipe->push(drawValueForwarder, Altitude);

    pipe->push(drawChartForwarder, Lag);
    pipe->push(drawChartForwarder, Vib);
    pipe->push(drawChartForwarder, Rssi);
    pipe->push(drawChartForwarder, Vcc);
    pipe->push(drawChartForwarder, Rvcc);
    pipe->push(drawChartForwarder, Rrssi);
}

void writeLog() {
    if (disableLogging) {
        return;
    }

    if (!dataFile.open(root, "DATA.TXT", O_APPEND | O_WRITE)) {
#ifdef DEBUG
        Serial.println(F("Could not open file for writing"));
#endif
        return;
    }

    // Local sensor readings
    dataFile.print(lastUpdate);
    dataFile.print(F("\t"));
    dataFile.print(sinceLastAck);
    dataFile.print(F("\t"));
    dataFile.print(lastVcc);
    dataFile.print(F("\t"));
    if (lastRoundtrip != NO_READING_INT) {
        dataFile.print(lastRoundtrip);
    }
    dataFile.print(F("\t"));
    if (lastRssi != NO_READING_INT) {
        dataFile.print(lastRssi);
    }
    dataFile.print(F("\t"));

    // Remote sensor readings
    if (validReadingi(remote.lastVcc)) {
        dataFile.print(remote.lastVcc);
    }
    dataFile.print(F("\t"));
    if (validReadingi(remote.lastRssi)) {
        dataFile.print(remote.lastRssi);
    }
    dataFile.print(F("\t"));
    if (validReadingi(remote.lastVibration)) {
        dataFile.print(remote.lastVibration);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastAltitude)) {
        dataFile.print(remote.lastAltitude);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastTemp)) {
        dataFile.print(remote.lastTemp);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastPitch)) {
        dataFile.print(remote.lastPitch);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastRoll)) {
        dataFile.print(remote.lastRoll);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastHeading)) {
        dataFile.print(remote.lastHeading);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastGyroX)) {
        dataFile.print(remote.lastGyroX);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastGyroY)) {
        dataFile.print(remote.lastGyroY);
    }
    dataFile.print(F("\t"));
    if (validReadingf(remote.lastGyroZ)) {
        dataFile.print(remote.lastGyroZ);
    }
    dataFile.println(F("\t"));
    dataFile.close();
}

void loop() {
    radio->tick();
    pipe->tick();
    Settings->tick();
    if (millis() - lastSent > SEND_WAIT) {
        Serial.println(F("sending helo"));
        Message msg("helo");
        lastSent = millis();
        radio->send(&msg);
    }
    if (millis() - lastUpdate > UPDATE_WAIT) {
        lastUpdate = millis();
        lastVcc = readVcc();

        sinceLastAck = (int) lround((lastUpdate - lastAck) / 1000.0);

        if (lastAck < lastSent) {
            remote.setNoReading();
            lastRoundtrip = NO_READING_INT;
            lastRssi = NO_READING_INT;
        }

        Ack->set(sinceLastAck);
        Vcc->set(lastVcc);
        Lag->set(lastRoundtrip);
        Rssi->set(lastRssi);

        Rvcc->set(remote.lastVcc);
        Vib->set(remote.lastVibration);
        Rrssi->set(remote.lastRssi);
        Altitude->set(remote.lastAltitude);

        writeLog();
        scheduleDraw();
    }
}


#endif