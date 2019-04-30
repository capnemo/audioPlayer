/* 
    Class to read and decode the audio stream
*/


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include <thread>
#include <cstdint>
#include "lockedQ.h"
#include "threadRunner.h"

#ifndef READER_H
#define READER_H

class reader:public threadRunner {
    public:
    reader(lockedQ<AVFrame*>& pQ, std::uint32_t aIn, AVFormatContext* fCtx,
           AVCodecContext* cCtx):
           outQ(pQ), audioIndex(aIn), fmtCtx(fCtx), cdcCtx(cCtx) {}
    reader(const reader&) = delete;
    reader& operator = (reader&) = delete;

    virtual void threadFunc();

    private:
    void decode(AVPacket* packet);

    private:
    lockedQ<AVFrame*>& outQ;
    std::int32_t audioIndex = -1;
    AVFormatContext* fmtCtx = 0;
    AVCodecContext*  cdcCtx = 0;
};
#endif /*READER_H*/
