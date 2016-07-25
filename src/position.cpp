#include "position.h"
#include "dashboard.h"
#include "message.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics remote;

void PositionViewController::tick() {
    _btnExit->tick();
    _btnCalibrate->tick();

    unsigned long update = millis();
    if (update >= _lastUpdate + UPDATE_WAIT) {
        Serial.println("updating");
        _lastUpdate = update;
        pipe->push(drawViewControllerForwarder, this);
    }
}

void PositionViewController::draw() {
    if (!_calibrated) {
        return;
    }

    float x = remote.lastRoll;
    float y = remote.lastPitch;
    float z = remote.lastHeading;

    Point center = Point{x: 55, y: 50};
    int width = 40;

    tft.fillCircle(center.x, center.y, width, ILI9341_BLACK);
    tft.drawCircle(center.x, center.y, width, tft.color565(50, 50, 50));

    tft.drawLine((int) center.x, (int) center.y, (int) center.x +((int) width * sin(radians(x))), (int) center.y + ((int) width*cos(radians(x))), ILI9341_WHITE);
    tft.drawLine((int) center.x, (int) center.y, (int) center.x -((int) width * sin(radians(x))), (int) center.y - ((int) width*cos(radians(x))), ILI9341_WHITE);

    center.x += 125;
    tft.fillCircle(center.x, center.y, width, ILI9341_BLACK);
    tft.drawCircle(center.x, center.y, width, tft.color565(50, 50, 50));

    tft.drawLine((int) center.x, (int) center.y, (int) center.x +((int) width * sin(radians(z))), (int) center.y + ((int) width*cos(radians(z))), ILI9341_WHITE);
    tft.drawLine((int) center.x, (int) center.y, (int) center.x -((int) width * sin(radians(z))), (int) center.y - ((int) width*cos(radians(z))), ILI9341_WHITE);

    Serial.print("(");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.print(z);
    Serial.println(")");


}

void PositionViewController::calibrate() {
    if (isnan(remote.lastHeading)) {
        pipe->push([](void *context) {
            static_cast<PositionViewController*>(context)->calibrate();
        }, this);

        return;
    }

    _calibration++;
    Serial.print("Calibration measure #");
    Serial.println(_calibration);

    float x = remote.lastRoll;
    float y = remote.lastPitch;
    float z = remote.lastHeading;

    Serial.print("(");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.print(z);
    Serial.println(")");

    _calibrationX = _calibrationX + ((x - _calibrationX) / _calibration);
    _calibrationY = _calibrationY + ((y - _calibrationY) / _calibration);
    _calibrationZ = _calibrationZ + ((z - _calibrationZ) / _calibration);

    if (_calibration > 100) {
        _calibration = 0;
        _zeroX = _calibrationX;
        _zeroY = _calibrationY;
        _zeroZ = _calibrationZ;
        _calibrated = true;

        Serial.println("Calibrated");
        Serial.print("(");
        Serial.print(_zeroX);
        Serial.print(", ");
        Serial.print(_zeroY);
        Serial.print(", ");
        Serial.print(_zeroZ);
        Serial.println(")");
        return;
    }

    pipe->push([](void *context) {
        static_cast<PositionViewController*>(context)->calibrate();
    }, this);
}

void PositionViewController::init() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawFastVLine(120, 0, 100, tft.color565(50, 50, 50));
    tft.drawFastHLine(0, 100, 240, tft.color565(50, 50, 50));

    _btnExit = new Button(Point{x: 205, y: 10}, Size{w: 25, h: 25});
    _btnExit->bgColor = ILI9341_DARKGREY;
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueTo(new DashboardViewController());
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _btnCalibrate = new Button(Point{x: 190, y: 280}, Size{w: 40, h: 30});
    _btnCalibrate->bgColor = ILI9341_DARKGREY;
    _btnCalibrate->setLabel("CAL");
    _btnCalibrate->then([](void *context) {
        PositionViewController *vc = static_cast<PositionViewController*>(context);
        vc->_calibration = 0;
        vc->calibrate();
    }, this);
    pipe->push(drawControlForwarder, _btnCalibrate);
}