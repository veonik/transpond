#ifndef TRANSPOND_INPUT_H
#define TRANSPOND_INPUT_H

#include "../gui.h"

typedef void (*inputReceivedCallback)(void*, const char*);

class InputViewController : public ViewController {
private:
    Textbox *_textbox;

    Button *_button0;
    Button *_button1;
    Button *_button2;
    Button *_button3;
    Button *_button4;
    Button *_button5;
    Button *_button6;
    Button *_button7;
    Button *_button8;
    Button *_button9;
    Button *_buttonBackspace;
    Button *_buttonDone;

    inputReceivedCallback _onDone = nullptr;
    void *_onDoneContext;

    char _value[16];

protected:
    void doInit();

    void doDeInit() {
        if (_onDone != nullptr) {
            _onDone(_onDoneContext, _textbox->getValue());
            _onDone = nullptr;
            _onDoneContext = nullptr;
        }
        delete _textbox;
        delete _button0;
        delete _button1;
        delete _button2;
        delete _button3;
        delete _button4;
        delete _button5;
        delete _button6;
        delete _button7;
        delete _button8;
        delete _button9;
        delete _buttonBackspace;
        delete _buttonDone;
    }

public:
    InputViewController(const char *value, inputReceivedCallback callback) : InputViewController(value, callback, nullptr) {}
    InputViewController(const char *value, inputReceivedCallback callback, void *context) : ViewController() {
        strncpy(_value, value, 16);
        _onDone = callback;
        _onDoneContext = context;
    }

    ~InputViewController() { }

    void tick();

    void draw();

    void write(char c);

    void backspace();

    void clear();

    const char *getValue();
};


#endif
