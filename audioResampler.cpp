#include "audioResampler.h"
#include <iostream>

bool audioResampler::init()
{
    channelLayout = codecContext->channel_layout;
    if (channelLayout == 0) 
        channelLayout = av_get_channel_layout_nb_channels(codecContext->channels);

    resampleContext = swr_alloc_set_opts(nullptr, 
                                         channelLayout,
                                         outputFormat, 
                                         codecContext->sample_rate,
                                         channelLayout,
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
    //outputFrame->channel_layout = channelLayout;
    outputFrame->sample_rate = codecContext->sample_rate;

    int cRc = swr_convert_frame(resampleContext, outputFrame, inputFrame);
    if (cRc != 0) {
        //av_frame_unref(outputFrame);
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

#if 0
bool audioResampler::flush()
{
    int cRc = swr_convert_frame(resampleContext, outputFrame, 0);
    if (cRc != 0) {
        //av_frame_unref(outputFrame);
        char errBuf[100];
        av_strerror(cRc, errBuf, 100);
        std::cout << "Resample error: " << errBuf << std::endl;
        return false;
    }
}
#endif

void* audioResampler::resampleData(const AVFrame* inputFrame)
{
    uint8_t *outData;
    if (av_samples_alloc(&outData, 0, codecContext->channels,
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
    /*
    if (swr_convert(resampleContext, &outData, inputFrame->nb_samples, 
                    (const uint8_t **)inputFrame->data, 
                    inputFrame->nb_samples) < 0) {
        std::cout << "Error converting frame" << std::endl;
        av_freep(&outData);
        return nullptr;
    }
    */
    return (void *) outData;
}

audioResampler::~audioResampler()
{
    if (resampleContext != 0) 
        swr_free(&resampleContext);
}
