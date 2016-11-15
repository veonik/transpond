#ifndef TRANSPOND_DASHBOARD_CONFIG_H
#define TRANSPOND_DASHBOARD_CONFIG_H

#include "handset/gui.h"

class DashboardConfigViewController : public ViewController {
private:
    Button *_btnExit;

public:
    DashboardConfigViewController() : ViewController() { }

    ~DashboardConfigViewController() {
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
