#include "gui.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;
TouchScreen screen = TouchScreen(XP, YP, XM, YM);

void Pipeline::segueTo(ViewController *nextController) {
    if (_viewController) {
        flush(); // ensure we don't use any null pointers later on.
        _viewController->deinit();
        _lastViewController = _viewController;
    }
    _viewController = nextController;
    _viewController->init();
}

void Pipeline::segueBack() {
    if (_lastViewController) {
        segueTo(_lastViewController);
    }
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

void Label::draw() {}

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
        pipe->push(drawControlForwarder, this);

    } else if (_touching > 0) {
        _touching = 0;
        pipe->push(_onClick, _onClickContext);
    }
}

void Button::draw() {
    int color = 0x2104;
    if (_touching) {
        color = ILI9341_LIGHTGREY;
    }

    tft.fillRect(_pos.x, _pos.y, _size.w, _size.h, color);
    tft.setCursor(_pos.x + 10, _pos.y + 10);
    tft.setTextColor(ILI9341_WHITE, color);
    tft.print(_label);
}

void drawControlForwarder(void *context) {
    static_cast<Control*>(context)->draw();
}

void drawViewControllerForwarder(void *context) {
    static_cast<ViewController*>(context)->draw();
}
