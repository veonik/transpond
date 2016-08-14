#ifndef TRANSPOND_VIB_OFFSET_H
#define TRANSPOND_VIB_OFFSET_H

#include "gui.h"

class VibrationConfigViewController : public ViewController {
private:
    Button *_btnExit;

public:
    VibrationConfigViewController() : ViewController() { }

    ~VibrationConfigViewController() {
        deinit();
    }

    void tick();

    void draw();

    void init();

    void deinit() {
        delete _btnExit;
    }
};


#endif
