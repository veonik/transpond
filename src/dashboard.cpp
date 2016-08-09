#include "dashboard.h"
#include "message.h"
#include "settings.h"
#include "programs.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics remote;

extern unsigned long lastTick;
extern unsigned long lastUpdate;
extern unsigned long lastAck;
extern unsigned long lastSent;
extern int lastRssi;              // dBm
extern int lastRoundtrip;         // ms
extern int lastVcc;               // mV
extern int sinceLastAck;          // sec

void drawLabelForwarder(void* context) {
    static_cast<Stat*>(context)->drawLabel();
}
void drawValueForwarder(void* context) {
    static_cast<Stat*>(context)->drawValue();
}
void drawChartForwarder(void* context) {
    static_cast<Stat*>(context)->drawChart();
}


void DashboardViewController::tick() {
    _btnSettings->tick();
    _btnPrograms->tick();
    _btnClear->tick();

    if (_lastUpdate < lastUpdate) {
        _ack->set(sinceLastAck);
        _vcc->set(lastVcc);
        _lag->set(lastRoundtrip);
        _rssi->set(lastRssi);

        _rvcc->set(remote.vcc);
        _vib->set(remote.vibration);
        _rrssi->set(remote.rssi);
        _altitude->set(remote.altitude);

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

void DashboardViewController::draw() {
    tft.fillScreen(bgColor);
}

void DashboardViewController::init() {
    bgColor = ILI9341_BLACK;
    pipe->push(drawViewControllerForwarder, this);

    _btnSettings = new Button(Point{x: 10, y: 280}, Size{w: 60, h: 30});
    _btnSettings->bgColor = tft.color565(75, 75, 75);
    _btnSettings->setLabel("SET");
    _btnSettings->then([](void *context) {
        pipe->seguePopover(new SettingsViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnSettings);

    _btnPrograms = new Button(Point{x: 80, y: 280}, Size{w: 60, h: 30});
    _btnPrograms->bgColor = tft.color565(75, 75, 75);
    _btnPrograms->setLabel("PROG");
    _btnPrograms->then([](void *context) {
        pipe->seguePopover(new ProgramsViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnPrograms);

    _btnClear = new Button(Point{x: 170, y: 280}, Size{w: 60, h: 30});
    _btnClear->bgColor = tft.color565(75, 75, 75);
    _btnClear->setLabel("CLR");
    _btnClear->then([](void *context) {
        DashboardViewController* vc = static_cast<DashboardViewController*>(context);
        vc->deinit();
        vc->init();
    }, this);
    pipe->push(drawControlForwarder, _btnClear);

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

void Stat::setEnableChart(bool enable) {
    _enableChart = enable;
}

void Stat::setChartWidth(int width) {
    _chartWidth = width;
}

void Stat::setLabel(const char *label) {
    _labelText = label;
}

void Stat::setHideLabel(bool hide) {
    _hideLabel = hide;
    _controlWidth = !_hideLabel ? 60 : 100;
}

void Stat::setUnit(const char *unit) {
    _unitText = unit;
}

void Stat::setColor(int color) {
    _chartColor = (uint16_t) color;
}

void Stat::setInvert(bool invert) {
    _invertChart = invert;
}

void Stat::set(float stat) {
    if (invalidReadingf(stat)) {
        return set(NO_READING_INT);
    }
    set((int) stat);
}

void Stat::set(int stat) {
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

void Stat::tick() {
    pipe->push(drawValueForwarder, this);
    if (_enableChart) {
        pipe->push(drawChartForwarder, this);
    }
}

void Stat::draw() {
    // no op, see tick
}

void Stat::drawLabel() {
    if (!_hideLabel) {
        tft.setFont(&Inconsolata_g8pt7b);
        tft.setCursor(_pos.x, _pos.y+1+CURSOR_Y_LARGE);
        tft.setTextColor(ILI9341_WHITE);
        tft.print(_labelText);
    }
    tft.fillRect(_value.x, _value.y, _controlWidth, 17, tft.color565(10, 10, 10));
    _lastDrawWidth = _controlWidth;
}

void Stat::drawValue() {
    char valStr[8];
    int valLength;
    if (validReadingi(_stat)) {
        valLength = sprintf(valStr, "%d", _stat);
    } else {
        valLength = sprintf(valStr, "-- ");
    }
    if (_lastValLength > 0) {
        if (_lastValDrawn == _stat) {
            return;
        }

        tft.setCursor(_value.x, _value.y + 1 + CURSOR_Y_LARGE);
        tft.setFont(&Inconsolata_g8pt7b);
        tft.setTextColor(tft.color565(10, 10, 10));
        tft.print(_lastVal);


        if (valLength != _lastValLength) {
            tft.setFont(&Inconsolata_g5pt7b);
            tft.print(_unitText);
        }
    }

    tft.setCursor(_value.x, _value.y+1+CURSOR_Y_LARGE);
    tft.setFont(&Inconsolata_g8pt7b);
    tft.setTextColor(ILI9341_WHITE, tft.color565(10, 10, 10));
    tft.print(valStr);
    tft.setFont(&Inconsolata_g5pt7b);
    if (valLength != _lastValLength) {
        tft.setFont(&Inconsolata_g5pt7b);
        tft.print(_unitText);
    }
    memcpy(_lastVal, valStr, valLength+1);
    _lastValDrawn = _stat;
    _lastValLength = valLength;
}

void Stat::drawChart() {
    if (_redrawChart) {
        _cur = 0;
        tft.fillRect(_chart.x, _chart.y - 1 /* catch the top of the unit of the axis */,
                     _chartWidth, 31 /* to catch the bottom of the unit on the axis */,
                     ILI9341_BLACK);
        tft.setCursor(_chart.x + 27 - ((int16_t) strlen(_labelText) * 6), _chart.y + 12+CURSOR_Y_SMALL);
        tft.setTextColor(ILI9341_WHITE);
        tft.setFont(&Inconsolata_g5pt7b);
        tft.print(_labelText);
        tft.setCursor(_chart.x + _chartWidth - 45, _chart.y+CURSOR_Y_SMALL);
        if (_invertChart) {
            tft.print("-");
            tft.print(_min);
        } else {
            tft.print(_max);
        }

        if (_unitText) {
            tft.setTextColor(_chartColor);
            tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 12+CURSOR_Y_SMALL);
            tft.print(_unitText);
            tft.setTextColor(ILI9341_WHITE);
        }
        tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 24+CURSOR_Y_SMALL);
        if (_invertChart) {
            tft.print("-");
            tft.print(_max);
        } else {
            tft.print(_min);
        }
        tft.setFont(&Inconsolata_g8pt7b);
        _redrawChart = false;
        tft.fillRect(_chart.x + 30, _chart.y, _chartWidth - 80, 30, tft.color565(10, 10, 10));
    }

    for ( ; _cur < _end; _cur++) {
        if (invalidReadingi(_historical[_cur])) {
            tft.drawFastVLine(_chart.x+30+_cur, _chart.y, 30, ILI9341_MAROON);
            continue;
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
            continue;
        }
        tft.fillRect(x, y, 1, 1, _chartColor);
    }
}