#include "position.h"
#include "dashboard.h"
#include "message.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics m;

void PositionViewController::tick() {
    _btnCenter->tick();
    _btnExit->tick();

    unsigned long update = millis();
    if (update >= _lastUpdate + UPDATE_WAIT) {
        _lastUpdate = update;
        pipe->push(drawViewControllerForwarder, this);
    }
}

void PositionViewController::draw() {
    pipe->push([](void *context) {
        static_cast<PositionViewController*>(context)->drawPRH();
    }, this);
    pipe->push([](void *context) {
        static_cast<PositionViewController*>(context)->drawPRH2();
    }, this);
    pipe->push([](void *context) {
        static_cast<PositionViewController*>(context)->drawPos();
    }, this);
}

void PositionViewController::drawPRH() {
    _accel_event.acceleration.x = m.accelX;
    _accel_event.acceleration.y = m.accelY;
    _accel_event.acceleration.z = m.accelZ;
    _mag_event.magnetic.x = m.magX;
    _mag_event.magnetic.y = m.magY;
    _mag_event.magnetic.z = m.magZ;

    _dof.fusionGetOrientation(&_accel_event, &_mag_event, &_orientation);

    float x = _orientation.roll;
    float y = _orientation.pitch;
    float z = _orientation.heading;
    y -= 90;  // Sensor is upright
    if (x < 0) { x += 360; }
    if (y < 0) { y += 360; }
    if (z < 0) { z += 360; }

    _txtXValue->setValue(x);
    pipe->push(drawControlForwarder, _txtXValue);
    _txtYValue->setValue(y);
    pipe->push(drawControlForwarder, _txtYValue);
    _txtZValue->setValue(z);
    pipe->push(drawControlForwarder, _txtZValue);

    if (invalidReadingf(x) || invalidReadingf(y) || invalidReadingf(z)) {
        tft.setCursor(10, 300+CURSOR_Y_SMALL);
        tft.setFont(&Inconsolata_g5pt7b);
        tft.setTextColor(ILI9341_RED);
        tft.print("x");
    } else {
        tft.fillRect(10, 300, 20, 20, ILI9341_BLACK);
    }

    Point center = Point{x: 40, y: 75};
    int width = 30;
    if (validReadingf(x)) {
        tft.fillCircle(center.x, center.y, width-1, ILI9341_BLACK);
        tft.drawCircle(center.x, center.y, width, tft.color565(50, 50, 50));

        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(x))),
                     (int) center.y + ((int) (width - 1) * cos(radians(x))), ILI9341_WHITE);
        tft.drawLine((int) center.x, (int) center.y, (int) center.x - ((int) (width - 1) * sin(radians(x))),
                     (int) center.y - ((int) (width - 1) * cos(radians(x))), ILI9341_WHITE);
    }

    center.x = 120;
    if (validReadingf(y)) {
        tft.fillCircle(center.x, center.y, width-1, ILI9341_BLACK);
        tft.drawCircle(center.x, center.y, width, tft.color565(50, 50, 50));

        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(y))),
                     (int) center.y + ((int) (width - 1) * cos(radians(y))), ILI9341_WHITE);
        tft.drawLine((int) center.x, (int) center.y, (int) center.x - ((int) (width - 1) * sin(radians(y))),
                     (int) center.y - ((int) (width - 1) * cos(radians(y))), ILI9341_WHITE);

    }

    center.x = 200;
    if (validReadingf(z)) {
        tft.fillCircle(center.x, center.y, width-1, ILI9341_BLACK);
        tft.drawCircle(center.x, center.y, width, tft.color565(50, 50, 50));

        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(z))),
                     (int) center.y + ((int) (width - 1) * cos(radians(z))), ILI9341_WHITE);

        tft.setCursor(center.x - 2, center.y - 20);
        tft.setTextColor(tft.color565(50, 50, 50));
        tft.setFont(&Inconsolata_g5pt7b);
        tft.print('N');
    }
}

void PositionViewController::drawPRH2() {
    _accel_event.acceleration.x = m.accel2X;
    _accel_event.acceleration.y = m.accel2Y;
    _accel_event.acceleration.z = m.accel2Z;
    _mag_event.magnetic.x = m.mag2X;
    _mag_event.magnetic.y = m.mag2Y;
    _mag_event.magnetic.z = m.mag2Z;

    _dof.fusionGetOrientation(&_accel_event, &_mag_event, &_orientation);

    float x = _orientation.roll;
    float y = _orientation.pitch;
    float z = _orientation.heading;
    y -= 90; // Sensor is upright
    if (x < 0) { x += 360; }
    if (y < 0) { y += 360; }
    if (z < 0) { z += 360; }

    _txtX2Value->setValue(x);
    pipe->push(drawControlForwarder, _txtX2Value);
    _txtY2Value->setValue(y);
    pipe->push(drawControlForwarder, _txtY2Value);
    _txtZ2Value->setValue(z);
    pipe->push(drawControlForwarder, _txtZ2Value);

    Point center = Point{x: 40, y: 75};
    int width = 30;
    if (validReadingf(x)) {
        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(x))),
                     (int) center.y + ((int) (width - 1) * cos(radians(x))), tft.color565(180, 180, 180));
        tft.drawLine((int) center.x, (int) center.y, (int) center.x - ((int) (width - 1) * sin(radians(x))),
                     (int) center.y - ((int) (width - 1) * cos(radians(x))), tft.color565(180, 180, 180));
    }

    center.x = 120;
    if (validReadingf(y)) {
        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(y))),
                     (int) center.y + ((int) (width - 1) * cos(radians(y))), tft.color565(180, 180, 180));
        tft.drawLine((int) center.x, (int) center.y, (int) center.x - ((int) (width - 1) * sin(radians(y))),
                     (int) center.y - ((int) (width - 1) * cos(radians(y))), tft.color565(180, 180, 180));

    }

    center.x = 200;
    if (validReadingf(z)) {
        tft.drawLine((int) center.x, (int) center.y, (int) center.x + ((int) (width - 1) * sin(radians(z))),
                     (int) center.y + ((int) (width - 1) * cos(radians(z))), tft.color565(45, 45, 45));
    }
}

void PositionViewController::drawPos() {
    int offsetLat = (int) m.latitude;
    int offsetLong = (int) m.longitude;
    long fixedLat = (long) (((m.latitude - offsetLat) * 100000000L) - _centerLat);
    long fixedLong = (long) (((m.longitude - offsetLong) * 100000000L) - _centerLong);
    Serial.print(fixedLat);
    Serial.print(",");
    Serial.println(fixedLong);
    int x = (int) round(fixedLong / 1000.0);
    int y = (int) round(fixedLat / 1000.0);
    if (abs(x) > 100 || abs(y) > 100) {
        return;
    }
    Serial.print("(");
    Serial.print(x);
    Serial.print(",");
    Serial.print(y);
    Serial.println(")");
    x += 120;
    y += 60;
    tft.drawCircle(x, y+150, 2, ILI9341_RED);
}

void PositionViewController::center() {
    int offsetLat = (int) m.latitude;
    int offsetLong = (int) m.longitude;
    _centerLat = (long) ((m.latitude - offsetLat) * 100000000L);
    _centerLong = (long) ((m.longitude - offsetLong) * 100000000L);
    Serial.print("center (");
    Serial.print(_centerLat);
    Serial.print(",");
    Serial.print(_centerLong);
    Serial.println(")");
    pipe->push([](void *context) {
        tft.fillRect(0, 200, 240, 120, ILI9341_BLACK);
    }, NULL);
    pipe->push(drawControlForwarder, _btnCenter);
    pipe->push(drawControlForwarder, _btnExit);
}

void PositionViewController::init() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawFastVLine(80, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastVLine(160, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastHLine(0, 110, 240, tft.color565(50, 50, 50));

    _btnCenter = new Button(Point{x: 150, y: 285}, Size{w: 50, h: 25});
    _btnCenter->bgColor = ILI9341_DARKGREY;
    _btnCenter->setLabel("CTR");
    _btnCenter->then([](void *context) {
        static_cast<PositionViewController*>(context)->center();
    }, this);
    pipe->push(drawControlForwarder, _btnCenter);

    _btnExit = new Button(Point{x: 205, y: 285}, Size{w: 25, h: 25});
    _btnExit->bgColor = ILI9341_DARKGREY;
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueBack();
    }, NULL);
    pipe->push(drawControlForwarder, _btnExit);

    _lblXValue = new Label(Point{x: 10, y: 10}, Size{w: 25, h: 13});
    _lblXValue->setLabel("x:");
    _lblXValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblXValue);

    _txtXValue = new Textbox(Point{x: 25, y: 10}, Size{w: 50, h: 13});
    _txtXValue->fontSize = 1;
    _txtXValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtXValue);

    _lblYValue = new Label(Point{x: 90, y: 10}, Size{w: 25, h: 13});
    _lblYValue->setLabel("y:");
    _lblYValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblYValue);

    _txtYValue = new Textbox(Point{x: 105, y: 10}, Size{w: 50, h: 13});
    _txtYValue->fontSize = 1;
    _txtYValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtYValue);

    _lblZValue = new Label(Point{x: 170, y: 10}, Size{w: 25, h: 13});
    _lblZValue->setLabel("z:");
    _lblZValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblZValue);

    _txtZValue = new Textbox(Point{x: 185, y: 10}, Size{w: 50, h: 13});
    _txtZValue->fontSize = 1;
    _txtZValue->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtZValue);

    _lblX2Value = new Label(Point{x: 10, y: 25}, Size{w: 25, h: 13});
    _lblX2Value->setLabel("x:");
    _lblX2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblX2Value);

    _txtX2Value = new Textbox(Point{x: 25, y: 25}, Size{w: 50, h: 13});
    _txtX2Value->fontSize = 1;
    _txtX2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtX2Value);

    _lblY2Value = new Label(Point{x: 90, y: 25}, Size{w: 25, h: 13});
    _lblY2Value->setLabel("y:");
    _lblY2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblY2Value);

    _txtY2Value = new Textbox(Point{x: 105, y: 25}, Size{w: 50, h: 13});
    _txtY2Value->fontSize = 1;
    _txtY2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtY2Value);

    _lblZ2Value = new Label(Point{x: 170, y: 25}, Size{w: 25, h: 13});
    _lblZ2Value->setLabel("z:");
    _lblZ2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _lblZ2Value);

    _txtZ2Value = new Textbox(Point{x: 185, y: 25}, Size{w: 50, h: 13});
    _txtZ2Value->fontSize = 1;
    _txtZ2Value->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtZ2Value);
}