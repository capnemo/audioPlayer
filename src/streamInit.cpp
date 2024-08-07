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

    for (uint32_t i = 0; i < fmtCtx->nb_streams; i++) {
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
    audioCodec = (AVCodec *)avcodec_find_decoder(codecPar->codec_id);
    if (audioCodec == 0) {
        std::cout << "Error finding the audio codec" << std::endl;
        return false;
    }

    samplingRate = codecPar->sample_rate;
    totalSamples = (fmtCtx->streams[audioIndex]->duration * 
                    audioTimeBase.num * samplingRate) / audioTimeBase.den;
    
    cdcCtx = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(cdcCtx, codecPar);
    extractSampleFormat();
    
    if (avcodec_open2(cdcCtx, audioCodec, 0) < 0) {
        avcodec_free_context(&cdcCtx);
        cdcCtx = 0; 
        std::cout << "Error opening the audio codec" << std::endl;
        return false;
    }
    
    numChannels = cdcCtx->ch_layout.nb_channels;
    //numChannels = cdcCtx->channels;
    numStreams = fmtCtx->nb_streams;

    return true;
}

/* 
 *  Print out audio stream stats. Maybe in another function.
 */

void streamInit::dump() const
{
    std::cout << "File name: " << inputFile << std::endl;
    std::cout << "No. of streams in clip: " << numStreams << std::endl;
    std::cout << "Number of channels in the audio stream: " 
              << numChannels << std::endl;
    std::cout << "Sampling Rate: " << samplingRate << std::endl;
    std::cout << "Samples per channel: " << totalSamples << std::endl;
    std::cout << "Sample Format: " << smpFmt << std::endl;
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
uint32_t streamInit::getAudioStreamIndex() const
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
uint64_t streamInit::getNumSamplesInStream() const
{
    return totalSamples;
}

/* 
 *  returns the audio time base. 
 */
AVRational streamInit::getAudioTimeBase() const
{ 
    return audioTimeBase; 
}

/*
 * returns the sampling rate of the audio stream.
 */
uint32_t streamInit::getSamplingRate() const
{
    return samplingRate;
}

/*
 *  returns the number of channnels in the audio stream.
 */
uint32_t streamInit::getNumChannels() const
{
    return numChannels;
}

/*
 *  returns the number of streams in the input clip.
 */
uint32_t streamInit::getNumStreams() const
{
    return numStreams;
}

/*
 *  returns the sample format of the audio stream.
 */
void streamInit::getSampleFormat(std::string& format) const
{
    format = smpFmt;
}

/*
 * initializes the sample format from the codec context.
 */
void streamInit::extractSampleFormat()
{

    switch (cdcCtx->sample_fmt) {
        case AV_SAMPLE_FMT_NONE :
            smpFmt = "Unknown";
            break;
        case AV_SAMPLE_FMT_U8:
            smpFmt = "Unsigned 8 bit packed";
            break;
        case AV_SAMPLE_FMT_U8P:
            smpFmt = "Unsigned 8 bit planar";
            break;
        case AV_SAMPLE_FMT_S16:
            smpFmt = "Signed 16 bit packed";
            break;
        case AV_SAMPLE_FMT_S16P:
            smpFmt = "Signed 16 bit planar";
            break;
        case AV_SAMPLE_FMT_S32:
            smpFmt = "Signed 32 bit packed";
            break;
        case AV_SAMPLE_FMT_S32P:
            smpFmt = "Signed 32 bit planar";
            break;
        case AV_SAMPLE_FMT_FLT:
            smpFmt = "Floating point packed";
            break;
        case AV_SAMPLE_FMT_FLTP:
            smpFmt = "Floating point planar";
            break;
        case AV_SAMPLE_FMT_DBL:
            smpFmt = "Double precision packed";
            break;
        case AV_SAMPLE_FMT_DBLP:
            smpFmt = "Double precision planar";
            break;
        case AV_SAMPLE_FMT_S64:
            smpFmt = "Signed 64 bit packed";
            break;
        case AV_SAMPLE_FMT_S64P:
            smpFmt = "Signed 64 bit planar";
            break;
        default:
            smpFmt = "Unknown";
            break;
    }
}


/*
 *  Destructor
 */
streamInit::~streamInit()
{   
    //avcodec_close(cdcCtx);
    avcodec_free_context(&cdcCtx);
    avformat_close_input(&fmtCtx);
}
