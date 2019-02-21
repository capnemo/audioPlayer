#include <iostream>
#include "streamInit.h"


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

void streamInit::dump()
{
    av_dump_format(fmtCtx, audioIndex, inputFile, 0);
}

AVFormatContext* streamInit::getFormatContext()
{
    return fmtCtx;
}

int streamInit::getAudioStreamIndex() 
{
    return audioIndex;
}

AVCodec* streamInit::getCodec() 
{
    return audioCodec;
}

AVCodecContext* streamInit::getCodecContext() 
{
    return cdcCtx;
}

AVRational* streamInit::getAudioTimeBase()  //Should not be a pointer!
{ 
    return &audioTimeBase; 
}

streamInit::~streamInit()
{   
    avcodec_close(cdcCtx);
    avcodec_free_context(&cdcCtx);
    avformat_close_input(&fmtCtx);
}
