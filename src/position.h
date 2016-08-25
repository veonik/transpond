#ifndef TRANSPOND_POSITION_H
#define TRANSPOND_POSITION_H

#include "gui.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_10DOF.h>

class PositionViewController : public ViewController {
private:
    const unsigned long UPDATE_WAIT = 250;

    Button *_btnExit;
    Button *_btnCenter;
    Label *_lblXValue;
    Textbox *_txtXValue;
    Label *_lblYValue;
    Textbox *_txtYValue;
    Label *_lblZValue;
    Textbox *_txtZValue;

    Label *_lblX2Value;
    Textbox *_txtX2Value;
    Label *_lblY2Value;
    Textbox *_txtY2Value;
    Label *_lblZ2Value;
    Textbox *_txtZ2Value;

    Textbox *_txtPositionLat;
    Textbox *_txtPositionLong;

    Adafruit_10DOF _dof;
    sensors_event_t _accel_event;
    sensors_event_t _mag_event;
    sensors_vec_t _orientation;

    long _centerLat;
    long _centerLong;

    unsigned long _lastUpdate = 0;

public:
    PositionViewController() : ViewController() {
        _dof = Adafruit_10DOF();
    }

    ~PositionViewController() {
        deinit();
    }

    void tick();

    void draw();

    void drawPRH();
    void drawPRH2();
    void drawPos();

    void center();

    void init();

    void deinit() {
        delete _btnExit;
        delete _btnCenter;
        delete _lblXValue;
        delete _txtXValue;
        delete _lblYValue;
        delete _txtYValue;
        delete _lblZValue;
        delete _txtZValue;
        delete _lblX2Value;
        delete _txtX2Value;
        delete _lblY2Value;
        delete _txtY2Value;
        delete _lblZ2Value;
        delete _txtZ2Value;
        delete _txtPositionLat;
        delete _txtPositionLong;
    }
};


#endif
