#include "position.h"
#include "dashboard.h"
#include "message.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

extern metrics remote;

void PositionViewController::tick() {
    _btnExit->tick();

    unsigned long update = millis();
    if (update >= _lastUpdate + UPDATE_WAIT) {
        _lastUpdate = update;
        pipe->push(drawViewControllerForwarder, this);
    }
}

void PositionViewController::draw() {
    float x = remote.lastRoll;
    float y = remote.lastPitch;
    float z = remote.lastHeading;
    // TODO: These adjustments are ill-understood, but make the measurements
    // sensical for my setup. Probably going to need to be able to adjust
    // these at runtime.
    // TODO: Should these adjustments happen on the transponder?
    x -= 90;  // Sensor is upright
    z -= 100; // 10 for Utah declination, 90 just because?
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

    Point center = Point{x: 40, y: 65};
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

#ifdef DEBUGV
    Serial.print("(");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.print(z);
    Serial.println(")");
#endif
}

void PositionViewController::init() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawFastVLine(80, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastVLine(160, 0, 110, tft.color565(50, 50, 50));
    tft.drawFastHLine(0, 110, 240, tft.color565(50, 50, 50));

    _btnExit = new Button(Point{x: 205, y: 285}, Size{w: 25, h: 25});
    _btnExit->bgColor = ILI9341_DARKGREY;
    _btnExit->setLabel("x");
    _btnExit->then([](void *context) {
        pipe->segueTo(new DashboardViewController());
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
}