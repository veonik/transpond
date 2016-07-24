#ifndef _SETTINGS_H_
#define TRANSPOND_SETTINGS_H_H

#include "gui.h"

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;
    Button *_btnGraphicsTest;

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
    }
};


#endif
