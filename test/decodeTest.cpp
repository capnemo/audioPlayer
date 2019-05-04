
#include <iostream>
#include "streamInit.h"
#include "reader.h"
//#include ffmpeg includes.


/* 
 * Entry point for the program. Takes in a file name and 
 * writes to the audio stream to the default device.
*/

bool initializeOutputContexts(const char* outFile,
                              const AVFormatContext* inputFormatContext,
                              const AVCodecContext* inputCodecContext,
                              std::uint32_t inputAudioIndex,
                              AVFormatContext** outputFormatContext,
                              AVCodecContext** outputCodecContext);
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

    AVFormatContext *formatContext = inputStream.getFormatContext();
    AVCodecContext* audioContext = inputStream.getCodecContext();
    std::uint32_t audioIndex = inputStream.getAudioStreamIndex();
    std::string outFile = "out." + fileName;
    AVFormatContext* outputFormatCtx;
    AVCodecContext* outputCodecCtx;
    if (initializeOutputContexts(outFile.c_str(), formatContext, audioContext, 
                  audioIndex, &outputFormatCtx, &outputCodecCtx) == false) {
        std::cout << "Error initializing output contexts.." << std::endl;   
        //CLOSE
        return -1;
    }

    if (avformat_write_header(outputFormatCtx, nullptr) < 0) {
        std::cout << "Cannot write header" << std::endl;
        //CLOSE
        return -1;
    }

    lockedQ<AVFrame*> frameQ("frame");
    reader rt(frameQ, audioIndex, formatContext, audioContext);
    rt.startThread();
    rt.joinThread();
    
    int serial = 0;
    while (frameQ.terminateOutput() == false) {
        AVFrame* fr = frameQ.deQueue();
        int frameRC;
        if ((frameRC = avcodec_send_frame(outputCodecCtx, fr)) == 0) {
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
            while (avcodec_receive_packet(outputCodecCtx, &pkt) == 0) {
                int rc = av_write_frame(outputFormatCtx, &pkt);
                if (rc < 0) 
                    std::cout << "Error writing frame" << std::endl;
                av_write_frame(outputFormatCtx, nullptr);
            }
        }
        if (frameRC != 0) {
            char errBuf[32];
            av_strerror(frameRC, errBuf, 32);
            std::cout << "Frame RC " << errBuf << std::endl;
        }
        av_frame_unref(fr);
    }

    avio_closep(&outputFormatCtx->pb);
    avformat_free_context(outputFormatCtx);

    streamInit outputStream(outFile.c_str());
    if (outputStream.init() == false) {
        std::cout << "Error opening output file" << std::endl;
        return -1;
    }

    inputStream.dump();
    outputStream.dump();
    std::string inputSampleFormat;
    inputStream.getSampleFormat(inputSampleFormat);
    std::string outputSampleFormat;
    outputStream.getSampleFormat(outputSampleFormat);

    if ((inputStream.getNumChannels() == outputStream.getNumChannels()) &&
        (inputStream.getSamplingRate() == inputStream.getSamplingRate()) &&
        (inputSampleFormat == outputSampleFormat) &&
        (inputStream.getNumSamplesInStream() == 
                                    outputStream.getNumSamplesInStream()))
            std::cout << "Test passed" << std::endl;
        else
            std::cout << "Test failed" << std::endl;
}
    

void usage(const char* progName)
{
    std::cout << "Usage: " << progName << " <fileName> "<< std::endl;
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
/*
    outStream->time_base = inputCodecContext->time_base;
    outStream->duration = 
*/
    
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
        //AVCODEC CLOSE
        return false;
    }
    
    if (avcodec_parameters_from_context(outStream->codecpar, 
                                         (*outputCodecContext)) < 0) {
        std::cout << "Error initializing stream parameters" << std::endl;
        avio_closep(&(*outputFormatContext)->pb);
        avformat_free_context(*outputFormatContext);
        //AVCODEC CLOSE
        return false;
    }

    return true;
}
