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

class xPlot{
public:
    //xPlot(int nC, int tS):numChannels(nC), totalSamples(tS),cdcCtx(0) {}
    xPlot(AVCodecContext* cT, AVSampleFormat iF, int tS, int sR):
          codecCtx(cT), inputFormat(iF), totalSamples(tS), 
          samplingRate(sR) {}

    int init();
    void plotData(const AVFrame* inFrame);
    ~xPlot();

private:
    void drawAxes();
    void plotLine(int channel, int currentY);
    void removeDuplicates(std::vector<int>& inVec);
    int avgUnique(const std::vector<short>& input);
    int aggregateMax(const std::vector<short>& input);
    void plotStream();
    void appendData(const AVFrame* inFrame);

private:
    Display *display = 0;
    std::int32_t screen;        
    Window window;        
    GC graphicsContext;
    XGCValues gcValues;
    std::uint32_t xAxisBegin, xAxisEnd;
    std::uint32_t yAxisBegin, yAxisEnd;
    std::uint32_t yRange, xRange;
    std::uint32_t currentXPos = 0;
    std::uint32_t numChannels;
    std::uint32_t samplesPerPoint;
    std::uint32_t dataRange;
    std::vector<short> xCoord;
    std::vector<short> yCoord;
    audioResampler* resampler = nullptr;
    std::vector<std::int64_t> lineColors;
    const AVCodecContext* codecCtx;
    AVSampleFormat inputFormat;
    AVSampleFormat plotFormat = AV_SAMPLE_FMT_S16;
    std::uint64_t totalSamples;
    std::uint32_t samplingRate;
    std::vector<std::vector<std::int16_t>> planarData;
    std::vector<std::vector<std::int16_t>> points;
};

#endif /*XPLOT_H*/
