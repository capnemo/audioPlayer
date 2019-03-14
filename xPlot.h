#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
#include <alsa/asoundlib.h>

#include "lockedQ.h"

#ifndef XPLOT_H
#define XPLOT_H

class xPlot{
public:
    xPlot(int nC, int tS):numChannels(nC), totalSamples(tS) {}

    int init();
    void plotData(short* buffer, int buffSize);

private:
    void waitForUserInput();
    void drawAxes();

private:
    Display *display;  
    int screen;        
    Window window;        
    XEvent event;      /* The event data structure */
    GC graphicsContext;
    XGCValues gcValues;
    double lowerBound = std::numeric_limits<short>::min();
    double upperBound = std::numeric_limits<short>::max();
    double dataRange = upperBound - lowerBound;
    int xAxisBegin, xAxisEnd;
    int yAxisBegin, yAxisEnd;
    int currentXPos;
    int samplesPerPixel;
    int numChannels;
    long totalSamples;
};

#endif /*XPLOT_H*/
