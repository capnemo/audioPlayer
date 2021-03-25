#include <thread>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include "audioPlayer.h"


/*
 * Initialize and open up the audio device.
 */

bool audioPlayer::init()
{
    snd_pcm_format_t audioFormat;
    inputSampleFormat = audioCodecCtx->sample_fmt;
    outputSampleFormat = inputSampleFormat;
    switch (audioCodecCtx->sample_fmt) {
        case AV_SAMPLE_FMT_NONE :
            audioFormat = SND_PCM_FORMAT_UNKNOWN;
            break;
        case AV_SAMPLE_FMT_U8:
            audioFormat = SND_PCM_FORMAT_U8;
            formatDivisor = 1;
            break;
        case AV_SAMPLE_FMT_U8P:
            audioFormat = SND_PCM_FORMAT_U8;
            formatDivisor = 1;
            outputSampleFormat = AV_SAMPLE_FMT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            audioFormat = SND_PCM_FORMAT_S16_LE;
            formatDivisor = 2;
            break;
        case AV_SAMPLE_FMT_S16P:
            audioFormat = SND_PCM_FORMAT_S16_LE;
            formatDivisor = 2;
            outputSampleFormat = AV_SAMPLE_FMT_S16;
            break;
        case AV_SAMPLE_FMT_S32:
            audioFormat =  SND_PCM_FORMAT_S32_LE;
            formatDivisor = 4;
            break;
        case AV_SAMPLE_FMT_S32P:
            audioFormat =  SND_PCM_FORMAT_S32_LE;
            formatDivisor = 4;
            break;
        case AV_SAMPLE_FMT_FLT:
            audioFormat =  SND_PCM_FORMAT_FLOAT;
            formatDivisor = 4;
            break;
        case AV_SAMPLE_FMT_FLTP:
            audioFormat =  SND_PCM_FORMAT_FLOAT;
            formatDivisor = 4;
            outputSampleFormat = AV_SAMPLE_FMT_FLT;
            break;
        case AV_SAMPLE_FMT_DBL:
            audioFormat =  SND_PCM_FORMAT_FLOAT64;
            formatDivisor = 8;
            break;
        case AV_SAMPLE_FMT_DBLP:
            audioFormat =  SND_PCM_FORMAT_FLOAT64;
            formatDivisor = 8;
            break;
        default:
            audioFormat = SND_PCM_FORMAT_UNKNOWN;
    }

    if (audioFormat == SND_PCM_FORMAT_UNKNOWN)
        return  false; 

    formatDivisor *= audioCodecCtx->channels;
    if (inputSampleFormat != outputSampleFormat)  {
        planarResampler = new audioResampler(audioCodecCtx, 
                                             inputSampleFormat,
                                             outputSampleFormat);
        if (planarResampler->init() == false)
            return false;
    }
    
    if (plot == true) {
        plotter = new conPlot(audioCodecCtx, outputSampleFormat, totalSamples);
        if ((plotter != 0) && (plotter->init() == false))  {
            plot = false;
            delete plotter;
            std::cout << "Error initializing plotter for graph" << std::endl;
        }
    }

    if (snd_pcm_open(&playbackHandle, "default", 
                       SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        std::cout << "Error opening the audio device" << std::endl;
        return false;
    }

    int err;
    err = snd_pcm_set_params(playbackHandle, audioFormat,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             audioCodecCtx->channels,
                             audioCodecCtx->sample_rate, 0, 500000);
    if (err < 0)
        return false;

    return true;
}

/*
 * Thread function that writes the decoded data to the audio device. 
 * Converts planar data to interleaved data if needed. Useful for mp3.
 * If set, runs the plotter.
 */

void audioPlayer::threadFunc()
{
    bool fatalError = false;
    beginClock = stdClock::now();
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

        AVFrame resFrame;
        void* samples = frame->data[0];
        AVFrame* plotInputFrame = frame;
        if (planarResampler != 0) {
            bzero((void *)&resFrame, sizeof(resFrame));
            if (planarResampler->resampleFrame(frame, &resFrame) == true) {
                samples = (void *)resFrame.data[0];
                plotInputFrame = &resFrame;
            } else 
                continue;
        }

        if (plot == true)
            plotter->plotData(plotInputFrame);

        int wRc = snd_pcm_writei(playbackHandle, samples, dataSz);
        if (wRc == -EPIPE) {
            if (snd_pcm_prepare(playbackHandle) < 0)  {
                std::cout << "Cannot recover from write error" << std::endl;
                fatalError = true;
            }
        } else if (wRc == -ESTRPIPE) {
            int strErr;
            while ((strErr = snd_pcm_resume(playbackHandle)) == -EAGAIN) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if (strErr < 0) {
                    if (snd_pcm_prepare(playbackHandle) < 0) {
                        std::cout << "Cannot recover from broken stream" <<                             std::endl;
                    }
                }
            }
        }

        if (planarResampler != 0)  
            av_frame_unref(&resFrame);

        av_frame_unref(frame);
        av_frame_free(&frame);
        if (fatalError == true)
            break;
    }

    if (fatalError == false)
        snd_pcm_drain(playbackHandle);
    endClock = stdClock::now();
}

/*  
 * Resample the audio frame from the input format to the output format.
 */
#if 0
bool audioPlayer::resampleAudioData(SwrContext* resampleCtx, 
                                    AVFrame* outputFrame,
                                    AVFrame* inputFrame,
                                    AVSampleFormat inFormat,
                                    AVSampleFormat outFormat)
{
    outputFrame->format = outFormat;
    outputFrame->channel_layout = audioCodecCtx->channel_layout;
    outputFrame->sample_rate = audioCodecCtx->sample_rate;

    int cRc = swr_convert_frame(resampleCtx, outputFrame, inputFrame);
    if (cRc != 0) {
        av_frame_unref(outputFrame);
        char errBuf[100];
        av_strerror(cRc, errBuf, 100);
        std::cout << "Resample error: " << errBuf << std::endl;
        return false;
    }
    return true;
}
#endif

/*
 * Destructor.
 * Also prints out the time taken to play the stream
 */
audioPlayer::~audioPlayer()
{
    snd_pcm_close(playbackHandle);
    snd_config_update_free_global();

    if (planarResampler != 0)
        delete planarResampler;

    if (plotResCtx != 0)
        swr_free(&plotResCtx);
    
    if (plotter != 0)
        delete plotter;

    std::chrono::duration<double> elapsedTime = endClock - beginClock;
    std::cout << "Elapsed time is " << elapsedTime.count() << " seconds" << 
    std::endl;
}
