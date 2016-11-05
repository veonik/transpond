#include <synack.h>
#include "log_config.h"
#include "common/message.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics m;

extern Radio *radio;

void LogSettingsViewController::tick() {
    _btnExit->tick();
    _btnLocalToggle->tick();
    _btnRemoteToggle->tick();

    unsigned long update = millis();
    if (update >= _lastUpdate + UPDATE_WAIT) {
        _lastUpdate = update;
        _txtLocalEnabled->setValue("TODO");
        pipe->push(drawControlForwarder, _txtLocalEnabled);
        _txtRemoteEnabled->setValue(m.logging == MODULE_ENABLED ? "ON" : "OFF");
        pipe->push(drawControlForwarder, _txtRemoteEnabled);
    }
}

void LogSettingsViewController::draw() {
    tft.fillScreen(0x2104);
}

void LogSettingsViewController::init() {
    pipe->push(drawViewControllerForwarder, this);

    _btnExit = new Button(Point{x: 200, y: 10}, Size{w: 30, h: 30});
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _lblLocalEnabled = new Label(Point{x: 10, y: 50}, Size{w: 25, h: 25});
    _lblLocalEnabled->setLabel("Local");
    _lblLocalEnabled->fontSize = 2;
    _lblLocalEnabled->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblLocalEnabled);

    _txtLocalEnabled = new Textbox(Point{x: 50, y: 50}, Size{w: 50, h: 25});
    _txtLocalEnabled->fontSize = 2;
    _txtLocalEnabled->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtLocalEnabled);

    _btnLocalToggle = new Button(Point{x: 120, y: 50}, Size{w: 100, h: 30});
    _btnLocalToggle->setLabel("Toggle");
    _btnLocalToggle->fontSize = 2;
    _btnLocalToggle->then([](void *context) {
        // TODO: Actually disable local logging

    }, NULL);
    pipe->push(drawControlForwarder, _btnLocalToggle);

    _lblRemoteEnabled = new Label(Point{x: 10, y: 90}, Size{w: 25, h: 25});
    _lblRemoteEnabled->setLabel("Remote");
    _lblRemoteEnabled->fontSize = 2;
    _lblRemoteEnabled->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblRemoteEnabled);

    _txtRemoteEnabled = new Textbox(Point{x: 50, y: 90}, Size{w: 50, h: 30});
    _txtRemoteEnabled->fontSize = 2;
    _txtRemoteEnabled->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtRemoteEnabled);

    _btnRemoteToggle = new Button(Point{x: 120, y: 90}, Size{w: 100, h: 30});
    _btnRemoteToggle->setLabel("Toggle");
    _btnRemoteToggle->fontSize = 2;
    _btnRemoteToggle->then([](void *context) {
        if (m.logging != MODULE_ENABLED) {
            Serial.println(F("Requesting to enable logging"));
        } else {
            Serial.println(F("Requesting to disable logging"));
        }
        Message msg("tl");
        radio->send(&msg);
        delay(10);
    }, NULL);
    pipe->push(drawControlForwarder, _btnRemoteToggle);
}
