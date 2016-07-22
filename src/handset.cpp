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

class DashboardViewController;
class SettingsViewController;
class Stat;


// TODO: deprecated
void drawLabelForwarder(void* context);
void drawValueForwarder(void* context);
void drawChartForwarder(void* context);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

Pipeline *pipe;

Radio *radio;

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile dataFile;

metrics remote;

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

const long MIN_SEND_WAIT = 100;  // in ms
const long MAX_SEND_WAIT = 1000; // in ms
const long TIMEOUT_WAIT = 1000;  // in ms
const long UPDATE_WAIT = 500;    // in ms

const char *LOG_FILE = "DATA.TXT";

class Stat : public Control {
private:
    Point _value;
    Point _chart;

    int _chartWidth = 240;
    int _controlWidth = 60;
    int _lastDrawWidth = 60;
    const char *_labelText = "";
    const char *_unitText = "";
    int _stat;
    int _historical[160];
    bool _enableChart = false;
    bool _hideLabel = false;
    bool _redrawChart = true;
    bool _invertChart = false;
    int _min = 0;
    int _max = 0;
    short _cur = 0;
    short _end = 0;
    uint16_t _chartColor = ILI9341_RED;


public:
    Stat(short labelX, short labelY, short valueX, short valueY, short chartX, short chartY) :
            Control(Point{x: labelX, y: labelY}, Size{w: 0, h: 0}) {
        _value = Point{x: valueX, y: valueY};
        _chart = Point{x: chartX, y: chartY};
    }

    void setEnableChart(bool enable) {
        _enableChart = enable;
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
        _chartColor = (uint16_t) color;
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
        if (validReadingi(stat)) {
            if (abs(stat) < _min || _min == 0) {
                _min = abs(stat);
                if (_min > 0) {
                    _min--;
                }
            } else if (abs(stat) > _max) {
                _redrawChart = true;
                _max = abs(stat) + 1;
            }
        }
        _historical[_end] = stat;
    }

    void tick() {
        pipe->push(drawValueForwarder, this);
        if (_enableChart) {
            pipe->push(drawChartForwarder, this);
        }
    }

    void draw() {
        // no op, see tick
    }

    void drawLabel() {
        if (!_hideLabel) {
            tft.setCursor(_pos.x, _pos.y+1);
            tft.setTextColor(ILI9341_WHITE);
            tft.print(_labelText);
        }
        tft.fillRect(_value.x, _value.y, _controlWidth, 17, 0x2104);
        _lastDrawWidth = _controlWidth;
    }

    void drawValue() {
        char val[8];
        int chars;
        if (validReadingi(_stat)) {
            chars = sprintf(val, "%d", _stat);
        } else {
            chars = sprintf(val, "-- ");
        }
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
            _cur = 0;
            tft.fillRect(_chart.x, _chart.y, _chartWidth, 31 /* to catch the bottom of the unit on the axis */,
                         ILI9341_BLACK);
            tft.setCursor(_chart.x + 27 - ((int16_t) strlen(_labelText) * 6), _chart.y + 12);
            tft.setTextColor(ILI9341_WHITE);
            tft.setTextSize(1);
            tft.print(_labelText);
            tft.setCursor(_chart.x + _chartWidth - 45, _chart.y);
            if (_invertChart) {
                tft.print("-");
                tft.print(_min);
            } else {
                tft.print(_max);
            }

            if (_unitText) {
                tft.setTextColor(_chartColor);
                tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 12);
                tft.print(_unitText);
                tft.setTextColor(ILI9341_WHITE);
            }
            tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 24);
            if (_invertChart) {
                tft.print("-");
                tft.print(_max);
            } else {
                tft.print(_min);
            }
            tft.setTextSize(2);
            _redrawChart = false;
            tft.fillRect(_chart.x + 30, _chart.y, _chartWidth - 80, 30, 0x2104);
        }

        for ( ; _cur < _end; _cur++) {
            if (invalidReadingi(_historical[_cur])) {
                tft.drawFastVLine(_chart.x+30+_cur, _chart.y, 30, ILI9341_MAROON);
                break;
            }
            float norm = ((float) abs(_historical[_cur]))/(_max);
            int x = _chart.x+30+_cur;
            int y;
            if (!_invertChart) {
                y = _chart.y+29-((int) round(norm*29));
            } else {
                y = _chart.y+((int) round(norm*29));
            }
            if (y < _chart.y || y > _chart.y+29) {
                break;
            }
            tft.fillRect(x, y, 1, 1, _chartColor);
        }
    }
};

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;

public:
    void tick() {
        _btnExit->tick();
    }

    void draw() {
        tft.fillScreen(0x2104);
    }

    void init() {
        pipe->push(drawViewControllerForwarder, this);

        _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 40, h: 10});
        _btnExit->setLabel("x");
        _btnExit->then([](void *context) {
            pipe->segueBack();
        }, NULL);
        pipe->push(drawControlForwarder, _btnExit);
    }

    void deinit() {
        _btnExit = NULL;
    }
};

class DashboardViewController : public ViewController {
private:
    Button *_btnSettings;
    Stat *_ack;
    Stat *_lag;
    Stat *_rssi;
    Stat *_rrssi;
    Stat *_vib;
    Stat *_vcc;
    Stat *_rvcc;
    Stat *_altitude;

    unsigned long _lastUpdate = 0;

public:

    DashboardViewController() : ViewController() { }

    ~DashboardViewController() {
        deinit();
    }

    void tick() {
        _btnSettings->tick();

        if (_lastUpdate < lastUpdate) {
            _ack->set(sinceLastAck);
            _vcc->set(lastVcc);
            _lag->set(lastRoundtrip);
            _rssi->set(lastRssi);

            _rvcc->set(remote.lastVcc);
            _vib->set(remote.lastVibration);
            _rrssi->set(remote.lastRssi);
            _altitude->set(remote.lastAltitude);

            _lastUpdate = lastUpdate;

            _ack->tick();
            _lag->tick();
            _vib->tick();
            _rssi->tick();
            _rrssi->tick();
            _vcc->tick();
            _rvcc->tick();
            _altitude->tick();
        }
    }

    void draw() {
        tft.fillScreen(ILI9341_BLACK);
    }

    void init() {
        pipe->push(drawViewControllerForwarder, this);

        _btnSettings = new Button(Point{x: 10, y: 280}, Size{w: 150, h: 30});
        _btnSettings->setLabel("Settings");
        _btnSettings->then([](void *context) {
            Serial.println("Clicked settings button!");
            pipe->segueTo(new SettingsViewController());
        }, NULL);
        pipe->push(drawControlForwarder, _btnSettings);

        _ack = new Stat(10, 10, 50, 10, 10, 90);
        _ack->setLabel("ack");
        pipe->push(drawLabelForwarder, _ack);

        _lag = new Stat(120, 10, 170, 10, 10, 90);
        _lag->setLabel("lag");
        _lag->setUnit("ms");
        _lag->setColor(ILI9341_BLUE);
        _lag->setEnableChart(true);
        pipe->push(drawLabelForwarder, _lag);
        pipe->push(drawChartForwarder, _lag);

        _rssi = new Stat(10, 30, 10, 30, 10, 125);
        _rssi->setLabel("rssi");
        _rssi->setHideLabel(true);
        _rssi->setUnit("dBm");
        _rssi->setColor(ILI9341_PURPLE);
        _rssi->setInvert(true);
        _rssi->setEnableChart(true);
        pipe->push(drawLabelForwarder, _rssi);
        pipe->push(drawChartForwarder, _rssi);

        _rrssi = new Stat(120, 30, 120, 30, 10, 160);
        _rrssi->setLabel("rrssi");
        _rrssi->setHideLabel(true);
        _rrssi->setUnit("dBm");
        _rrssi->setColor(ILI9341_CYAN);
        _rrssi->setInvert(true);
        _rrssi->setEnableChart(true);
        pipe->push(drawLabelForwarder, _rrssi);
        pipe->push(drawChartForwarder, _rrssi);

        _vcc = new Stat(10, 50, 50, 50, 10, 195);
        _vcc->setChartWidth(130);
        _vcc->setLabel("vcc");
        _vcc->setUnit("mV");
        pipe->push(drawLabelForwarder, _vcc);

        _rvcc = new Stat(120, 50, 170, 50, 120, 195);
        _rvcc->setChartWidth(130);
        _rvcc->setLabel("rvcc");
        _rvcc->setUnit("mV");
        pipe->push(drawLabelForwarder, _rvcc);

        _vib = new Stat(10, 70, 50, 70, 10, 195);
        _vib->setLabel("vib");
        _vib->setColor(ILI9341_GREEN);
        _vib->setEnableChart(true);
        pipe->push(drawLabelForwarder, _vib);
        pipe->push(drawChartForwarder, _vib);

        _altitude = new Stat(120, 70, 170, 70, 10, 230);
        _altitude->setLabel("alt");
        _altitude->setUnit("m");
        _altitude->setColor(ILI9341_MAGENTA);
        _altitude->setEnableChart(true);
        pipe->push(drawLabelForwarder, _altitude);
    }

    void deinit() {
        _btnSettings = NULL;
        _ack = NULL;
        _lag = NULL;
        _rssi = NULL;
        _rrssi = NULL;
        _vcc = NULL;
        _rvcc = NULL;
        _vib = NULL;
        _altitude = NULL;
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

void onMessageReceived(Message *msg) {
    unsigned long ack = millis();

    const char *body = msg->getBody();
#ifdef DEBUGV
    Serial.print("received ");
    Serial.print(msg->size);
    Serial.println(" bytes");
#endif
    // expects "ack<data>"
    if (body[0] != 'a' || body[1] != 'c' || body[2] != 'k') {
        Serial.println("received non-ack or malformed");
        return;
    }

    lastAck = ack;
    lastRssi = msg->rssi;
    lastRoundtrip = (int) (lastAck-lastSent);

    unpack((char *)body, remote);
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

        Message msg("helo");
        radio->send(&msg);
    }

    // Update metrics.
    // TODO: This should probably happen after radio->tick but before pipe->tick.
    if (tick - lastUpdate > UPDATE_WAIT) {
        lastUpdate = tick;
        lastVcc = readVcc();

        sinceLastAck = (int) lround((lastUpdate - lastAck) / 1000.0);

        if (lastAck + TIMEOUT_WAIT < lastSent) {
            remote.setNoReading();
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
        }
    }
}


#endif