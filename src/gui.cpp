#include "gui.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;
TouchScreen screen = TouchScreen(XP, YP, XM, YM);

void Pipeline::segueTo(ViewController *nextController) {
    if (_viewController) {
        flush(); // ensure we don't use any null pointers later on.
        _viewController->deinit();
        delete _viewController;
    }
    _viewController = nextController;
    _viewController->init();
}


void Pipeline::push(tickCallback fn, void *context) {
    if (_draining) {
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

void Label::draw() {

}

void Label::setLabel(const char *label) {
    _label = label;
}

void Button::setLabel(const char *label) {
    _label = label;
}

void Button::then(tickCallback cb, void *context) {
    _onClick = cb;
    _onClickContext = context;
}

void Button::tick() {
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
        pipe->push(_onClick, _onClickContext);
    }
}

void Button::draw() {
    int ox = _pos.x;
    int oy = _pos.y;
    uint16_t fontBgColor = bgColor;
    if (_touching) {
        fontBgColor = touchColor;
        tft.fillRect(_pos.x, _pos.y, _size.w, _size.h, touchColor);
    } else {
        ox--;
        oy--;
        tft.fillRect(_pos.x-1, _pos.y-1, _size.w, _size.h, bgColor);
        tft.drawFastVLine(_pos.x-1+_size.w, _pos.y-1, _size.h, ILI9341_BLACK);
        tft.drawFastHLine(_pos.x-1, _pos.y-1+_size.h, _size.w, ILI9341_BLACK);
    }

    tft.setTextColor(fontColor, fontBgColor);
    int offset = CURSOR_Y_SMALL;
    if (fontSize == 1) {
        tft.setFont(&Inconsolata_g5pt7b);
    } else {
        tft.setFont(&Inconsolata_g8pt7b);
        offset = CURSOR_Y_LARGE;
    }
    uint16_t w, h;
    tft.getTextBounds((char *)_label, 10, 10, 0, 0, &w, &h);
    int x = (int) ox + (_size.w / 2) - ((int) w / 2);
    int y = (int) oy + (_size.h / 2) - ((int) h / 2);
    tft.setCursor(x, y+offset);
    tft.print(_label);
}

void drawControlForwarder(void *context) {
    static_cast<Control*>(context)->draw();
}

void drawViewControllerForwarder(void *context) {
    static_cast<ViewController*>(context)->draw();
}
