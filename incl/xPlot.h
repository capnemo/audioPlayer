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
    xPlot(AVCodecContext* cT, AVSampleFormat iF, std::uint64_t tS):
          codecCtx(cT), inputFormat(iF), totalSamples(tS), 
          samplingRate(cT->sample_rate) {}
    xPlot() {}
    virtual bool init() = 0;
    virtual void plotData(const AVFrame* inFrame) = 0;
    virtual ~xPlot();

protected:
    bool initX();
    void drawAxes() const;
    void plotLine(std::uint32_t channel, std::int32_t currentY);
    void removeDuplicates(std::vector<std::int32_t>& inVec);
    std::int16_t maxAggregate(const std::vector<std::int16_t>& input) const;

protected:
    const AVCodecContext* codecCtx; //in constructor
    AVSampleFormat inputFormat; //in constructor
    std::uint64_t totalSamples; //in constructor
    std::uint32_t samplingRate; //in constructor
    std::uint32_t numChannels; //in xPlot constructor
    std::uint32_t samplesPerPoint;
    std::vector<std::vector<std::int16_t>> planarData;
    std::uint32_t yRange, xRange;
    std::uint32_t dataRange;
    
private:
    Display *display = 0;
    std::int32_t screen;        
    Window window;        
    GC graphicsContext;
    XGCValues gcValues;
    std::uint32_t xAxisBegin, xAxisEnd;
    std::uint32_t yAxisBegin, yAxisEnd;
    std::uint32_t currentXPos = 0;
    std::vector<short> xCoord;
    std::vector<short> yCoord;
    std::vector<long> lineColors;
    std::vector<std::vector<std::int16_t>> points;
};

#endif /*XPLOT_H*/
