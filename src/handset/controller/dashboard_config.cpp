#include "dashboard_config.h"
#include "log_config.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void DashboardConfigViewController::tick() {
    _btnExit->tick();
}

void DashboardConfigViewController::draw() {
    tft.fillScreen(0x2104);
}

void DashboardConfigViewController::init() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);
}
