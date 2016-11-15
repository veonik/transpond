#include "common/message.h"
#include "gui.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;
TouchScreen screen = TouchScreen(XP, YP, XM, YM);

void Pipeline::segueTo(ViewController *nextController) {
    flush();
    if (_previousController != NULL) {
        delete _previousController;
        _previousController = NULL;
    }
    if (_viewController != NULL) {
        delete _viewController;
    }
    _viewController = nextController;
    _viewController->init();
}

void Pipeline::seguePopover(ViewController *popoverController) {
    flush();
    if (_previousController == NULL) {
        _previousController = _viewController;
        _previousController->deinit();
    }
    _viewController = popoverController;
    _viewController->init();
}

void Pipeline::segueBack() {
    if (_previousController == NULL) {
        // TODO: Inconsistent api
#ifdef DEBUGV
        Serial.println(F("segue back called, no previous controller"));
#endif
        return;
    }
    flush();
    if (_viewController != NULL) {
        delete _viewController;
    }
    _viewController = _previousController;
    _previousController = NULL;
    _viewController->init();
}

void Pipeline::push(tickCallback fn, void *context) {
    if (_draining) {
#ifdef DEBUGV
        Serial.println(F("ignoring push, draining"));
#endif
        return;
    }
    _callbacks[_push] = fn;
    _contexts[_push] = context;
    _push++;
    if (_push == SIZE) {
        _push = 0; // wrap
    }
#ifdef DEBUG
    if (_push == _pop) {
        Serial.println(F("overflowed Pipeline"));
    }
#endif
}

void Pipeline::tick() {
    if (_viewController) {
        _viewController->tick();
    }
    if (!_callbacks[_pop]) {
        return;
    }
    int last = _pop;
    _pop++;
    if (_pop == SIZE) {
        _pop = 0; // wrap
    }

    void *context = _contexts[last];
    _callbacks[last](context);
    _callbacks[last] = NULL;
    _contexts[last] = NULL;
}

void Pipeline::flush() {
    _draining = true;
    while (_pop != _push) {
        tick();
    }
    _draining = false;
}

ViewController::~ViewController() {}

Control::~Control() {}

void Clickable::tick() {
    if (screen.isTouching(
        _pos.x,
        _pos.y,
        _pos.x+_size.w,
        _pos.y+_size.h
    )) {
        _touching++;
        if (_touching == 1) {
            pipe->push(drawControlForwarder, this);
        }
    } else if (_touching > 0) {
        _touching = 0;
        if (_onClick != NULL) {
            pipe->push(_onClick, _onClickContext);
        }
        pipe->push(drawControlForwarder, this);
    }
}

void Clickable::then(tickCallback cb, void *context) {
    _onClick = cb;
    _onClickContext = context;
}

void Label::tick() {
    Clickable::tick();
}

void Label::draw() {
    int ox = _pos.x;
    int oy = _pos.y;
    if (_touching) {
        ox++;
        oy++;
    }
    tft.setTextColor(fontColor);
    int offset = CURSOR_Y_SMALL;
    if (fontSize == 1) {
        tft.setFont(&Inconsolata_g5pt7b);
    } else {
        tft.setFont(&Inconsolata_g8pt7b);
        offset = CURSOR_Y_LARGE;
    }
    if (centerLabel) {
        uint16_t w, h;
        int16_t x1, y1;
        tft.getTextBounds(_label, 10, 10, &x1, &y1, &w, &h);
        int x = (int) ox + (_size.w / 2) - ((int) w / 2) - 1;
        int y = (int) oy + (_size.h / 2) - ((int) h / 2);
        tft.setCursor(x, y + offset);
        tft.print(_label);
    } else {
        tft.setCursor(ox, oy+_size.h-2);
        tft.print(_label);
    }
}

void Label::setLabel(const char *label) {
    strcpy(_label, label);
}

void Button::tick() {
    Label::tick();
}

void Button::draw() {
    if (_touching) {
        tft.fillRect(_pos.x, _pos.y, _size.w, _size.h, touchColor);
    } else {
        tft.fillRect(_pos.x-1, _pos.y-1, _size.w, _size.h, bgColor);
        // TODO: Better drop shadow
        tft.drawFastVLine(_pos.x-1+_size.w, _pos.y-1, _size.h, ILI9341_BLACK);
        tft.drawFastHLine(_pos.x-1, _pos.y-1+_size.h, _size.w, ILI9341_BLACK);
    }

    Label::draw();
}

void Textbox::setValueSuffix(const char *suffix) {
    strcpy(_valueSuffixText, suffix);
}

void Textbox::setValue(const char *val) {
    strcpy(_value, val);
}

void Textbox::setValue(long val) {
    ltoa(val, _value, 10);
}

void Textbox::setValue(double val, int precision) {
    String str = String(val, precision);
    strcpy(_value, str.c_str());
}

const char *Textbox::getValue() {
    return _value;
}

void Textbox::tick() {
    Clickable::tick();
}

void Textbox::draw() {
    if (_fillBackground) {
        tft.fillRect(_pos.x, _pos.y, _size.w, _size.h, bgColor);
        _fillBackground = false;
    }
    if (fontSize == 1) {
        tft.setFont(&Inconsolata_g5pt7b);
    } else {
        tft.setFont(&Inconsolata_g8pt7b);
    }

    const char *valStr = _value;
    int valLength = strlen(valStr);
    if (_lastValLength > 0) {
        if (strcmp(_lastVal, valStr) == 0) {
            return;
        }

        tft.setCursor(_pos.x, _pos.y +_size.h-2);
        tft.setTextColor(bgColor);
        tft.print(_lastVal);
    }

    tft.setCursor(_pos.x, _pos.y +_size.h-2);
    tft.setTextColor(fontColor);
    tft.print(valStr);
    tft.setFont(&Inconsolata_g5pt7b);
    strcpy(_lastVal, valStr);
    _lastValLength = valLength;
}

void Textbox::invalidate() {
    memset(_lastVal, 0, sizeof(_lastVal));
    _lastValLength = 0;
}


void Stat::setEnableChart(bool enable) {
    _enableChart = enable;
}

void Stat::setChartWidth(int width) {
    _chartWidth = width;
}

void Stat::setLabel(const char *label) {
    _labelText = label;
}

void Stat::setHideLabel(bool hide) {
    _hideLabel = hide;
    _controlWidth = !_hideLabel ? 60 : 100;
}

void Stat::setUnit(const char *unit) {
    _unitText = unit;
}

void Stat::setColor(int color) {
    _chartColor = (uint16_t) color;
}

void Stat::set(float stat) {
    if (invalidReadingf(stat)) {
        return set(NO_READING_INT);
    }
    set((int) stat);
}

void Stat::set(int stat) {
    _stat = stat;
    _end++;
    if (_end >= _chartWidth-80) {
        _end = 0;
        _redrawChart = true;
    }
    if (validReadingi(stat)) {
        if (stat < _min) {
            _min = stat - 1;
            _redrawChart = true;
        } else if (stat > _max) {
            _max = stat + 1;
            _redrawChart = true;
        }
    }
    _historical[_end] = stat;
}

void Stat::tick() {
    pipe->push(drawValueForwarder, this);
    if (_enableChart) {
        pipe->push(drawChartForwarder, this);
    }
}

void Stat::draw() {
    // no op, see tick
}

void Stat::drawLabel() {
    if (!_hideLabel) {
        tft.setFont(&Inconsolata_g8pt7b);
        tft.setCursor(_pos.x, _pos.y+1+CURSOR_Y_LARGE);
        tft.setTextColor(ILI9341_WHITE);
        tft.print(_labelText);
    }
    tft.fillRect(_value.x, _value.y, _controlWidth, 17, tft.color565(10, 10, 10));
}

void Stat::drawValue() {
    char valStr[8];
    int valLength;
    if (validReadingi(_stat)) {
        valLength = sprintf(valStr, "%d", _stat);
    } else {
        valLength = sprintf(valStr, "-- ");
    }
    if (_lastValLength > 0) {
        if (_lastValDrawn == _stat) {
            return;
        }

        tft.setCursor(_value.x, _value.y + 1 + CURSOR_Y_LARGE);
        tft.setFont(&Inconsolata_g8pt7b);
        tft.setTextColor(tft.color565(10, 10, 10));
        tft.print(_lastVal);


        if (valLength != _lastValLength) {
            tft.setFont(&Inconsolata_g5pt7b);
            tft.print(_unitText);
        }
    }

    tft.setCursor(_value.x, _value.y+1+CURSOR_Y_LARGE);
    tft.setFont(&Inconsolata_g8pt7b);
    tft.setTextColor(ILI9341_WHITE, tft.color565(10, 10, 10));
    tft.print(valStr);
    tft.setFont(&Inconsolata_g5pt7b);
    if (valLength != _lastValLength) {
        tft.setFont(&Inconsolata_g5pt7b);
        tft.print(_unitText);
    }
    memcpy(_lastVal, valStr, valLength+1);
    _lastValDrawn = _stat;
    _lastValLength = valLength;
}

void Stat::drawChart() {
    if (_redrawChart) {
        _cur = 0;
        tft.fillRect(_chart.x, _chart.y - 1 /* catch the top of the unit of the axis */,
                     _chartWidth, 32 /* to catch the bottom of the unit on the axis */,
                     ILI9341_BLACK);
        tft.setCursor(_chart.x + 27 - ((int16_t) strlen(_labelText) * 6), _chart.y + 12+CURSOR_Y_SMALL);
        tft.setTextColor(ILI9341_WHITE);
        tft.setFont(&Inconsolata_g5pt7b);
        tft.print(_labelText);
        tft.setCursor(_chart.x + _chartWidth - 45, _chart.y+CURSOR_Y_SMALL);
        tft.print(_max == INT_MIN ? 0 : _max);

        if (_unitText) {
            tft.setTextColor(_chartColor);
            tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 12+CURSOR_Y_SMALL);
            tft.print(_unitText);
            tft.setTextColor(ILI9341_WHITE);
        }
        tft.setCursor(_chart.x + _chartWidth - 45, _chart.y + 24+CURSOR_Y_SMALL);
        tft.print(_min == INT_MAX ? 0 : _min);
        tft.setFont(&Inconsolata_g8pt7b);
        _redrawChart = false;
        tft.fillRect(_chart.x + 30, _chart.y, _chartWidth - 80, 30, tft.color565(10, 10, 10));
    }

    for ( ; _cur < _end; _cur++) {
        if (invalidReadingi(_historical[_cur])) {
            tft.drawFastVLine(_chart.x+30+_cur, _chart.y, 30, ILI9341_MAROON);
            continue;
        }
        int norm = (int) map(_historical[_cur], _min, _max, 0, 29);
        int x = _chart.x+30+_cur;
        int y = _chart.y+29-norm;
        tft.fillRect(x, y, 1, 1, _chartColor);
    }
}

void drawControlForwarder(void *context) {
    static_cast<Control*>(context)->draw();
}

void drawLabelForwarder(void* context) {
    static_cast<Stat*>(context)->drawLabel();
}

void drawValueForwarder(void* context) {
    static_cast<Stat*>(context)->drawValue();
}
void drawChartForwarder(void* context) {
    static_cast<Stat*>(context)->drawChart();
}

void drawViewControllerForwarder(void *context) {
    static_cast<ViewController*>(context)->draw();
}