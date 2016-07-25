#ifndef _DASHBOARD_H_
#define _DASHBOARD_H_

#include "gui.h"

class Stat : public Control {
private:
    Point _value;
    Point _chart;

    int _chartWidth = 240;
    int _controlWidth = 60;
    int _lastDrawWidth = 60;
    const char *_labelText = "";
    const char *_unitText = "";

    int _lastValDrawn;
    int _lastValLength = 0;
    char _lastVal[8];

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

    ~Stat() { }

    void setEnableChart(bool enable);

    void setChartWidth(int width);

    void setLabel(const char *label);

    void setHideLabel(bool hide);

    void setUnit(const char *unit);

    void setColor(int color);

    void setInvert(bool invert);

    void set(float stat);

    void set(int stat);

    void tick();

    void draw();

    void drawLabel();

    void drawValue();

    void drawChart();
};

class DashboardViewController : public ViewController {
private:
    Button *_btnSettings;
    Button *_btnProgram;
    Button *_btnClear;
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
    uint16_t bgColor = ILI9341_BLACK;

    DashboardViewController() : ViewController() { }

    ~DashboardViewController() {
        deinit();
    }

    void tick();

    void draw();

    void init();

    void deinit() {
        delete _btnSettings;
        delete _btnProgram;
        delete _btnClear;
        delete _ack;
        delete _lag;
        delete _rssi;
        delete _rrssi;
        delete _vcc;
        delete _rvcc;
        delete _vib;
        delete _altitude;
    }
};


#endif