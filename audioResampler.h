extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}

#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

class audioResampler {
    public:
    audioResampler(const AVCodecContext* cdcCtx, 
                   AVSampleFormat inFmt, AVSampleFormat outFmt):
                   codecContext(cdcCtx), inputFormat(inFmt), 
                   outputFormat(outFmt) {}

    audioResampler(const audioResampler&) = delete;
    audioResampler& operator = (const audioResampler&) = delete;

    bool init();
    bool resampleFrame(const AVFrame* inputFrame, AVFrame *outputFrame);
    void* resampleData(const AVFrame* inputFrame);
    bool flushable();
    ~audioResampler();

    private:
    const AVCodecContext* codecContext;
    AVSampleFormat inputFormat, outputFormat;
    SwrContext* resampleContext = 0;
    int channelLayout;
};

#endif /* AUDIORESAMPLER_H */
