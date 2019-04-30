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

    bool init();
    void dump() const;
    AVFormatContext* getFormatContext() const;
    std::uint32_t getAudioStreamIndex() const;
    AVCodec* getCodec() const;
    AVCodecContext* getCodecContext() const;
    AVRational getAudioTimeBase() const; 
    std::uint64_t getNumSamplesInStream() const;
    std::uint32_t getSamplingRate() const;
    ~streamInit();

    private:
    void getSampleFormat(std::string& format) const;

    private:
    const char* inputFile;
    AVFormatContext* fmtCtx = 0;
    std::uint32_t audioIndex = -1;
    std::uint32_t samplingRate;
    std::uint64_t totalSamples = 0;
    AVCodecParameters* codecPar = 0;
    AVCodec* audioCodec = 0;
    AVCodecContext* cdcCtx = 0;
    AVStream* audioStream;
    AVRational audioTimeBase;
};
#endif /*STREAMINIT_H*/
