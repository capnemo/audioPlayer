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
#include "xPlot.h"

#ifndef CONPLOT_H
#define CONPLOT_H

class conPlot: public xPlot {
public:
    conPlot(AVCodecContext* cT, AVSampleFormat iF, std::uint64_t tS):
            codecCtx(cT), inputFormat(iF), totalSamples(tS),
            numChannels(codecCtx->channels) {}

    virtual bool init() override;
    virtual void plotData(const AVFrame* inFrame) override;
    virtual ~conPlot();

    private:
    bool initResampler();
    void appendData(const AVFrame* inFrame);
    void initializeWindowData();
    void setColors();
    void drawAxes();
    void plotLine(std::uint32_t channel, std::int32_t currentY);
    void removeDuplicates(std::vector<std::int32_t>& inVec);
    std::int16_t maxAggregate(const std::vector<std::int16_t>& input) const;

    private:
    const AVCodecContext* codecCtx; 
    AVSampleFormat inputFormat; 
    std::uint64_t totalSamples; 
    std::uint32_t numChannels;

    const AVSampleFormat plotFormat = AV_SAMPLE_FMT_S16;
    audioResampler* resampler = nullptr;

    std::uint32_t xAxisBegin, xAxisEnd;
    std::uint32_t yAxisBegin, yAxisEnd;
    std::uint32_t currentXPos = 0;
    std::vector<short> xCoord;
    std::vector<short> yCoord;
    std::uint32_t yRange, xRange;
    std::uint32_t dataRange;
    std::uint32_t samplesPerPoint;
    std::vector<std::vector<std::int16_t>> points;
    std::vector<std::vector<std::int16_t>> planarData;
};

#endif /*CONPLOT_H*/
