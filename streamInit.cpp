#include <iostream>
#include "streamInit.h"

/*
   Initializes the audio stream from the input file and gets it ready
   to be read. 
   Initializes the formatContext and the audio codec for use
   by the decoder. 
*/

int streamInit::init()
{
    if (avformat_open_input(&fmtCtx, inputFile, 0, 0) != 0) 
        return -1;

    if (avformat_find_stream_info(fmtCtx, 0) < 0) 
        return -2;

    for (int i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioTimeBase = fmtCtx->streams[i]->time_base;
            audioIndex = i;
            break;
        }
    }

    if (audioIndex == -1)
        return -3;

    codecPar = fmtCtx->streams[audioIndex]->codecpar;
    audioCodec = avcodec_find_decoder(codecPar->codec_id);
    if (audioCodec == 0)
        return -4;

    cdcCtx = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(cdcCtx, codecPar);
    
    if (avcodec_open2(cdcCtx, audioCodec, 0) < 0) {
        avcodec_free_context(&cdcCtx);
        cdcCtx = 0; 
        return -5;
    }
    
    return 0;
}

/* 
    Writes out the stats of all the streams in the input file.
*/
void streamInit::dump()
{
    av_dump_format(fmtCtx, audioIndex, inputFile, 0);
}

/*
   returns the formatContext for the file.
*/
AVFormatContext* streamInit::getFormatContext()
{
    return fmtCtx;
}

/*
    returns the index of the audio stream from the demuxer.
*/
int streamInit::getAudioStreamIndex() 
{
    return audioIndex;
}

/* 
    returns the codec for the audio stream
*/
AVCodec* streamInit::getCodec() 
{
    return audioCodec;
}

/*
    returns the audio codec context.
*/
AVCodecContext* streamInit::getCodecContext() 
{
    return cdcCtx;
}

/* 
    returns the audio time base. (inverse of the sampling frequency)
*/
AVRational streamInit::getAudioTimeBase()  //Should not be a pointer!
{ 
    return &audioTimeBase; 
}

/*
    Destructor
*/
streamInit::~streamInit()
{   
    avcodec_close(cdcCtx);
    avcodec_free_context(&cdcCtx);
    avformat_close_input(&fmtCtx);
}
