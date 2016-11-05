#include "programs.h"
#include "graphics_test.h"
#include "position.h"
#include "input.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void ProgramsViewController::tick() {
    _btnExit->tick();
    _btnGraphicsTest->tick();
    _btnInputTest->tick();
    _btnPosition->tick();
}

void ProgramsViewController::draw() {
    tft.fillScreen(0x2104);
}

void ProgramsViewController::init() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _btnGraphicsTest = new Button(Point{x: 10, y: 10}, Size{w: 150, h: 30});
    _btnGraphicsTest->setLabel("Graphics Test");
    _btnGraphicsTest->fontSize = 2;
    _btnGraphicsTest->then([](void *context) {
        pipe->seguePopover(new GraphicsTestViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnGraphicsTest);

    _btnInputTest = new Button(Point{x: 10, y: 50}, Size{w: 150, h: 30});
    _btnInputTest->setLabel("Input Test");
    _btnInputTest->fontSize = 2;
    _btnInputTest->then([](void *context) {
        pipe->seguePopover(new InputViewController("", [](void *context, const char *result) {
            Serial.println(result);
        }));
    }, NULL);
    pipe->push(drawControlForwarder, _btnInputTest);

    _btnPosition = new Button(Point{x: 10, y: 90}, Size{w: 150, h: 30});
    _btnPosition->setLabel("Positioning");
    _btnPosition->fontSize = 2;
    _btnPosition->then([](void *context) {
        pipe->seguePopover(new PositionViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnPosition);
}
