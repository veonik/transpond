#include <EEPROM.h>
#include "position.h"
#include "message.h"

const unsigned int CENTER_ADDR = 0x80;

union {
    float f;
    byte b[4];
} val;

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics m;

float haversine(float lat1, float lng1, float lat2, float lng2) {
    float R = 6371e3; // metres
    float radLat1 = radians(lat1);
    float radLat2 = radians(lat2);
    float radDeltaLat = radians(lat2-lat1);
    float radDeltaLng = radians(lng2-lng1);

    float a = sin(radDeltaLat/2.0f) * sin(radDeltaLat/2.0f) +
            cos(radLat1) * cos(radLat2) *
            sin(radDeltaLng/2.0f) * sin(radDeltaLng/2.0f);
    float c = 2.0f * atan2(sqrt(a), sqrt(1.0f-a));

    float d = R * c;

    return d;
}

void PositionViewController::tick() {
    _btnCenter->tick();
    _btnExit->tick();
    _btnZoomIn->tick();
    _btnZoomOut->tick();
    _txtPositionAltMax->tick();

    unsigned long update = millis();
    if (update >= _lastUpdate + UPDATE_WAIT) {
        _lastUpdate = update;
        pipe->push(drawViewControllerForwarder, this);

        _txtPositionLat->setValue(m.latitude, 6);
        _txtPositionLong->setValue(m.longitude, 5);
        _txtPositionAlt->setValue(m.altitudeGps, 2);
        pipe->push(drawControlForwarder, _txtPositionLat);
        pipe->push(drawControlForwarder, _txtPositionLong);
        pipe->push(drawControlForwarder, _txtPositionAlt);
        if (m.altitudeGps > _altMax) {
            _altMax = m.altitudeGps;
            _txtPositionAltMax->setValue(_altMax, 2);
            pipe->push(drawControlForwarder, _txtPositionAltMax);
        }
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
        tft.setCursor(10, 140+CURSOR_Y_SMALL);
        tft.setFont(&Inconsolata_g5pt7b);
        tft.setTextColor(ILI9341_RED);
        tft.print("x");
    } else {
        tft.fillRect(10, 140, 20, 20, ILI9341_BLACK);
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
    _txtPositionDistance->setValue(haversine(_centerLat, _centerLong, m.latitude, m.longitude), 2);
    pipe->push(drawControlForwarder, _txtPositionDistance);
    
    float latToMeters = 111046.5974459675f;
    float longToMeters = 84612.89229305493f;
    float fixedLat = m.latitude - _centerLat;
    float fixedLong = m.longitude - _centerLong;

    float fx = fixedLong * longToMeters / _zoom;
    float fy = fixedLat * latToMeters / _zoom;
#ifdef DEBUGV
    Serial.print(fx, 8);
    Serial.print(",");
    Serial.println(fy, 8);
#endif
    int x = (int) round(fx);
    int y = (int) round(fy);
    if (abs(x) > 100 || abs(y) > 100) {
        return;
    }

    x += 120;
    y += 60;
    if (_lastPosX != x || _lastPosY != y) {
        tft.drawCircle(_lastPosX, _lastPosY+150, 2, ILI9341_MAROON);
        _lastPosX = x;
        _lastPosY = y;
        tft.drawCircle(_lastPosX, _lastPosY+150, 2, ILI9341_RED);
    }
}

void PositionViewController::center() {
    _centerLat = m.latitude;
    _centerLong = m.longitude;
    _lastPosX = 0;
    _lastPosY = 0;
#ifdef DEBUGV
    Serial.print("center (");
    Serial.print(_centerLat);
    Serial.print(",");
    Serial.print(_centerLong);
    Serial.println(")");
#endif
    pipe->push([](void *context) {
        tft.fillRect(0, 120, 240, 120, ILI9341_BLACK);
    }, NULL);
    _deferDrawPositionStats();
    val.f = _centerLat;
    EEPROM.write(CENTER_ADDR, val.b[0]);
    EEPROM.write(CENTER_ADDR+1, val.b[1]);
    EEPROM.write(CENTER_ADDR+2, val.b[2]);
    EEPROM.write(CENTER_ADDR+3, val.b[3]);
    val.f = _centerLong;
    EEPROM.write(CENTER_ADDR+4, val.b[0]);
    EEPROM.write(CENTER_ADDR+5, val.b[1]);
    EEPROM.write(CENTER_ADDR+6, val.b[2]);
    EEPROM.write(CENTER_ADDR+7, val.b[3]);
}

void PositionViewController::zoomOut() {
    if (_zoom < 1000000000L) {
        _zoom *= 10;
    }
    _txtZoomLevel->setValue(10L - (long) log10(_zoom));
    tft.fillRect(0, 120, 240, 120, ILI9341_BLACK);
    tft.drawCircle(_lastPosX, _lastPosY+150, 2, ILI9341_MAROON);
    _deferDrawPositionStats();
}

void PositionViewController::zoomIn() {
    if (_zoom > 10) {
        _zoom /= 10;
    }
    _txtZoomLevel->setValue(10L - (long) log10(_zoom));
    tft.fillRect(0, 120, 240, 120, ILI9341_BLACK);
    tft.drawCircle(_lastPosX, _lastPosY+150, 2, ILI9341_MAROON);
    _deferDrawPositionStats();
}


void PositionViewController::init() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawFastVLine(80, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastVLine(160, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastHLine(0, 110, 240, tft.color565(50, 50, 50));

    uint16_t buttonBg = tft.color565(25, 25, 25);

    _btnZoomIn = new Button(Point{x: 15, y: 285}, Size{w: 30, h: 25});
    _btnZoomIn->bgColor = buttonBg;
    _btnZoomIn->setLabel("+");
    _btnZoomIn->then([](void *context) {
        static_cast<PositionViewController*>(context)->zoomIn();
    }, this);
    pipe->push(drawControlForwarder, _btnZoomIn);

    _btnZoomOut = new Button(Point{x: 75, y: 285}, Size{w: 30, h: 25});
    _btnZoomOut->bgColor = buttonBg;
    _btnZoomOut->setLabel("-");
    _btnZoomOut->then([](void *context) {
        static_cast<PositionViewController*>(context)->zoomOut();
    }, this);
    pipe->push(drawControlForwarder, _btnZoomOut);

    _txtZoomLevel = new Textbox(Point{x: 55, y: 279}, Size{w: 15, h: 25});
    _txtZoomLevel->fontSize = 2;
    _txtZoomLevel->fontColor = ILI9341_WHITE;
    _txtZoomLevel->setValue(10L - (long) log10(_zoom));
    pipe->push(drawControlForwarder, _txtZoomLevel);

    _btnCenter = new Button(Point{x: 150, y: 285}, Size{w: 50, h: 25});
    _btnCenter->bgColor = buttonBg;
    _btnCenter->setLabel("CTR");
    _btnCenter->then([](void *context) {
        static_cast<PositionViewController*>(context)->center();
    }, this);
    pipe->push(drawControlForwarder, _btnCenter);

    _btnExit = new Button(Point{x: 205, y: 285}, Size{w: 25, h: 25});
    _btnExit->bgColor = buttonBg;
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

    _txtPositionLat = new Textbox(Point{x: 10, y: 120}, Size{w: 80, h: 13});
    _txtPositionLat->fontSize = 1;
    _txtPositionLat->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtPositionLat);

    _txtPositionLong = new Textbox(Point{x: 90, y: 120}, Size{w: 80, h: 13});
    _txtPositionLong->fontSize = 1;
    _txtPositionLong->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtPositionLong);

    _txtPositionAlt = new Textbox(Point{x: 170, y: 120}, Size{w: 80, h: 13});
    _txtPositionAlt->fontSize = 1;
    _txtPositionAlt->fontColor = ILI9341_WHITE;
    pipe->push(drawControlForwarder, _txtPositionAlt);

    _txtPositionAltMax = new Textbox(Point{x: 170, y: 140}, Size{w: 80, h: 13});
    _txtPositionAltMax->fontSize = 1;
    _txtPositionAltMax->fontColor = ILI9341_GREEN;
    _txtPositionAltMax->then([](void *context) {
        static_cast<PositionViewController*>(context)->resetAltMax();
    }, this);
    pipe->push(drawControlForwarder, _txtPositionAltMax);

    _txtPositionDistance = new Textbox(Point{x: 90, y: 140}, Size{w: 80, h: 13});
    _txtPositionDistance->fontSize = 1;
    _txtPositionDistance->fontColor = ILI9341_WHITE;
    _txtPositionDistance->setValueSuffix("m");
    pipe->push(drawControlForwarder, _txtPositionDistance);

    val.b[0] = EEPROM.read(CENTER_ADDR);
    val.b[1] = EEPROM.read(CENTER_ADDR+1);
    val.b[2] = EEPROM.read(CENTER_ADDR+2);
    val.b[3] = EEPROM.read(CENTER_ADDR+3);
    _centerLat = val.f;
    val.b[0] = EEPROM.read(CENTER_ADDR+4);
    val.b[1] = EEPROM.read(CENTER_ADDR+5);
    val.b[2] = EEPROM.read(CENTER_ADDR+6);
    val.b[3] = EEPROM.read(CENTER_ADDR+7);
    _centerLong = val.f;
}

void PositionViewController::_deferDrawPositionStats() {
    _txtPositionLat->invalidate();
    _txtPositionLong->invalidate();
    _txtPositionAlt->invalidate();
    _txtPositionAltMax->invalidate();
    _txtPositionDistance->invalidate();
    pipe->push(drawControlForwarder, _btnCenter);
    pipe->push(drawControlForwarder, _btnExit);
    pipe->push(drawControlForwarder, _btnZoomIn);
    pipe->push(drawControlForwarder, _btnZoomOut);
    pipe->push(drawControlForwarder, _txtZoomLevel);
    pipe->push(drawControlForwarder, _txtPositionLat);
    pipe->push(drawControlForwarder, _txtPositionLong);
    pipe->push(drawControlForwarder, _txtPositionAlt);
    pipe->push(drawControlForwarder, _txtPositionAltMax);
    pipe->push(drawControlForwarder, _txtPositionDistance);
}
