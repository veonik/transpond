#ifndef _GUI_H_
#define _GUI_H_

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <SeeedTouchScreen.h>

#include "Inconsolata_g5pt7b.h"
#include "Inconsolata_g8pt7b.h"

#define CURSOR_Y_SMALL 6
#define CURSOR_Y_LARGE 13

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

void drawControlForwarder(void *context);
void drawViewControllerForwarder(void *context);

typedef void (*tickCallback)(void*);

struct Point {
    short x;
    short y;
};

struct Size {
    int w;
    int h;
};

class ViewController {
public:
    virtual void tick() = 0;
    virtual void draw() = 0;

    virtual void init() = 0;
    virtual void deinit() = 0;
};

class Pipeline {
private:
    static const byte SIZE = 64;

    tickCallback _callbacks[SIZE];
    void *_contexts[SIZE];

    short _pop = 0;
    short _push = 0;
    bool _draining = false;

    ViewController *_viewController;

public:
    void segueTo(ViewController *nextController);

    void push(tickCallback fn, void *context);

    void tick();

    void flush();
};

class Control {
protected:
    Point _pos;
    Size _size;

public:
    Control(Point pos, Size siz) {
        _pos = pos;
        _size = siz;
    }

    virtual ~Control() = 0;

    virtual void tick() = 0;
    virtual void draw() = 0;
};

class Clickable : public Control {
private:
    tickCallback _onClick;
    void *_onClickContext;

protected:
    byte _touching = 0;

public:
    Clickable(Point pos, Size siz) : Control(pos, siz) {}

    void then(tickCallback cb, void *context);

    void tick();

    virtual void draw() = 0;
};

class Label : public Clickable {
private:
    const char *_label;

public:
    int fontSize = 1;
    uint16_t fontColor = ILI9341_WHITE;
    bool centerLabel = false;

    Label(Point pos, Size siz) : Clickable(pos, siz) { }

    void setLabel(const char *label);

    void tick();

    void draw();
};

class Button : public Label {
public:
    uint16_t bgColor = ILI9341_BLACK;
    uint16_t touchColor = ILI9341_DARKGREY;

    Button(Point pos, Size siz) : Label(pos, siz) {
        centerLabel = true;
    }

    void tick();

    void draw();
};

#endif