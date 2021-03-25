#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include "conPlot.h"

/*
 * Initialize the xPlot object. Sets up the connection to the XServer and 
 * starts the graphing window. 
 */
bool conPlot::init()
{
    if (initResampler() == false)
        return false;

    return initX();
}

bool conPlot::initResampler() 
{
    bool rc = true;

    if (inputFormat != plotFormat) {
        resampler = new audioResampler(codecCtx, inputFormat, plotFormat);
        if (resampler == nullptr)  {
            rc = false;
        } else if (resampler->init() == false) {
            delete resampler;
            resampler = nullptr;
            rc = false;
        }
    }

    if (rc == false)
        std::cout << "Error allocating plotter" << std::endl;

    return rc;
}

/* 
 * Class entry point. Plots the data in a frame. Calls the resampler 
 * if necessary.
 * IN: inFrame. Audio frame. Data to be graphed is in AVFrame->data
 */

void conPlot::plotData(const AVFrame* inFrame)
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

void conPlot::appendData(const AVFrame* inFrame)
{
    short* dataPtr = (short *)(inFrame->data[0]);
    for (std::uint32_t i = 0; i < inFrame->nb_samples * numChannels;
                                                            i++) {
        planarData[i%numChannels].push_back(dataPtr[i]);
        if (planarData[i%numChannels].size() == samplesPerPoint) {
            int avg = maxAggregate(planarData[i%numChannels]);
            int y = yRange/2 - (avg * yRange)/dataRange;
            plotLine(i%numChannels, y);
            planarData[i%numChannels].clear();
        }
    }
}

conPlot::~conPlot()
{
    if (resampler != nullptr)
        delete resampler;
}
