
/* 
 * Class to write a decoded audio stream to the audio device.
*/


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
}


#include <alsa/asoundlib.h>
#include "threadRunner.h"
#include "lockedQ.h"
#include <chrono>

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

class audioPlayer:public threadRunner {
    public:
    audioPlayer(AVCodecContext* cdcCtx, lockedQ<AVFrame*>& fSrc):
                audioCodecCtx(cdcCtx), frameSource(fSrc) {}

    int init();
    void threadFunc();
    ~audioPlayer();

    private:
    std::thread audioThread;
    snd_pcm_t *playbackHandle;
    uint32_t formatDivisor = 0;
    uint32_t frameCounter = 0;
    uint32_t totalData = 0;
    AVCodecContext* audioCodecCtx;
    lockedQ<AVFrame*>& frameSource;
    bool interleaved = true;
    SwrContext* resCtx = 0;
    std::chrono::high_resolution_clock::time_point beginClock, endClock;
};

#endif /*AUDIOPLAYER_H*/
