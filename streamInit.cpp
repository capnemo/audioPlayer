#include <iostream>
#include "streamInit.h"

/*
   Initializes the audio stream from the input file and gets it ready
   to be read. 
   Initializes the formatContext and the audio codec for use
   by the decoder. 
*/

bool streamInit::init()
{
    if (avformat_open_input(&fmtCtx, inputFile, 0, 0) != 0) {
        std::cout << "Error opening file" << std::endl;
        return false;
    }

    if (avformat_find_stream_info(fmtCtx, 0) < 0) {
        std::cout << "Error getting header info." << std::endl;
        return false;
    }

    for (int i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioTimeBase = fmtCtx->streams[i]->time_base;
            audioIndex = i;
            break;
        }
    }

    if (audioIndex == -1) {
        std::cout << "Error finding the audio stream" << std::endl;
        return false;
    }

    audioStream = fmtCtx->streams[audioIndex];
    codecPar = fmtCtx->streams[audioIndex]->codecpar;
    audioCodec = avcodec_find_decoder(codecPar->codec_id);
    if (audioCodec == 0) {
        std::cout << "Error finding the audio codec" << std::endl;
        return false;
    }

    samplingRate = codecPar->sample_rate;
    totalSamples = (fmtCtx->streams[audioIndex]->duration * audioTimeBase.num * 
                    samplingRate) / audioTimeBase.den;
    
    cdcCtx = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(cdcCtx, codecPar);
    
    if (avcodec_open2(cdcCtx, audioCodec, 0) < 0) {
        avcodec_free_context(&cdcCtx);
        cdcCtx = 0; 
        std::cout << "Error opening the audio codec" << std::endl;
        return false;
    }
    
    return true;
}

/* 
 *  Print out audio stream stats. Maybe in another function.
 */

void streamInit::dump() const
{
    std::cout << "File name: " << inputFile << std::endl;
    std::cout << "No. of streams in clip: " << fmtCtx->nb_streams << std::endl;
    std::cout << "Number of channels in the audio stream: " 
              << cdcCtx->channels << std::endl;
    std::cout << "Sampling Rate: " << samplingRate << std::endl;
    std::cout << "Samples per channel: " << totalSamples << std::endl;
}

/*
 *  returns the formatContext for the file.
 */
AVFormatContext* streamInit::getFormatContext() const
{
    return fmtCtx;
}

/*
 *  returns the index of the audio stream from the demuxer.
 */
std::uint32_t streamInit::getAudioStreamIndex() const
{
    return audioIndex;
}

/* 
 *  returns the codec for the audio stream
 */
AVCodec* streamInit::getCodec() const
{
    return audioCodec;
}

/*
 *  returns the audio codec context.
 */
AVCodecContext* streamInit::getCodecContext() const
{
    return cdcCtx;
}

/* 
 *  returns the total number of samples in the audio clip
 */
std::uint64_t streamInit::getNumSamplesInStream() const
{
    return totalSamples;
}

/* 
 *  returns the audio time base. (inverse of the sampling frequency)
 */
AVRational streamInit::getAudioTimeBase() const
{ 
    return audioTimeBase; 
}

std::uint32_t streamInit::getSamplingRate() const
{
    return samplingRate;
}

/*
 *  Destructor
 */
streamInit::~streamInit()
{   
    avcodec_close(cdcCtx);
    avcodec_free_context(&cdcCtx);
    avformat_close_input(&fmtCtx);
}
