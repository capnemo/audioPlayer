
/* 
 * Class to write a decoded audio stream to the audio device.
 * Used the ALSA api (libasound) to interface with the playback device
 * If the input stream is planar, the stream is interleaved.
 * The output stream to the plotter is always AV_SAMPLE_FMT_S16
 */


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}


#include <chrono>
#include <alsa/asoundlib.h>
#include "threadRunner.h"
#include "lockedQ.h"
#include "audioResampler.h"
#include "xPlot.h"

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

using stdClock = std::chrono::high_resolution_clock;
class audioPlayer:public threadRunner {
public:
    audioPlayer(AVCodecContext* cdcCtx, lockedQ<AVFrame*>& fSrc, 
                bool pL = false):
                audioCodecCtx(cdcCtx), plot(pL), frameSource(fSrc) {}

    audioPlayer(const audioPlayer&) = delete;
    audioPlayer& operator = (const audioPlayer&) = delete;

    int init();
    void threadFunc();
    ~audioPlayer();
 
private: 
    bool resampleAudioData(SwrContext* resampleCtx, AVFrame* outputFrame,
                           AVFrame* inputFrame, AVSampleFormat inFormat,
                           AVSampleFormat outFormat);

private:
    snd_pcm_t *playbackHandle;
    uint32_t formatDivisor = 0;
    uint32_t frameCounter = 0;
    uint32_t totalData = 0;
    AVCodecContext* audioCodecCtx;
    audioResampler* planarResampler = 0;
    SwrContext* plotResCtx = 0;
    xPlot* plotter = 0;
    bool plot;
    AVSampleFormat inputSampleFormat, outputSampleFormat;
    stdClock::time_point beginClock, endClock;
    lockedQ<AVFrame*>& frameSource;
};

#endif /*AUDIOPLAYER_H*/
