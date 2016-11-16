#ifndef TRANSPOND_DASHBOARD_CONFIG_H
#define TRANSPOND_DASHBOARD_CONFIG_H

#include "../gui.h"

class DashboardConfigViewController : public ViewController {
private:
    Button *_btnExit;

protected:
    void doInit();

    void doDeInit() {
        delete _btnExit;
    }

public:
    DashboardConfigViewController() : ViewController() { }

    ~DashboardConfigViewController() { deinit(); }

    void tick();

    void draw();
};


#endif
