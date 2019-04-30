#include <iostream>
#include "reader.h"
/*
 * Thread function to read and decode the stream. 
 */
void reader::threadFunc()
{
    int pRc = 0;
    while(pRc == 0) {
        AVPacket* pkt = av_packet_alloc();
        pRc = av_read_frame(fmtCtx, pkt);
        if ((pRc == 0) && (pkt->stream_index == audioIndex)) 
            decode(pkt);
        av_packet_free(&pkt);
    }
    outQ.inputComplete();
}

/* 
 * Decodes and writes a frame to the queue. Called from threadFunc
 */
void reader::decode(AVPacket* packet)
{
    int fRc = avcodec_send_packet(cdcCtx, packet);
    if (fRc == 0) {
        AVFrame* frame = av_frame_alloc();
        int dRc;
        while ((dRc = avcodec_receive_frame(cdcCtx, frame)) >= 0) {
            outQ.enQueue(frame);
            frame = av_frame_alloc();
        }
        av_frame_free(&frame);
    }
}
    
