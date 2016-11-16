#include "dashboard2.h"
#include "../../common/message.h"
#include "settings.h"
#include "programs.h"
#include "dashboard_config.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics m;

extern unsigned long lastTick;
extern unsigned long lastUpdate;
extern unsigned long lastAck;
extern unsigned long lastSent;
extern int lastRssi;              // dBm
extern int lastRoundtrip;         // ms
extern int lastVcc;               // mV
extern int sinceLastAck;          // sec

void Dashboard2ViewController::tick() {
    _btnSettings->tick();
    _btnPrograms->tick();
    _btnConfig->tick();

    if (_lastUpdate < lastUpdate) {
        _ack->set(sinceLastAck);
        _vcc->set(lastVcc);
        _lag->set(lastRoundtrip);
        _rssi->set(lastRssi);

        _rvcc->set(m.vcc);
        _vib->set(m.vibration);
        _rrssi->set(m.rssi);
        _altitude->set(m.altitudeGps);

        _lastUpdate = lastUpdate;

        _ack->tick();
        _lag->tick();
        _vib->tick();
        _rssi->tick();
        _rrssi->tick();
        _vcc->tick();
        _rvcc->tick();
        _altitude->tick();
    }
}

void Dashboard2ViewController::draw() {
    tft.fillScreen(bgColor);
}

void Dashboard2ViewController::doInit() {
    bgColor = ILI9341_BLACK;
    pipe->push(drawViewControllerForwarder, this);

    _btnSettings = new Button(Point{x: 10, y: 280}, Size{w: 60, h: 30});
    _btnSettings->bgColor = tft.color565(75, 75, 75);
    _btnSettings->setLabel("SET");
    _btnSettings->then([](void *context) {
        pipe->seguePopover(new SettingsViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnSettings);

    _btnPrograms = new Button(Point{x: 80, y: 280}, Size{w: 60, h: 30});
    _btnPrograms->bgColor = tft.color565(75, 75, 75);
    _btnPrograms->setLabel("PROG");
    _btnPrograms->then([](void *context) {
        pipe->seguePopover(new ProgramsViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnPrograms);

    _btnConfig = new Button(Point{x: 170, y: 280}, Size{w: 60, h: 30});
    _btnConfig->bgColor = tft.color565(75, 75, 75);
    _btnConfig->setLabel("CFG");
    _btnConfig->then([](void *context) {
        pipe->seguePopover(new DashboardConfigViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnConfig);

    _ack = new Stat(10, 10, 50, 10, 10, 90);
    _ack->setLabel("ack");
    pipe->push(drawLabelForwarder, _ack);

    _lag = new Stat(120, 10, 170, 10, 10, 90);
    _lag->setLabel("lag");
    _lag->setUnit("ms");
    _lag->setColor(ILI9341_BLUE);
    _lag->setEnableChart(true);
    pipe->push(drawLabelForwarder, _lag);
    pipe->push(drawChartForwarder, _lag);

    _rssi = new Stat(10, 30, 10, 30, 10, 125);
    _rssi->setLabel("rssi");
    _rssi->setHideLabel(true);
    _rssi->setUnit("dBm");
    _rssi->setColor(ILI9341_PURPLE);
    _rssi->setEnableChart(true);
    pipe->push(drawLabelForwarder, _rssi);
    pipe->push(drawChartForwarder, _rssi);

    _rrssi = new Stat(120, 30, 120, 30, 10, 160);
    _rrssi->setLabel("rrssi");
    _rrssi->setHideLabel(true);
    _rrssi->setUnit("dBm");
    _rrssi->setColor(ILI9341_CYAN);
    _rrssi->setEnableChart(true);
    pipe->push(drawLabelForwarder, _rrssi);
    pipe->push(drawChartForwarder, _rrssi);

    _vcc = new Stat(10, 50, 50, 50, 10, 195);
    _vcc->setChartWidth(130);
    _vcc->setLabel("vcc");
    _vcc->setUnit("mV");
    pipe->push(drawLabelForwarder, _vcc);

    _rvcc = new Stat(120, 50, 170, 50, 120, 195);
    _rvcc->setChartWidth(130);
    _rvcc->setLabel("rvcc");
    _rvcc->setUnit("mV");
    pipe->push(drawLabelForwarder, _rvcc);

    _vib = new Stat(10, 70, 50, 70, 10, 195);
    _vib->setLabel("vib");
    _vib->setColor(ILI9341_GREEN);
    _vib->setEnableChart(true);
    pipe->push(drawLabelForwarder, _vib);
    pipe->push(drawChartForwarder, _vib);

    _altitude = new Stat(120, 70, 170, 70, 10, 230);
    _altitude->setLabel("alt");
    _altitude->setUnit("m");
    _altitude->setColor(ILI9341_MAGENTA);
    _altitude->setEnableChart(true);
    pipe->push(drawLabelForwarder, _altitude);
}
