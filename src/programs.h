#ifndef _PROGRAMS_H_
#define _PROGRAMS_H_

#include "gui.h"

class ProgramsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnGraphicsTest;
    Button *_btnInputTest;
    Button *_btnPosition;

public:
    ProgramsViewController() : ViewController() { }

    ~ProgramsViewController() {
        deinit();
    }

    void tick();

    void draw();

    void init();

    void deinit() {
        delete _btnExit;
        delete _btnGraphicsTest;
        delete _btnInputTest;
        delete _btnPosition;
    }
};


#endif
