extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}


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
    ~audioResampler();

    private:
    const AVCodecContext* codecContext;
    AVSampleFormat inputFormat, outputFormat;
    SwrContext* resampleContext = 0;
};

