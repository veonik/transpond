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
        int x = (int) ox + (_size.w / 2) - ((int) w / 2);
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

//        if (valLength != _lastValLength && _valueSuffixText[0] != 0) {
//            tft.setFont(&Inconsolata_g5pt7b);
//            tft.print(_valueSuffixText);
//        }
    }

    tft.setCursor(_pos.x, _pos.y +_size.h-2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(valStr);
    tft.setFont(&Inconsolata_g5pt7b);
//    if (valLength != _lastValLength) {
//        tft.setFont(&Inconsolata_g5pt7b);
//        tft.print(_valueSuffixText);
//    }
    strcpy(_lastVal, valStr);
    _lastValLength = valLength;
}

void drawControlForwarder(void *context) {
    static_cast<Control*>(context)->draw();
}

void drawViewControllerForwarder(void *context) {
    static_cast<ViewController*>(context)->draw();
}