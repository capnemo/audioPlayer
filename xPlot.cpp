#include <iostream>
#include <vector>
#include "xPlot.h"

int xPlot::init()
{
    if (inputFormat != plotFormat) {
        resampler = new audioResampler(codecCtx, inputFormat, plotFormat);
        if (resampler == 0) 
            return -1;
        if (resampler->init() == false) {
            delete resampler;
            resampler = 0;
            return -1;
        }
    }

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
    XSelectInput(display, window, ExposureMask);

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

    numChannels = codecCtx->channels;
    xAxisBegin = 10;
    xAxisEnd = windowWidth - 20;
    yAxisBegin = 10;
    yAxisEnd = windowHeight - 20;
    yRange = yAxisEnd - yAxisBegin;
    xRange = xAxisEnd - xAxisBegin;
    currentXPos = xAxisBegin;
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


void xPlot::plotData(const AVFrame* inFrame)
{
    uint8_t* dataPtr = (uint8_t*)inFrame->data[0];
    if (resampler != nullptr) {
        dataPtr = (uint8_t *)resampler->resampleData(inFrame);
        if (dataPtr == nullptr)
            return;
    }

    static std::vector<std::vector<int>> sums(inFrame->channels);
    int i = 0;
    while (i  < inFrame->nb_samples * inFrame->channels) {
        for (int j = 0; j < inFrame->channels; j++) 
            sums[j].push_back(dataPtr[i + j]);
        i += inFrame->channels;

        if (sums[0].size() == szAvg) {
            plotPoints(sums);
            for (int k = 0; k < sums.size(); k++) 
                sums[k].clear();
        }
    }
}

void xPlot::plotPoints(std::vector<std::vector<int>>& yVals)
{
    std::vector<int> aggregates(yVals.size(), 0);
    
    for (int i = 0; i < yVals.size(); i++) 
        removeDuplicates(yVals[i]);

    for (int i = 0; i < yVals.size(); i++) 
        for (int j = 0; j < yVals[i].size(); j++) 
            aggregates[i] += yVals[i][j];

    for (int i = 0; i < aggregates.size(); i++)
        aggregates[i] = (aggregates[i]/yVals[i].size()) + 0.5;

    int xC = (currentXPos % xRange) + xAxisBegin;
    for (int i = 0; i < aggregates.size(); i++)  {
        int yC = yRange - aggregates[i];
        XDrawPoint(display, window, graphicsContext, xC, yC);
        std::cout << xC << "," << yC << std::endl;
    }
    currentXPos++;
    XFlush(display);
}

void xPlot::removeDuplicates(std::vector<int>& inVec)
{
    std::vector<int> tmp;
    int i = 0;
    while (i < inVec.size()) {
        tmp.push_back(inVec[i]);
        while ((tmp[tmp.size() - 1] == inVec[i]) && (i < inVec.size()))
            i++;
    }
    inVec = tmp;
}

xPlot::~xPlot()
{
    if (resampler != 0)
        delete resampler;
    
    //XDestroyWindow(display, window);
    //XCloseDisplay(display);   
}
