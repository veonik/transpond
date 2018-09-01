#include "programs.h"
#include "graphics_test.h"
#include "position.h"
#include "input.h"
#include "dashboard.h"
#include "dashboard2.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void ProgramsViewController::tick() {
    _btnExit->tick();
    _btnGraphicsTest->tick();
    _btnInputTest->tick();
    _btnPosition->tick();
    _btnDashboard2->tick();
    _btnDashboard->tick();
}

void ProgramsViewController::draw() {
    tft.fillScreen(0x2104);
}

void ProgramsViewController::doInit() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, nullptr);
    pipe->push(drawControlForwarder, _btnExit);

    _btnDashboard = new Button(Point{x: 10, y: 10}, Size{w: 150, h: 30});
    _btnDashboard->setLabel("Dashboard");
    _btnDashboard->fontSize = 2;
    _btnDashboard->then([](void *context) {
        pipe->segueTo(new DashboardViewController());
    }, nullptr);
    pipe->push(drawControlForwarder, _btnDashboard);

    _btnDashboard2 = new Button(Point{x: 10, y: 50}, Size{w: 150, h: 30});
    _btnDashboard2->setLabel("Dashboard 2.0");
    _btnDashboard2->fontSize = 2;
    _btnDashboard2->then([](void *context) {
        pipe->segueTo(new Dashboard2ViewController());
    }, nullptr);
    pipe->push(drawControlForwarder, _btnDashboard2);

    _btnGraphicsTest = new Button(Point{x: 10, y: 90}, Size{w: 150, h: 30});
    _btnGraphicsTest->setLabel("Graphics Test");
    _btnGraphicsTest->fontSize = 2;
    _btnGraphicsTest->then([](void *context) {
        pipe->seguePopover(new GraphicsTestViewController());
    }, nullptr);
    pipe->push(drawControlForwarder, _btnGraphicsTest);

    _btnInputTest = new Button(Point{x: 10, y: 130}, Size{w: 150, h: 30});
    _btnInputTest->setLabel("Input Test");
    _btnInputTest->fontSize = 2;
    _btnInputTest->then([](void *context) {
        pipe->seguePopover(new InputViewController("", [](void *ctx, const char *result) {
            static_cast<ProgramsViewController*>(ctx)->_btnInputTest->setLabel(result);
            Serial.println(result);
        }, context));
    }, this);
    pipe->push(drawControlForwarder, _btnInputTest);

    _btnPosition = new Button(Point{x: 10, y: 170}, Size{w: 150, h: 30});
    _btnPosition->setLabel("Positioning");
    _btnPosition->fontSize = 2;
    _btnPosition->then([](void *context) {
        pipe->seguePopover(new PositionViewController());
    }, nullptr);
    pipe->push(drawControlForwarder, _btnPosition);
}
