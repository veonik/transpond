#ifndef TRANSPOND_DASHBOARD2_H
#define TRANSPOND_DASHBOARD2_H

#include <limits.h>
#include "handset/gui.h"

class Dashboard2ViewController : public ViewController {
private:
    Button *_btnSettings;
    Button *_btnPrograms;
    Button *_btnConfig;
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

    Dashboard2ViewController() : ViewController() { }

    ~Dashboard2ViewController() {
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
        delete _btnConfig;
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