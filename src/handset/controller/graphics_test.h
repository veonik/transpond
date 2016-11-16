#ifndef TRANSPOND_GRAPHICS_TEST_H
#define TRANSPOND_GRAPHICS_TEST_H

#include "../gui.h"

class GraphicsTestViewController : public ViewController {
protected:
    void doInit();

    void doDeInit() { }

public:
    GraphicsTestViewController() : ViewController() { }

    ~GraphicsTestViewController() { deinit(); }

    void tick();

    void draw();

    unsigned long testFillScreen();

    unsigned long testText();

    unsigned long testLines(uint16_t color);

    unsigned long testFastLines(uint16_t color1, uint16_t color2);

    unsigned long testRects(uint16_t color);

    unsigned long testFilledRects(uint16_t color1, uint16_t color2);

    unsigned long testFilledCircles(uint8_t radius, uint16_t color);

    unsigned long testCircles(uint8_t radius, uint16_t color);

    unsigned long testTriangles();

    unsigned long testFilledTriangles();

    unsigned long testRoundRects();

    unsigned long testFilledRoundRects();
};

#endif
