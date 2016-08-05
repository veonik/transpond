#ifndef _VIB_OFFSET_H_
#define _VIB_OFFSET_H_

#include "gui.h"

class VibrationOffsetViewController : public ViewController {
private:
    Button *_btnExit;

public:
    VibrationOffsetViewController() : ViewController() { }

    ~VibrationOffsetViewController() {
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
