#include <limits>
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
    xPlot(AVCodecContext* cT, AVSampleFormat iF):
          codecCtx(cT), inputFormat(iF) {}

    int init();
    void plotData(short* buffer, int buffSize);
    void plotData(const AVFrame* inFrame);
    ~xPlot();

private:
    void drawAxes();
    void plotPoints(std::vector<std::vector<int>>& yVals);
    void removeDuplicates(std::vector<int>& inVec);


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
    //int samplesPerPixel;
    int numChannels;
    //long totalSamples;
    audioResampler* resampler;
    const AVCodecContext* codecCtx;
    AVSampleFormat inputFormat;
    AVSampleFormat plotFormat = AV_SAMPLE_FMT_U8;
};

#endif /*XPLOT_H*/
