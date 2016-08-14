#ifndef TRANSPOND_SETTINGS_H
#define TRANSPOND_SETTINGS_H

#include "gui.h"

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnConfigVib;

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
        delete _btnConfigVib;
    }
};


#endif
