/* 
   Class to initialize FFmpeg data structures for use by the reader
   and decoder.
*/



extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifndef STREAMINIT_H
#define STREAMINIT_H

class streamInit {
    public:
    streamInit(const char* fileName):inputFile(fileName)  {}
    streamInit(const streamInit&) = delete;
    streamInit& operator = (streamInit&) = delete;
    int init();
    void dump();
    AVFormatContext* getFormatContext();
    int getAudioStreamIndex();
    AVCodec* getCodec();
    AVCodecContext* getCodecContext();
    AVRational getAudioTimeBase(); 
    int64_t getNumSamplesInStream();
    int getSamplingRate();
    ~streamInit();

    private:
    const char* inputFile;
    AVFormatContext* fmtCtx = 0;
    int audioIndex = -1;
    int samplingRate;
    AVCodecParameters* codecPar = 0;
    AVCodec* audioCodec = 0;
    AVCodecContext* cdcCtx = 0;
    AVStream* audioStream;
    AVRational audioTimeBase;
    //int64_t clipLength;
};
#endif /*STREAMINIT_H*/
