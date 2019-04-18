#include <iostream>
#include <vector>
#include <set>
#include <thread>
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

    numChannels = codecCtx->channels;
    if ((display = XOpenDisplay(0)) == 0) 
        return -1;
    
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
    for (int i = 0; i < numChannels; i++)  {
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
    
    return 0;
}

void xPlot::drawAxes() 
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

//Called from outside.
void xPlot::plotData(const AVFrame* inFrame)
{
    if (resampler != nullptr) {
        AVFrame plotFrame;
        bzero((void *)&plotFrame, sizeof(AVFrame));
        if (resampler->resampleFrame(inFrame, &plotFrame) == true)  {
            appendData(&plotFrame);
            av_frame_unref(&plotFrame);
            while (resampler->flushable() == true) {
                std::cout << "AAA" << std::endl;
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

void xPlot::appendData(const AVFrame* inFrame)
{
    short* dataPtr = (short *)(inFrame->data[0]);
    for (int i = 0; i < inFrame->nb_samples * numChannels; i++) {
        planarData[i%numChannels].push_back(dataPtr[i]);
        if (planarData[i%numChannels].size() == samplesPerPoint) {
            int avg = avgUnique(planarData[i%numChannels]);
            int y = yRange/2 - (avg * yRange)/dataRange;
            plotLine(i%numChannels, y);
            planarData[i%numChannels].clear();
        }
    }
}

void xPlot::plotLine(int channel, int currentY)
{
    XSetForeground(display, graphicsContext, lineColors[channel]);
    XDrawLine(display, window, graphicsContext, 
              xCoord[channel], yCoord[channel], xCoord[channel] + 1, currentY);
    xCoord[channel]++;
    yCoord[channel] = currentY;
    XFlush(display);
}

/* This barely works!! To Be Qualified */
void xPlot::plotStream()
{
    for (int i = 0; i < planarData.size(); i++) 
        for (int j = 0; j < planarData[i].size(); j++) 
            planarData[i][j] = yRange/2 - (planarData[i][j] * yRange)/dataRange;
    
    int leftSegment = xRange/5;
    do {
        int currX = 0;
        for (int i = 0; i < planarData.size(); i++) {
            currX = currentXPos;
            XSetForeground(display, graphicsContext, lineColors[i]);
            for (int j = 0; j < planarData[i].size() ;j++) {
                XDrawLine(display, window, graphicsContext, 
                          currX, planarData[i][j], 
                          currX + 1, planarData[i][j + 1]);
                currX++;
                if (currX >= xAxisEnd)  
                    break;
            }
        }
        XFlush(display);
        
        for (int i = 0; i < planarData.size(); i++) {
            if (planarData[i].size() > xRange/5) {
                std::vector<short>::iterator itB = planarData[i].begin();
                std::vector<short>::iterator itE = itB + leftSegment;
                planarData[i].erase(itB, itE);
            }
        }
    
        int displayDelay = (leftSegment*1000)/samplingRate;
        std::this_thread::sleep_for(std::chrono::milliseconds(displayDelay));
        XClearWindow(display, window);
        drawAxes();
        
        currentXPos = currX;
        if (currX == xAxisEnd) 
            currentXPos = xAxisBegin;
    } while (planarData[0].size() > xRange);
}

int xPlot::avgUnique(const std::vector<short>& input)
{
    int max = std::abs(input[0]);
    int index = 0;
    for (int i = 0; i < input.size(); i++) {
        if (max < std::abs(input[i])) {
            index = i;
            max = std::abs(input[i]);
        }
    }
    return (int)input[index];
}

#if 0 //Keep this!!!
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
 
    XFreeGC(display, graphicsContext);   
    XDestroyWindow(display, window);
    XCloseDisplay(display);   
}
