#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include "xPlot.h"

/*
 * Initialize the xPlot object. Sets up the connection to the XServer and 
 * starts the graphing window. 
 */
bool xPlot::init()
{
    if (inputFormat != plotFormat) {
        bool rc = true;
        resampler = new audioResampler(codecCtx, inputFormat, plotFormat);
        if (resampler == nullptr) 
            rc = false;
        if (resampler->init() == false) {
            delete resampler;
            resampler = nullptr;
            rc = false;
        }
        if (rc == false) {
            std::cout << "Error allocating plotter" << std::endl;
            return rc;
        }
    }

    numChannels = codecCtx->channels;
    if ((display = XOpenDisplay(0)) == 0)  {
        std::cout << "Error opening connection to the X server" << std::endl;
        return false;
    }
    
    screen = DefaultScreen(display);
    Colormap screenColorMap = DefaultColormap(display, screen);
    std::vector<std::string> colors = {"red","blue","green","yellow","white"};

    for (auto col:colors) {
        XColor sCol;
        int rc = XAllocNamedColor(display, screenColorMap, col.c_str(), &sCol, 
                                  &sCol);
        if (rc != 0) 
            lineColors.push_back(sCol.pixel);
    }


    Screen* scrn = ScreenOfDisplay(display, screen);
    int screenWidth = scrn->width;
    int screenHeight = scrn->height;
    if (screenWidth > 2 * screenHeight)
        screenWidth = 1.5 * screenHeight;
    
    int windowWidth = screenWidth - 0.2 * screenWidth;
    int windowHeight = screenHeight * 0.33;

    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                0, 0, windowWidth, windowHeight, 1, 0,
                                BlackPixel(display, screen));

    XSelectInput(display, window, ExposureMask);

    graphicsContext = XCreateGC(display, window, 0, &gcValues);
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
    yRange = yAxisEnd - yAxisBegin;
    xRange = xAxisEnd - xAxisBegin;
    for (std::uint32_t i = 0; i < numChannels; i++)  {
        planarData.push_back(std::vector<short>());
        points.push_back(std::vector<short>());
        xCoord.push_back(xAxisBegin);
        yCoord.push_back(yRange/2);
    }
    currentXPos = xAxisBegin;
    samplesPerPoint = (totalSamples/(double)xRange) + 0.5;
    dataRange = std::numeric_limits<short>::max() 
                - std::numeric_limits<short>::min();
    drawAxes();
    
    return true;
}

/*  
 *  Draws the X and Y axes on the graphing window.
 *
 */
void xPlot::drawAxes() const
{
    XSetForeground(display, graphicsContext, WhitePixel(display, screen));
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

/* 
 * Class entry point. Plots the data in a frame. Calls the resampler 
 * if necessary.
 * IN: inFrame. Audio frame. Data to be graphed is in AVFrame->data
 */

void xPlot::plotData(const AVFrame* inFrame)
{
    if (resampler != nullptr) {
        AVFrame plotFrame;
        bzero((void *)&plotFrame, sizeof(AVFrame));
        if (resampler->resampleFrame(inFrame, &plotFrame) == true)  {
            appendData(&plotFrame);
            av_frame_unref(&plotFrame);
            while (resampler->flushable() == true) {
                bzero((void *)&plotFrame, sizeof(AVFrame));
                resampler->resampleFrame(nullptr, &plotFrame);
                appendData(&plotFrame);
                av_frame_unref(&plotFrame);
            }
        } else  {
            std::cout << "Plot resample fail" << std::endl;
            return;
        }
    } else {
        appendData(inFrame);
    }
}

/* Appends data to the coordinate vectors and calls the plotting function.
 * IN: inFrame. Audio frame. Data to be graphed is in AVFrame->data
 */
void xPlot::appendData(const AVFrame* inFrame)
{
    short* dataPtr = (short *)(inFrame->data[0]);
    for (int i = 0; i < inFrame->nb_samples * numChannels; i++) {
        planarData[i%numChannels].push_back(dataPtr[i]);
        if (planarData[i%numChannels].size() == samplesPerPoint) {
            int avg = maxAggregate(planarData[i%numChannels]);
            int y = yRange/2 - (avg * yRange)/dataRange;
            plotLine(i%numChannels, y);
            planarData[i%numChannels].clear();
        }
    }
}

/*
 * Draws a line from the previous point to the current one.
 * IN channel is the channel number.
 * IN currentY is the data point. 
 */
void xPlot::plotLine(std::uint32_t channel, std::int32_t currentY)
{
    XSetForeground(display, graphicsContext, lineColors[channel]);
    XDrawLine(display, window, graphicsContext, 
              xCoord[channel], yCoord[channel], xCoord[channel] + 1, currentY);
    xCoord[channel]++;
    yCoord[channel] = currentY;
    XFlush(display);
}

/*
 *  Returns the max of a set of points.
 *  IN input. Set of points to be aggregated.
 *  OUT  Max of input.
 */
std::int16_t xPlot::maxAggregate(const std::vector<std::int16_t>& input) const
{
    std::int16_t max = std::abs(input[0]);
    int index = 0;
    for (int i = 0; i < input.size(); i++) {
        if (max < std::abs(input[i])) {
            index = i;
            max = std::abs(input[i]);
        }
    }
    return input[index];
}

#if 0 //Keep this!!! Change to use std::?int??_t
//Returns the average of the input.
int xPlot::avgUnique(const std::vector<short>& input)
{
    
    std::set<short> uniques;
    for (auto m:input) 
        uniques.insert(m);
    
    int avg = 0;
    for (auto u:uniques)
        avg += u;

    return avg/(int)uniques.size();
}
#endif

/* 
 * Removes duplicates from the input.
 * IN inVec. 
 * OUT inVec Unique set from the input.
 */

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

/*
 * Destructor.
 */
xPlot::~xPlot()
{
    if (resampler != nullptr)
        delete resampler;
 
    XFreeGC(display, graphicsContext);   
    XDestroyWindow(display, window);
    XCloseDisplay(display);   
}
