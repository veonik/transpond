#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "gui.h"

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnVibOffset;

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
        delete _btnVibOffset;
    }
};


#endif
