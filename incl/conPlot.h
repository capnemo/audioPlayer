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
            xPlot(cT, iF, tS) {}

    virtual bool init() override;
    virtual void plotData(const AVFrame* inFrame) override;
    virtual ~conPlot();

    private:
    bool initResampler();
    void appendData(const AVFrame* inFrame);

    private:
    const AVSampleFormat plotFormat = AV_SAMPLE_FMT_S16;
    audioResampler* resampler = nullptr;
};

#endif /*CONPLOT_H*/
