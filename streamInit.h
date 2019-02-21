extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifndef STREAMINIT_H
#define STREAMINIT_H

class streamInit {
    public:
    streamInit(const char* fileName):inputFile(fileName)  {}
    int init();
    void dump();

    AVFormatContext* getFormatContext();
    int getAudioStreamIndex();
    AVCodec* getCodec();
    AVCodecContext* getCodecContext();
    AVRational* getAudioTimeBase(); 
    ~streamInit();

    private:
    const char* inputFile;
    AVFormatContext* fmtCtx = 0;
    int audioIndex = -1;
    AVCodecParameters* codecPar = 0;
    AVCodec* audioCodec = 0;
    AVCodecContext* cdcCtx = 0;
    AVRational audioTimeBase;

};
#endif /*STREAMINIT_H*/
