#include "audioResampler.h"
#include <iostream>

bool audioResampler::init()
{
    resampleContext = swr_alloc_set_opts(nullptr,
                                         codecContext->channel_layout,
                                         outputFormat, 
                                         codecContext->sample_rate,
                                         codecContext->channel_layout,
                                         inputFormat,
                                         codecContext->sample_rate, 0, 0);

    if (resampleContext == 0) 
        return false;

    if (swr_init(resampleContext) != 0)
            return false;

    return true;
}

bool audioResampler::resampleFrame(const AVFrame* inputFrame, 
                                   AVFrame* outputFrame)
{
    outputFrame->format = outputFormat;
    outputFrame->channel_layout = codecContext->channel_layout;
    outputFrame->sample_rate = codecContext->sample_rate;

    int cRc = swr_convert_frame(resampleContext, outputFrame, inputFrame);
    if (cRc != 0) {
        av_frame_unref(outputFrame);
        char errBuf[100];
        av_strerror(cRc, errBuf, 100);
        std::cout << "Resample error: " << errBuf << std::endl;
        return false;
    }
    return true;
}

audioResampler::~audioResampler()
{
    if (resampleContext != 0) 
        swr_free(&resampleContext);
}
