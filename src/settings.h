#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "gui.h"

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnGraphicsTest;
    Button *_btnPosition;

public:
    SettingsViewController() : ViewController() { }

    ~SettingsViewController() {
        deinit();
    }

    void tick();

    void draw();

    void init();

    void deinit() {
        delete _btnExit;
        delete _btnGraphicsTest;
        delete _btnPosition;
    }
};


#endif
