#include "settings.h"
#include "dashboard.h"
#include "graphics_test.h"
#include "position.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void SettingsViewController::tick() {
    _btnExit->tick();
    _btnGraphicsTest->tick();
    _btnPosition->tick();
}

void SettingsViewController::draw() {
    tft.fillScreen(0x2104);
}

void SettingsViewController::init() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueTo(new DashboardViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _btnGraphicsTest = new Button(Point{x: 10, y: 10}, Size{w: 150, h: 30});
    _btnGraphicsTest->setLabel("Graphics Test");
    _btnGraphicsTest->then([](void *context) {
        pipe->segueTo(new GraphicsTestViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnGraphicsTest);

    _btnPosition = new Button(Point{x: 10, y: 50}, Size{w: 150, h: 30});
    _btnPosition->setLabel("Positioning");
    _btnPosition->then([](void *context) {
        pipe->segueTo(new PositionViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnPosition);
}
