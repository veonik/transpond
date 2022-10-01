#ifndef TRANSPOND_GUI_H
#define TRANSPOND_GUI_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Wire.h>
#include <limits.h>

#include "Inconsolata_g5pt7b.h"
#include "Inconsolata_g8pt7b.h"

#define CURSOR_Y_SMALL 6
#define CURSOR_Y_LARGE 13

void drawControlForwarder(void *context);
void drawViewControllerForwarder(void *context);

// TODO: dont expose these
void drawValueForwarder(void* context);
void drawChartForwarder(void* context);
void drawLabelForwarder(void* context);

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
private:
    bool _inited = false;

protected:
    virtual void doInit() = 0;
    virtual void doDeInit() = 0;

public:
    virtual ~ViewController() = 0;

    virtual void tick() = 0;
    virtual void draw() = 0;

    void init() {
        if (_inited) {
            return;
        }

        doInit();
        _inited = true;
    }

    void deinit() {
        if (!_inited) {
            return;
        }

        doDeInit();
        _inited = false;
    }
};

class Pipeline {
private:
    static const byte SIZE = 128;

    tickCallback _callbacks[SIZE]{};
    void *_contexts[SIZE]{};

    byte _pop = 0;
    byte _push = 0;
    bool _draining = false;

    /**
     * The current ViewController.
     */
    ViewController *_viewController = nullptr;

    /**
     * The previous ViewController, populated only during "seguePopover" operations.
     */
    ViewController *_previousController = nullptr;

public:
    void segueTo(ViewController *nextController);
    void seguePopover(ViewController *popoverController);
    void segueBack();

    void push(tickCallback fn, void *context);

    void tick();

    void flush();
};

class Control {
protected:
    Point _pos{};
    Size _size{};

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
    tickCallback _onClick = nullptr;
    void *_onClickContext = nullptr;

protected:
    byte _touching = 0;

public:
    Clickable(Point pos, Size siz) : Control(pos, siz) {}

    void then(tickCallback cb) {
        then(cb, nullptr);
    }
    void then(tickCallback cb, void *context);

    void tick() override;
};

class Label : public Clickable {
private:
    char _label[16]{};

public:
    int fontSize = 1;
    uint16_t fontColor = ILI9341_WHITE;
    bool centerLabel = false;

    Label(Point pos, Size siz) : Clickable(pos, siz) { }

    void setLabel(const char *label);

    void tick() override;

    void draw() override;
};

class Button : public Label {
public:
    uint16_t bgColor = ILI9341_BLACK;
    uint16_t touchColor = ILI9341_DARKGREY;

    Button(Point pos, Size siz) : Label(pos, siz) {
        centerLabel = true;
    }

    void tick() override;

    void draw() override;
};

class Textbox : public Clickable {
private:
    char _valueSuffixText[8]{};

    size_t _lastValLength = 0;
    char _lastVal[16]{};

    bool _fillBackground = true;

    char _value[16]{};

public:
    int fontSize = 2;
    uint16_t bgColor = ILI9341_BLACK;
    uint16_t fontColor = ILI9341_WHITE;
    uint16_t touchColor = ILI9341_DARKGREY;

    Textbox(Point pos, Size siz) : Clickable(pos, siz) {}

    void setValueSuffix(const char *suffix);

    void setValue(const char *val);
    void setValue(long val);
    void setValue(double val, unsigned char precision = 2);

    void invalidate();

    const char *getValue();

    void tick() override;

    void draw() override;
};



class Stat : public Control {
private:
    Point _value{};
    Point _chart{};

    int _chartWidth = 240;
    int _controlWidth = 60;
    const char *_labelText = "";
    const char *_unitText = "";

    int _lastValDrawn{};
    size_t _lastValLength = 0;
    char _lastVal[8]{};

    int _stat{};
    int _historical[160]{};
    bool _enableChart = false;
    bool _hideLabel = false;
    bool _redrawChart = true;
    int _min = INT_MAX;
    int _max = INT_MIN;
    short _cur = 0;
    short _end = 0;
    uint16_t _chartColor = ILI9341_RED;


public:
    Stat(short labelX, short labelY, short valueX, short valueY, short chartX, short chartY) :
            Control(Point{x: labelX, y: labelY}, Size{w: 0, h: 0}) {
        _value = Point{x: valueX, y: valueY};
        _chart = Point{x: chartX, y: chartY};
    }

    ~Stat() = default;

    void setEnableChart(bool enable);

    void setChartWidth(int width);

    void setLabel(const char *label);

    void setHideLabel(bool hide);

    void setUnit(const char *unit);

    void setColor(int color);

    void set(float stat);

    void set(int stat);

    void tick();

    void draw();

    void drawLabel();

    void drawValue();

    void drawChart();
};

#endif