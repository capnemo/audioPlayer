
#include <iostream>
#include "streamInit.h"
#include "reader.h"
//#include ffmpeg includes.

/*Reader.cpp has memory leaks according to valgrind*/
/* To Be investigated. */

bool initializeOutputContexts(const char* outFile,
                              const AVFormatContext* inputFormatContext,
                              const AVCodecContext* inputCodecContext,
                              std::uint32_t inputAudioIndex,
                              AVFormatContext** outputFormatContext,
                              AVCodecContext** outputCodecContext);

void encodeOutputFile(lockedQ<AVFrame*>& frQ, AVCodecContext* codecCtx,
                      AVFormatContext* fmtCtx);

void generateOutputFileName(const std::string& inFile, 
                            std::string& outFile);
void compareStreamStats(streamInit& str1, streamInit& str2);
void usage(const char* progName);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return -1;
    }
    
    std::string fileName = argv[1];

    streamInit inputStream(fileName.c_str());
    if (inputStream.init() == false) {
        std::cout << "Error initializing.." << std::endl;
        return -1;
    }

    std::string outFile = "out." + fileName;
    generateOutputFileName(fileName, outFile);
    if (outFile == "") {
        std::cout << "Input file is invalid" << std::endl;
        return -1;
    }

    AVFormatContext *formatContext = inputStream.getFormatContext();
    AVCodecContext* audioContext = inputStream.getCodecContext();
    std::uint32_t audioIndex = inputStream.getAudioStreamIndex();

    AVFormatContext* outputFormatCtx;
    AVCodecContext* outputCodecCtx;
    if (initializeOutputContexts(outFile.c_str(), formatContext, audioContext, 
                  audioIndex, &outputFormatCtx, &outputCodecCtx) == false) {
        std::cout << "Error initializing output contexts.." << std::endl;   
        return -1;
    }

    if (avformat_write_header(outputFormatCtx, nullptr) < 0) {
        std::cout << "Cannot write header" << std::endl;
        avio_closep(&outputFormatCtx->pb);
        avformat_free_context(outputFormatCtx);
        return -1;
    }

    lockedQ<AVFrame*> frameQ("frame");
    reader rt(frameQ, audioIndex, formatContext, audioContext);
    rt.startThread();
    rt.joinThread();
    encodeOutputFile(frameQ, outputCodecCtx, outputFormatCtx);

    avio_closep(&outputFormatCtx->pb);
    avformat_free_context(outputFormatCtx);

    streamInit outputStream(outFile.c_str());
    if (outputStream.init() == false) {
        std::cout << "Error opening output file" << std::endl;
        return -1;
    }

    compareStreamStats(inputStream, outputStream);
}
 
void generateOutputFileName(const std::string& inFile, std::string& outFile)
{
    outFile = "";
    size_t exPos = inFile.rfind('.');
    if (exPos == std::string::npos)
        return;

    if (inFile.substr(exPos, inFile.size()) != ".wav") 
        return;
    
    outFile = inFile.substr(0, exPos) + ".out.wav";
}

void encodeOutputFile(lockedQ<AVFrame*>& frQ, AVCodecContext* codecCtx,
                      AVFormatContext* fmtCtx)
{
    while (frQ.terminateOutput() == false) {
        AVFrame* fr = frQ.deQueue();
        int frameRC;
        if ((frameRC = avcodec_send_frame(codecCtx, fr)) == 0) {
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
            while (avcodec_receive_packet(codecCtx, &pkt) == 0) {
                int rc = av_write_frame(fmtCtx, &pkt);
                if (rc < 0) 
                    std::cout << "Error writing frame" << std::endl;
                av_write_frame(fmtCtx, nullptr);
            }
        }
        if (frameRC != 0) {
            char errBuf[32];
            av_strerror(frameRC, errBuf, 32);
            std::cout << "Frame RC " << errBuf << std::endl;
        }
        av_frame_unref(fr);
    }
}

void compareStreamStats(streamInit& str1, streamInit& str2)
{
    str1.dump();
    str2.dump();
    
    std::string sampleFormat1;
    str1.getSampleFormat(sampleFormat1);
    std::string sampleFormat2;
    str2.getSampleFormat(sampleFormat2);
    
    if ((str1.getNumChannels() == str2.getNumChannels()) &&
        (str1.getSamplingRate() == str2.getSamplingRate()) &&
        (sampleFormat1 == sampleFormat2) &&
        (str1.getNumSamplesInStream() == str2.getNumSamplesInStream()))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed" << std::endl;
}

bool initializeOutputContexts(const char* outFile,
                              const AVFormatContext* inputFormatContext,
                              const AVCodecContext* inputCodecContext,
                              std::uint32_t inputAudioIndex,
                              AVFormatContext** outputFormatContext,
                              AVCodecContext** outputCodecContext)
{
    *outputFormatContext = nullptr;
    AVIOContext *outputIOContext = nullptr;
    if (avio_open(&outputIOContext, outFile, AVIO_FLAG_WRITE) < 0) {
        std::cout << "Error opening output file" << std::endl;
        return false;
    }

    if ((*outputFormatContext = avformat_alloc_context()) == 0) {
        std::cout << "Error allocating output context" << std::endl;
        return false;
    }
    (*outputFormatContext)->pb = outputIOContext;

    AVOutputFormat* ofor = av_guess_format(nullptr, outFile, nullptr);
    if (ofor != nullptr) {
        (*outputFormatContext)->oformat = ofor;
    } else {
        std::cout << "Error getting the output file format" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }

    char* fName = av_strdup(outFile);
    if (fName == nullptr) {
        std::cout << "Error allocating url" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }
    (*outputFormatContext)->url = fName;

    AVCodec* outputCodec = avcodec_find_encoder(inputCodecContext->codec_id);
    if (outputCodec == nullptr) {
        std::cout << "Cannot find the output encoder" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }
    
    AVStream* outStream = avformat_new_stream(*outputFormatContext, nullptr);
    if (outStream == nullptr) {
        std::cout << "Cannot get a new output stream" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }

    outStream->time_base = 
                     inputFormatContext->streams[inputAudioIndex]->time_base;
    outStream->duration = 
                     inputFormatContext->streams[inputAudioIndex]->duration;
    
    *outputCodecContext = avcodec_alloc_context3(outputCodec);
    if (*outputCodecContext == nullptr) {
        std::cout << "Error allocating codec context" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }
    
    (*outputCodecContext)->channels = inputCodecContext->channels;
    (*outputCodecContext)->channel_layout = inputCodecContext->channel_layout;
    (*outputCodecContext)->sample_rate = inputCodecContext->sample_rate;
    (*outputCodecContext)->sample_fmt = inputCodecContext->sample_fmt;
    (*outputCodecContext)->bit_rate = inputCodecContext->bit_rate;

    if (avcodec_open2((*outputCodecContext), outputCodec, nullptr) < 0) {
        std::cout << "Error opening output codec" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }
    
    if (avcodec_parameters_from_context(outStream->codecpar, 
                                         (*outputCodecContext)) < 0) {
        std::cout << "Error initializing stream parameters" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        return false;
    }

    return true;
}

void usage(const char* progName)
{
    std::cout << "Usage: " << progName << " <WAV file> "<< std::endl;
    std::cout << "Argument should be a .wav file" << std::endl;
}

