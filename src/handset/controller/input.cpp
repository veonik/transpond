#include "input.h"

extern Adafruit_ILI9341 tft;
extern Pipeline *pipe;

void InputViewController::tick() {
    _textbox->tick();
    _button0->tick();
    _button1->tick();
    _button2->tick();
    _button3->tick();
    _button4->tick();
    _button5->tick();
    _button6->tick();
    _button7->tick();
    _button8->tick();
    _button9->tick();
    _buttonBackspace->tick();
    _buttonDone->tick();
}

void InputViewController::draw() {
    tft.fillScreen(0x2104);
}

void InputViewController::write(char c) {
    const char *val = _textbox->getValue();
    char buf[16];
    int len = strlen(val);
    if (len > 0) {
        strncpy(buf, val, 16);
    }
    buf[len] = c;
    buf[len+1] = 0;
    _textbox->setValue(buf);
    pipe->push(drawControlForwarder, _textbox);
}

void InputViewController::backspace() {
    const char *val = _textbox->getValue();
    char buf[16];
    int len = strlen(val);
    if (len > 0) {
        strncpy(buf, val, 16);
        buf[len - 1] = 0;
        _textbox->setValue(buf);
    }
    pipe->push(drawControlForwarder, _textbox);
}

const char *InputViewController::getValue() {
    if (_textbox) {
        return _textbox->getValue();
    }
    return (const char*)_value;
}

void InputViewController::clear() {
    _textbox->setValue("");
    pipe->push(drawControlForwarder, _textbox);
}

void InputViewController::doInit() {
    pipe->push(drawViewControllerForwarder, this);

    short cols[3] = {0,80,160};
    short rows[4] = {40,110,180,250};
    Size size = Size{w: 70, h: 60};
    int padding = 5;
    uint16_t bgColor = tft.color565(64, 64, 64);
    int fontSize = 2;

    _textbox = new Textbox(Point{x: 10, y: 10}, Size{w: 220, h: 25});
    _textbox->bgColor = ILI9341_DARKGREY;
    _textbox->fontSize = 2;
    _textbox->setValue((const char *)_value);

    // US 10-key order from physical top to bottom.
    _button7 = new Button(Point{x: cols[0] + padding, y: rows[0] + padding}, size);
    _button7->then([](void *context) {
        static_cast<InputViewController*>(context)->write('7');
    }, this);
    _button7->setLabel("7");
    _button7->fontSize = fontSize;
    _button7->bgColor = bgColor;

    _button8 = new Button(Point{x: cols[1] + padding, y: rows[0] + padding}, size);
    _button8->then([](void *context) {
        static_cast<InputViewController*>(context)->write('8');
    }, this);
    _button8->setLabel("8");
    _button8->fontSize = fontSize;
    _button8->bgColor = bgColor;

    _button9 = new Button(Point{x: cols[2] + padding, y: rows[0] + padding}, size);
    _button9->then([](void *context) {
        static_cast<InputViewController*>(context)->write('9');
    }, this);
    _button9->setLabel("9");
    _button9->fontSize = fontSize;
    _button9->bgColor = bgColor;

    _button4 = new Button(Point{x: cols[0] + padding, y: rows[1] + padding}, size);
    _button4->then([](void *context) {
        static_cast<InputViewController*>(context)->write('4');
    }, this);
    _button4->setLabel("4");
    _button4->fontSize = fontSize;
    _button4->bgColor = bgColor;

    _button5 = new Button(Point{x: cols[1] + padding, y: rows[1] + padding}, size);
    _button5->then([](void *context) {
        static_cast<InputViewController*>(context)->write('5');
    }, this);
    _button5->setLabel("5");
    _button5->fontSize = fontSize;
    _button5->bgColor = bgColor;

    _button6 = new Button(Point{x: cols[2] + padding, y: rows[1] + padding}, size);
    _button6->then([](void *context) {
        static_cast<InputViewController*>(context)->write('6');
    }, this);
    _button6->setLabel("6");
    _button6->fontSize = fontSize;
    _button6->bgColor = bgColor;

    _button1 = new Button(Point{x: cols[0] + padding, y: rows[2] + padding}, size);
    _button1->then([](void *context) {
        static_cast<InputViewController*>(context)->write('1');
    }, this);
    _button1->setLabel("1");
    _button1->fontSize = fontSize;
    _button1->bgColor = bgColor;

    _button2 = new Button(Point{x: cols[1] + padding, y: rows[2] + padding}, size);
    _button2->then([](void *context) {
        static_cast<InputViewController*>(context)->write('2');
    }, this);
    _button2->setLabel("2");
    _button2->fontSize = fontSize;
    _button2->bgColor = bgColor;

    _button3 = new Button(Point{x: cols[2] + padding, y: rows[2] + padding}, size);
    _button3->then([](void *context) {
        static_cast<InputViewController*>(context)->write('3');
    }, this);
    _button3->setLabel("3");
    _button3->fontSize = fontSize;
    _button3->bgColor = bgColor;

    _button0 = new Button(Point{x: cols[0] + padding, y: rows[3] + padding}, size);
    _button0->then([](void *context) {
        static_cast<InputViewController*>(context)->write('0');
    }, this);
    _button0->setLabel("0");
    _button0->fontSize = fontSize;
    _button0->bgColor = bgColor;

    _buttonBackspace = new Button(Point{x: cols[1] + padding, y: rows[3] + padding}, size);
    _buttonBackspace->then([](void *context) {
        // TODO: Clear on hold
        static_cast<InputViewController*>(context)->backspace();
    }, this);
    _buttonBackspace->setLabel("<-");
    _buttonBackspace->fontSize = fontSize;
    _buttonBackspace->bgColor = bgColor;

    _buttonDone = new Button(Point{x: cols[2] + padding, y: rows[3] + padding}, size);
    _buttonDone->then([](void *context) {
        pipe->segueBack();
    });
    _buttonDone->setLabel("Done");
    _buttonDone->fontSize = fontSize;
    _buttonDone->bgColor = bgColor;

    pipe->push(drawControlForwarder, _textbox);
    pipe->push(drawControlForwarder, _button0);
    pipe->push(drawControlForwarder, _button1);
    pipe->push(drawControlForwarder, _button2);
    pipe->push(drawControlForwarder, _button3);
    pipe->push(drawControlForwarder, _button4);
    pipe->push(drawControlForwarder, _button5);
    pipe->push(drawControlForwarder, _button6);
    pipe->push(drawControlForwarder, _button7);
    pipe->push(drawControlForwarder, _button8);
    pipe->push(drawControlForwarder, _button9);
    pipe->push(drawControlForwarder, _buttonBackspace);
    pipe->push(drawControlForwarder, _buttonDone);
}
