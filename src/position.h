#ifndef _POSITION_H_
#define _POSITION_H_

#include "gui.h"

class PositionViewController : public ViewController {
private:
    const unsigned long UPDATE_WAIT = 250;

    Button *_btnExit;

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
