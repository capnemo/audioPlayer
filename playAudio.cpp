#include <iostream>
#include "streamInit.h"
#include "reader.h"
#include "audioPlayer.h"

/* 
 * Entry point for the program. Takes in a file name and 
 * writes to the audio stream to the default device.
*/

void usage(const char* progName);

int main(int argc, char *argv[])
{
    if ((argc < 2) || (argc > 3)) {
        usage(argv[0]);
        return -1;
    }

    bool plot = false;
    std::vector<std::string> argVec;
    for (int i = 1; i < argc; i++) 
        argVec.push_back(argv[i]);
    
    int fileNameIndex = -1;
    if (argVec.size() == 1) {
        if (argVec[0] == "-plot")
            fileNameIndex = -1;
        else
            fileNameIndex = 0;
    } else if (argVec.size() == 2) {
        plot = true;
        if (argVec[0] == "-plot")
            fileNameIndex = 1;
        else if (argVec[1] == "-plot")
            fileNameIndex = 0;
    }
        
    if (fileNameIndex == -1) {
        usage(argv[0]);
        return -1;
    }

    streamInit avStr(argVec[fileNameIndex].c_str());
    if (avStr.init() != 0) {
        std::cout << "Error initializing.." << std::endl;
        return -1;
    }

    avStr.dump();
    AVFormatContext *formatCtx = avStr.getFormatContext();
    AVCodecContext* audioContext = avStr.getCodecContext();
    int audioIndex = avStr.getAudioStreamIndex();
    int64_t totalSamples = avStr.getNumSamplesInStream();
    lockedQ<AVFrame*> frameQ("frame");

    reader rt(frameQ, audioIndex, formatCtx, audioContext);
    audioPlayer at(audioContext, totalSamples, frameQ, plot);

    if (at.init() != 0) {
        std::cout << "Error initializing audio player" << std::endl;
        return -1;
    }

    rt.startThread();
    at.startThread();

    at.joinThread();
    rt.joinThread();

    frameQ.printStats();
}

void usage(const char* progName)
{
    std::cout << "Usage: " << progName << " <fileName> [-plot]" 
    << std::endl;
}
