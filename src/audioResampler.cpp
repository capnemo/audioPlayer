#include "audioResampler.h"
#include <iostream>

bool audioResampler::init()
{
    channel_layout = codecContext->ch_layout;

    int rc = swr_alloc_set_opts2(&resampleContext, 
                                 &channel_layout,
                                 outputFormat, 
                                 codecContext->sample_rate,
                                 &channel_layout,
                                 inputFormat,
                                 codecContext->sample_rate, 0, 0);

    
    if (rc != 0) 
        return false;

    if (swr_init(resampleContext) != 0)
            return false;

    return true;
}

bool audioResampler::resampleFrame(const AVFrame* inputFrame, 
                                   AVFrame* outputFrame)
{
    outputFrame->format = outputFormat;
    outputFrame->ch_layout = channel_layout;
    
    outputFrame->sample_rate = codecContext->sample_rate;

    int cRc = swr_convert_frame(resampleContext, outputFrame, inputFrame);
    if (cRc != 0) {
        char errBuf[100];
        av_strerror(cRc, errBuf, 100);
        std::cout << "Resample error: " << errBuf << std::endl;
        return false;
    }
    return true;
}

bool audioResampler::flushable() 
{
    if (swr_get_delay(resampleContext, codecContext->sample_rate) != 0)
        return true;
    return false; 
}

void* audioResampler::resampleData(const AVFrame* inputFrame)
{
    uint8_t *outData;
    if (av_samples_alloc(&outData, 0, codecContext->ch_layout.nb_channels,
                           inputFrame->nb_samples, 
                           codecContext->sample_fmt, 0) < 0) {
        std::cout << "Error allocating resampling frame" << std::endl;
        return nullptr;
    }
 
    int cRC = swr_convert(resampleContext, &outData, inputFrame->nb_samples, 
                          (const uint8_t **)inputFrame->data, 
                           inputFrame->nb_samples);
    std::cout << "SWR Convert " << cRC << std::endl;
    if (cRC <= 0) {
        std::cout << "Error converting frame" << std::endl;
        av_freep(&outData);
        return nullptr;
    }

    return (void *) outData;
}

audioResampler::~audioResampler()
{
    if (resampleContext != 0) 
        swr_free(&resampleContext);
}
