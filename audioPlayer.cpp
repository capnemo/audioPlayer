#include <thread>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include "audioPlayer.h"

int audioPlayer::init()
{
    AVSampleFormat inputSampleFormat, outputSampleFormat;

    snd_pcm_format_t audioFormat;
    switch (audioCodecCtx->sample_fmt) {
        case AV_SAMPLE_FMT_NONE :
            audioFormat = SND_PCM_FORMAT_UNKNOWN;
            break;
        case AV_SAMPLE_FMT_U8:
            audioFormat = SND_PCM_FORMAT_U8;
            formatDivisor = 1;
            interleaved = true;
            break;
        case AV_SAMPLE_FMT_U8P:
            audioFormat = SND_PCM_FORMAT_U8;
            formatDivisor = 1;
            interleaved = false;
            inputSampleFormat = AV_SAMPLE_FMT_U8P;
            outputSampleFormat = AV_SAMPLE_FMT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            audioFormat = SND_PCM_FORMAT_S16_LE;
            formatDivisor = 2;
            interleaved = true;
            break;
        case AV_SAMPLE_FMT_S16P:
            audioFormat = SND_PCM_FORMAT_S16_LE;
            formatDivisor = 2;
            interleaved = false;
            inputSampleFormat = AV_SAMPLE_FMT_S16P;
            outputSampleFormat = AV_SAMPLE_FMT_S16;
            break;
        case AV_SAMPLE_FMT_S32:
            audioFormat =  SND_PCM_FORMAT_S32_LE;
            formatDivisor = 4;
            interleaved = true;
            break;
        case AV_SAMPLE_FMT_S32P:
            audioFormat =  SND_PCM_FORMAT_S32_LE;
            formatDivisor = 4;
            interleaved = false;
            inputSampleFormat = AV_SAMPLE_FMT_S32P;
            outputSampleFormat = AV_SAMPLE_FMT_S32;
            break;
        case AV_SAMPLE_FMT_FLT:
            audioFormat =  SND_PCM_FORMAT_FLOAT;
            formatDivisor = 4;
            interleaved = true;
            break;
        case AV_SAMPLE_FMT_FLTP:
            audioFormat =  SND_PCM_FORMAT_FLOAT;
            formatDivisor = 4;
            interleaved = false;
            inputSampleFormat = AV_SAMPLE_FMT_FLTP;
            outputSampleFormat = AV_SAMPLE_FMT_FLT;
            break;
        case AV_SAMPLE_FMT_DBL:
            audioFormat =  SND_PCM_FORMAT_FLOAT64;
            formatDivisor = 8;
            interleaved = true;
            break;
        case AV_SAMPLE_FMT_DBLP:
            audioFormat =  SND_PCM_FORMAT_FLOAT64;
            formatDivisor = 8;
            interleaved = false;
            inputSampleFormat = AV_SAMPLE_FMT_DBLP;
            outputSampleFormat = AV_SAMPLE_FMT_DBL;
            break;
        default:
            audioFormat = SND_PCM_FORMAT_UNKNOWN;
    }

    if (audioFormat == SND_PCM_FORMAT_UNKNOWN)
        return  -1; 

    formatDivisor *= audioCodecCtx->channels;
    int err;
    err = snd_pcm_open(&playbackHandle, "default", SND_PCM_STREAM_PLAYBACK, 0); 
    if (err < 0) 
        return -1;
    
    err = 1;
    if (interleaved == false) {
        resCtx = swr_alloc();
        av_opt_set_sample_fmt(resCtx, "in_sample_fmt", inputSampleFormat, 0);
        av_opt_set_sample_fmt(resCtx, "out_sample_fmt", outputSampleFormat, 0);
        av_opt_set_int(resCtx, "in_channel_layout",
                                audioCodecCtx->channel_layout,  0);
        av_opt_set_int(resCtx, "out_channel_layout",
                                audioCodecCtx->channel_layout,  0);
        av_opt_set_int(resCtx, "in_sample_rate", audioCodecCtx->sample_rate, 0);
        av_opt_set_int(resCtx, "out_sample_rate", audioCodecCtx->sample_rate, 0);
        err = swr_init(resCtx);
        if (err  < 0)
            std::cout << "ERROR on swr init " << err << std::endl;
    }

    if (err < 0) 
        return -1;
        
    err = snd_pcm_set_params(playbackHandle, audioFormat,
                             SND_PCM_ACCESS_RW_INTERLEAVED,                                                  audioCodecCtx->channels, 
                             audioCodecCtx->sample_rate, 0, 500000);
    if (err < 0)
        return -1;

    return 0;
}

void audioPlayer::threadFunc()
{
    beginClock = std::chrono::high_resolution_clock::now();
    while (frameSource.terminateOutput() == false) {
        AVFrame *frame = frameSource.deQueue();
        if (frame == 0)
            continue;
        int dataSz = av_samples_get_buffer_size(0, audioCodecCtx->channels, 
                                                frame->nb_samples, 
                                                audioCodecCtx->sample_fmt, 
                                                1);
        dataSz /= formatDivisor;
        totalData += dataSz;
        frameCounter++;
        void* samples;
        if (interleaved == false)  {
            uint8_t* dstData; 
            int aRc = av_samples_alloc(&dstData, 0,
                                       audioCodecCtx->channels, 
                                       frame->nb_samples,
                                       audioCodecCtx->sample_fmt, 0);
            if (aRc < 0) {
                std::cout << "Error allocating sample buffer" << std::endl;
                av_frame_unref(frame);
                av_frame_free(&frame);
                continue;
            }   
            
            int resRc = swr_convert(resCtx, &dstData, 
                                    frame->nb_samples, 
                                    (const uint8_t **)frame->data, 
                                    frame->nb_samples);
            if (resRc < 0)  {
                std::cout << "Error on convert" << std::endl;
                av_frame_unref(frame);
                av_frame_free(&frame);
                av_freep(&dstData);
                continue;
            }

            samples = (void *)dstData;
        } else {
            samples = frame->data[0];
        }

        int rc;
        rc = snd_pcm_writei(playbackHandle, samples, dataSz);
        if (rc < 0) 
            std::cout << "Error on snd_pcm_write" << std::endl;

        av_frame_unref(frame);
        av_frame_free(&frame);
        if (interleaved == false)
            av_freep(&samples);
    }
    snd_pcm_drain(playbackHandle);
    endClock = std::chrono::high_resolution_clock::now();
}

audioPlayer::~audioPlayer()
{
    snd_pcm_close(playbackHandle);
    snd_config_update_free_global();
    std::chrono::duration<double> elapsedTime = endClock - beginClock;
    std::cout << "Elapsed time is " << elapsedTime.count() << " seconds" << 
    std::endl;
}
