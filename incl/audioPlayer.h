
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
#include "conPlot.h"

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

using stdClock = std::chrono::high_resolution_clock;

class audioPlayer:public threadRunner {
public:
    audioPlayer(AVCodecContext* cdcCtx, int64_t tS, std::uint32_t sR,
                lockedQ<AVFrame*>& fSrc, bool pL = false):
                audioCodecCtx(cdcCtx), plot(pL), totalSamples(tS), 
                frameSource(fSrc) {}

    audioPlayer(const audioPlayer&) = delete;
    audioPlayer& operator = (const audioPlayer&) = delete;

    bool init();
    void threadFunc();
    ~audioPlayer();
 
private:
    snd_pcm_t *playbackHandle;
    std::uint32_t formatDivisor = 0;
    std::uint32_t frameCounter = 0;
    std::uint32_t totalData = 0;
    AVCodecContext* audioCodecCtx;
    audioResampler* planarResampler = 0;
    SwrContext* plotResCtx = 0;
    conPlot* plotter = 0;
    bool plot;
    std::uint64_t totalSamples;
    AVSampleFormat inputSampleFormat, outputSampleFormat;
    stdClock::time_point beginClock, endClock;
    lockedQ<AVFrame*>& frameSource;
};

#endif /*AUDIOPLAYER_H*/
