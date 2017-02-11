#ifndef TRANSPOND_GRAPHICS_TEST_H
#define TRANSPOND_GRAPHICS_TEST_H

#include "../gui.h"

class GraphicsTestViewController : public ViewController {
protected:
    void doInit();

    void doDeInit() { }

public:
    GraphicsTestViewController() : ViewController() { }

    ~GraphicsTestViewController() { }

    void tick();

    void draw();
};

#endif
