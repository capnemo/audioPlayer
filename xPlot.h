#include <limits>
#include <chrono>
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

class xPlot{
public:
    //xPlot(int nC, int tS):numChannels(nC), totalSamples(tS),cdcCtx(0) {}
    xPlot(AVCodecContext* cT, AVSampleFormat iF, int tS, int sR):
          codecCtx(cT), inputFormat(iF), totalSamples(tS), 
          samplingRate(sR) {}

    int init();
    void plotData(short* buffer, int buffSize);
    void plotData(const AVFrame* inFrame);
    ~xPlot();

private:
    void drawAxes();
    void graphData();
    void plotPoints(std::vector<std::vector<int>>& yVals);
    void plotLine(int channel, int currentY);
    void removeDuplicates(std::vector<int>& inVec);
    int avgUnique(const std::vector<short>& input);
    void plotFull();
    void plotStream();
    void appendData(const AVFrame* inFrame);

private:
    Display *display = 0;
    int screen;        
    Window window;        
    GC graphicsContext;
    XGCValues gcValues;
    int xAxisBegin, xAxisEnd;
    int yAxisBegin, yAxisEnd;
    int yRange, xRange;
    int currentXPos = 0;
    int szAvg = 100;
    int numChannels;
    int samplesPerPoint;
    int dataRange;
    std::vector<short> xCoord;
    std::vector<short> yCoord;
    audioResampler* resampler = nullptr;
    std::vector<long> lineColors;
    const AVCodecContext* codecCtx;
    AVSampleFormat inputFormat;
    AVSampleFormat plotFormat = AV_SAMPLE_FMT_S16;
    long totalSamples;
    int samplingRate;
    std::vector<std::vector<short>> planarData;
    std::vector<std::vector<short>> points;
};

#endif /*XPLOT_H*/
