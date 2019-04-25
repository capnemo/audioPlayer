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
    xPlot(AVCodecContext* cT, AVSampleFormat iF, std::uint64_t tS, 
          std::uint32_t sR):
          codecCtx(cT), inputFormat(iF), totalSamples(tS), 
          samplingRate(sR) {}

    bool init();
    void plotData(const AVFrame* inFrame);
    ~xPlot();

private:
    void drawAxes() const;
    void plotLine(std::uint32_t channel, std::int32_t currentY);
    void removeDuplicates(std::vector<std::int32_t>& inVec);
    std::int16_t maxAggregate(const std::vector<std::int16_t>& input) const;
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
    std::vector<long> lineColors;
    const AVCodecContext* codecCtx;
    AVSampleFormat inputFormat;
    AVSampleFormat plotFormat = AV_SAMPLE_FMT_S16;
    std::uint64_t totalSamples;
    std::uint32_t samplingRate;
    std::vector<std::vector<std::int16_t>> planarData;
    std::vector<std::vector<std::int16_t>> points;
};

#endif /*XPLOT_H*/
