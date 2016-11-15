#ifndef TRANSPOND_DASHBOARD_H
#define TRANSPOND_DASHBOARD_H

#include <limits.h>
#include "handset/gui.h"

class DashboardViewController : public ViewController {
private:
    Button *_btnSettings;
    Button *_btnPrograms;
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
    bool _init = false;

public:
    uint16_t bgColor = ILI9341_BLACK;

    DashboardViewController() : ViewController() { }

    ~DashboardViewController() {
        if (_init) {
            deinit();
        }
    }

    void tick();

    void draw();

    void init();

    void deinit() {
        _init = false;
        delete _btnSettings;
        delete _btnPrograms;
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