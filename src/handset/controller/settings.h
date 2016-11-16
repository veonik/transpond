#ifndef TRANSPOND_SETTINGS_H
#define TRANSPOND_SETTINGS_H

#include "../gui.h"

class SettingsViewController : public ViewController {
private:
    Button *_btnExit;

    Button *_btnLogConfig;

protected:
    void doInit();

    void doDeInit() {
        delete _btnExit;
        delete _btnLogConfig;
    }

public:
    SettingsViewController() : ViewController() { }

    ~SettingsViewController() { deinit(); }

    void tick();

    void draw();
};


#endif
