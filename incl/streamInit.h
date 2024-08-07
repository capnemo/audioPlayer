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
    uint32_t getAudioStreamIndex() const;
    AVCodec* getCodec() const;
    AVCodecContext* getCodecContext() const;
    AVRational getAudioTimeBase() const; 
    uint64_t getNumSamplesInStream() const;
    uint32_t getSamplingRate() const;
    uint32_t getNumChannels() const;
    uint32_t getNumStreams() const;
    void getSampleFormat(std::string& format) const;
    ~streamInit();

    private:
    void extractSampleFormat();

    private:
    const char* inputFile;
    AVFormatContext* fmtCtx = 0;
    int32_t audioIndex = -1;
    uint32_t samplingRate;
    uint64_t totalSamples = 0;
    uint32_t numChannels = 0;
    uint32_t numStreams = 0;
    std::string smpFmt = "";
    AVCodecParameters* codecPar = 0;
    AVCodec* audioCodec = 0;
    AVCodecContext* cdcCtx = 0;
    AVStream* audioStream;
    AVRational audioTimeBase;
};
#endif /*STREAMINIT_H*/
