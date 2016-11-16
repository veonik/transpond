#include "settings.h"
#include "log_config.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void SettingsViewController::tick() {
    _btnExit->tick();
    _btnLogConfig->tick();
}

void SettingsViewController::draw() {
    tft.fillScreen(0x2104);
}

void SettingsViewController::doInit() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _btnLogConfig = new Button(Point{x: 10, y: 10}, Size{w: 150, h: 30});
    _btnLogConfig->setLabel("Config Logging");
    _btnLogConfig->fontSize = 2;
    _btnLogConfig->then([](void *context) {
        pipe->seguePopover(new LogSettingsViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnLogConfig);


}
