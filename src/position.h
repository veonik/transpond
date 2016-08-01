#ifndef _POSITION_H_
#define _POSITION_H_

#include "gui.h"

class PositionViewController : public ViewController {
private:
    const unsigned long UPDATE_WAIT = 250;

    Button *_btnExit;
    Label *_lblXValue;
    Textbox *_txtXValue;
    Label *_lblYValue;
    Textbox *_txtYValue;
    Label *_lblZValue;
    Textbox *_txtZValue;

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
        delete _lblXValue;
        delete _txtXValue;
        delete _lblYValue;
        delete _txtYValue;
        delete _lblZValue;
        delete _txtZValue;
    }
};


#endif
