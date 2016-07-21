#ifndef _GUI_H_
#define _GUI_H_

#include <SeeedTouchScreen.h>

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // mega
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3

#elif defined(__AVR_ATmega32U4__) // leonardo
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3

#else //168, 328, something else
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3

#endif

class Pipeline;
void drawButtonForwarder(void *context);
void clickButtonForwarder(void *context);

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;
TouchScreen screen = TouchScreen(XP, YP, XM, YM);

typedef void (*drawCallback)(void*);

class Pipeline {
private:
    static const byte SIZE = 64;

    drawCallback _callbacks[SIZE];
    void *_contexts[SIZE];

    short _pop = 0;
    short _push = 0;

public:
    void push(drawCallback fn, void *context) {
        _callbacks[_push] = fn;
        _contexts[_push] = context;
        _push++;
        if (_push == SIZE) {
            _push = 0; // wrap
#ifdef DEBUG
            if (_push == _pop) {
                Serial.println(F("overflowed Pipeline"));
            }
#endif
        }
    }

    void tick() {
        if (!_callbacks[_pop]) {
            return;
        }
        void *context = _contexts[_pop];
        _callbacks[_pop](context);
        _callbacks[_pop] = NULL;
        _contexts[_pop] = NULL;
        _pop++;
        if (_pop == SIZE) {
            _pop = 0; // wrap
        }
    }

    void flush() {
        while (_pop != _push) {
            tick();
        }
    }
};

struct Point {
    short x;
    short y;
};

struct Size {
    short w;
    short h;
};

class Button {
private:
    Point _pos;
    Size _size;
    const char *_label;

    byte _touching = 0;

public:
    Button(Point pos, Size siz) {
        _pos = pos;
        _size = siz;
    }

    void setLabel(const char *label) {
        _label = label;
    }

    void tick() {
        if (screen.isTouching()) {
            TouchPoint pt = screen.getPoint();
#ifdef DEBUG
            Serial.print(F("("));
            Serial.print(pt.x);
            Serial.print(F(","));
            Serial.print(pt.y);
            Serial.print(F(","));
            Serial.print(pt.z);
            Serial.println(F(")"));
#endif
            if (pt.x >= _pos.x && pt.x <= _pos.x+_size.w
                && pt.y >= _pos.y && pt.y <= _pos.y+_size.h) {
                _touching++;
                pipe->push(drawButtonForwarder, this);
                Serial.println("touching!");
            } else if (_touching > 0) {
                _touching = 0;
                pipe->push(clickButtonForwarder, this);
            }
        }
    }

    void draw() {
        int color = 0x2104;
        if (_touching) {
            color = ILI9341_LIGHTGREY;
        }
        tft.fillRect(_pos.x, _pos.y, _size.w, _size.h, color);
        tft.setCursor(_pos.x + 10, _pos.y + 10);
        tft.print(_label);
    }

    void click() {
        tft.fillScreen(0x2104);
        tft.setCursor(240-20, 20);
        tft.print("X");
    }
};

void drawButtonForwarder(void *context) {
    static_cast<Button*>(context)->draw();
}

void clickButtonForwarder(void *context) {
    static_cast<Button*>(context)->click();
}

#endif