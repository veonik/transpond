#ifndef _POSITION_H_
#define _POSITION_H_

#include "gui.h"

const uint16_t COLOR_X = ILI9341_RED;
const uint16_t COLOR_Y = ILI9341_GREEN;
const uint16_t COLOR_Z = ILI9341_BLUE;

const unsigned long UPDATE_WAIT = 1000;

class PositionViewController : public ViewController {
private:
    Button *_btnExit;
    Button *_btnCalibrate;

    float _zeroX;
    float _zeroY;
    float _zeroZ;

    bool _calibrated = false;
    int _calibration;
    float _calibrationX = 0.0;
    float _calibrationY = 0.0;
    float _calibrationZ = 0.0;

    unsigned long _lastUpdate = 0;

public:
    PositionViewController() : ViewController() { }

    ~PositionViewController() {
        deinit();
    }

    void calibrate();

    void tick();

    void draw();

    void init();

    void deinit() {
        delete _btnExit;
    }
};


#endif
