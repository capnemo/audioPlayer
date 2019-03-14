#include <iostream>
#include "xPlot.h"

int xPlot::init()
{
    if ((display = XOpenDisplay(0)) == 0) 
        return -1;
    
    screen = DefaultScreen(display);
    Screen* scrn = ScreenOfDisplay(display, screen);
    int screenWidth = scrn->width;
    int screenHeight = scrn->height;
    if (screenWidth > 2 * screenHeight)
        screenWidth = 1.5 * screenHeight;
    
    int windowWidth = screenWidth - 0.2 * screenWidth;
    int windowHeight = screenHeight * 0.33;
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 0, 0, windowWidth, windowHeight, 1, 
                                 BlackPixel(display, screen), 
                                 WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | ButtonPressMask);

    graphicsContext = XCreateGC(display, window, 0, &gcValues);
    XSetForeground(display, graphicsContext, BlackPixel(display, screen));
    XSetBackground(display, graphicsContext, WhitePixel(display, screen));
    XSetLineAttributes(display, graphicsContext, 1, LineSolid, CapRound,
                       JoinRound);
    XMapWindow(display, window);

    XEvent event;
    do {
        XNextEvent(display, &event);
    } while (event.type != Expose);

    xAxisBegin = 10;
    xAxisEnd = windowWidth - 20;
    yAxisBegin = 10;
    yAxisEnd = windowHeight - 20;
    currentXPos = xAxisBegin;
    samplesPerPixel = totalSamples/(numChannels * (yAxisEnd - yAxisBegin));
    
    drawAxes();
    return 0;
}

void xPlot::drawAxes() 
{
    XDrawLine(display, window, graphicsContext, xAxisBegin, yAxisBegin, 
              xAxisBegin, yAxisEnd);
    XDrawLine(display, window, graphicsContext, xAxisBegin, yAxisEnd,
              xAxisEnd, yAxisEnd);

    int interval = 100;
    for (int i = xAxisBegin + interval; i < xAxisEnd; i += interval) {
        XDrawLine(display, window, graphicsContext, i, yAxisEnd, 
                  i, yAxisEnd + 5);
    }
    
    XFlush(display);
}

void xPlot::plotData(short *inputBuffer, int inputBufferSize)
{
    static std::vector<std::vector<short>> channelVectors(numChannels);
    
    return;
    for (int i = 0; i < inputBufferSize; i++) 
        channelVectors[i%numChannels].push_back(inputBuffer[i]);
    
    std::vector<std::vector<int>> channelSums(numChannels);
    for (int i = 0; i < channelVectors.size(); i++) {
        int aveSamples = (channelVectors[i].size()/samplesPerPixel) 
                         * samplesPerPixel;
        int planarSum = 0;
        for (int j = 0; j < aveSamples; j++) {
            planarSum += channelVectors[i][j];
            if (j%samplesPerPixel == 0) {
                channelSums[i].push_back(planarSum);
                planarSum = 0;
            }
        }
    }
    
    for (int j = 0; j < channelSums[0].size(); j++) {
        for (int i = 0; i < numChannels; i++) {
            int avg = channelSums[i][j]/samplesPerPixel;
            int point = ((avg + lowerBound)*yAxisEnd)/dataRange;
            XDrawPoint(display, window, graphicsContext, currentXPos++, 
            point + yAxisBegin - yAxisEnd);
        }
    }
}

//ToBeRemoved.
void xPlot::waitForUserInput()
{
    while (1) {
        XNextEvent(display, &event);
        switch (event.type) {
            case Expose: 
                 drawAxes();  
                 break; /* Draw/re-draw everything */
            case ButtonPress: 
                 exit(0); 
                 break; /* Quit on mouse button press */
            default: break;
        }
    }
}

