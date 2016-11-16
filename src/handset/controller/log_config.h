#ifndef TRANSPOND_VIB_OFFSET_H
#define TRANSPOND_VIB_OFFSET_H

#include "../gui.h"

class LogSettingsViewController : public ViewController {
private:
    const unsigned long UPDATE_WAIT = 500;

    Button *_btnExit;

    Label *_lblLocalEnabled;
    Textbox *_txtLocalEnabled;
    Button *_btnLocalToggle;

    Label *_lblRemoteEnabled;
    Textbox *_txtRemoteEnabled;
    Button *_btnRemoteToggle;

    unsigned long _lastUpdate = 0;

protected:
    void doInit();

    void doDeInit() {
        delete _btnExit;
        delete _btnLocalToggle;
        delete _lblLocalEnabled;
        delete _txtLocalEnabled;
        delete _btnRemoteToggle;
        delete _lblRemoteEnabled;
        delete _txtRemoteEnabled;
    }

public:
    LogSettingsViewController() : ViewController() { }

    ~LogSettingsViewController() { deinit(); }

    void tick();

    void draw();
};


#endif
