#include "settings.h"
#include "vibration_offset.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void SettingsViewController::tick() {
    _btnExit->tick();
    _btnConfigVib->tick();
}

void SettingsViewController::draw() {
    tft.fillScreen(0x2104);
}

void SettingsViewController::init() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _btnConfigVib = new Button(Point{x: 10, y: 10}, Size{w: 150, h: 30});
    _btnConfigVib->setLabel("Config Vib");
    _btnConfigVib->fontSize = 2;
    _btnConfigVib->then([](void *context) {
        pipe->seguePopover(new VibrationConfigViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnConfigVib);


}
