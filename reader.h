/* 
    Class to read and decode the audio stream
*/


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include "lockedQ.h"
#include "threadRunner.h"
#include <thread>

#ifndef READER_H
#define READER_H

class reader:public threadRunner {
    public:
    reader(lockedQ<AVFrame*>& pQ, int aIn, AVFormatContext* fCtx,
           AVCodecContext* cCtx):
           outQ(pQ), audioIndex(aIn), fmtCtx(fCtx), cdcCtx(cCtx) {}

    virtual void threadFunc();
    private:
    void decode(AVPacket* packet);

    private:
    lockedQ<AVFrame*>& outQ;
    int audioIndex = -1;
    AVFormatContext* fmtCtx = 0;
    AVCodecContext*  cdcCtx = 0;
};
#endif /*READER_H*/
