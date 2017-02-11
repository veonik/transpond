#ifndef TRANSPOND_PROGRAMS_H
#define TRANSPOND_PROGRAMS_H

#include "../gui.h"

class ProgramsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnGraphicsTest;
    Button *_btnInputTest;
    Button *_btnPosition;
    Button *_btnDashboard2;
    Button *_btnDashboard;

protected:
    void doInit();

    void doDeInit() {
        delete _btnExit;
        delete _btnGraphicsTest;
        delete _btnInputTest;
        delete _btnPosition;
        delete _btnDashboard2;
        delete _btnDashboard;
    }

public:
    ProgramsViewController() : ViewController() { }

    ~ProgramsViewController() { }

    void tick();

    void draw();
};


#endif
