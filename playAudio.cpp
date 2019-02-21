#include <iostream>

#include "streamInit.h"
#include "reader.h"
#include "audioPlayer.h"

//TODO make sure all AV* data structures are released.
//     Make sure all the destructors are active.

//void audio_callback(void *opaque, Uint8 *stream, int len);
//One of the AV* structures is NOT thread safe!!
int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <fileName>" << std::endl;
        return -1;
    }

    streamInit avStr(argv[1]);   
    if (avStr.init() != 0) {
        std::cout << "Error initializing.." << std::endl;
        return -1;
    }
    avStr.dump();
    AVFormatContext *formatCtx = avStr.getFormatContext();
    AVCodecContext* audioContext = avStr.getCodecContext();
    int audioIndex = avStr.getAudioStreamIndex();
    lockedQ<AVFrame*> frameQ("frame");

    reader rt(frameQ, audioIndex, formatCtx, audioContext);
    audioPlayer at(audioContext, frameQ);

    if (at.init() != 0) {
        std::cout << "Error initializing audio player" << std::endl;
        return -1;
    }

    rt.startThread();
    at.startThread();

    at.joinThread();
    rt.joinThread();

    frameQ.printStats();
/*
*/
}

