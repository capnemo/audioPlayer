#include <limits>
#include <chrono>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#include "audioResampler.h"

#ifndef XPLOT_H
#define XPLOT_H

class xPlot {
public:
    xPlot() {}
    virtual bool init() = 0;
    virtual void plotData(const AVFrame* inFrame) = 0;
    virtual ~xPlot();

protected:
    bool initX();
    void drawWindow(int serial);
    void setColors();

    void drawWhiteLine(std::uint32_t x1, std::uint32_t y1, 
                       std::uint32_t x2, std::uint32_t y2);
    void drawColoredLine(std::uint32_t x1, std::uint32_t y1, 
                         std::uint32_t x2, std::uint32_t y2, 
                         long colorIndex);
protected:
    std::vector<long> lineColors;
    std::uint32_t windowWidth, windowHeight;
    std::int32_t screen;        
    Display *display = 0;
    
private:
    Window window;        
    GC graphicsContext;
    XGCValues gcValues;
};

#endif /*XPLOT_H*/
