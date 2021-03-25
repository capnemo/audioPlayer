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
    if ((argc < 2) || (argc > 4)) {
        usage(argv[0]);
        return -1;
    }
    
    std::string fileName = "";
    std::string plotArg = "-plot";
    std::string dumpArg = "-dump";
    bool plot = false;
    bool dump = false;
    int argsProcessed = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i] == plotArg)  {
            plot = true;
            argsProcessed++;
        } else if (argv[i] == dumpArg) {
            dump = true;
            argsProcessed++;
        } else {
            if (fileName == "") {
                fileName = argv[i];
                argsProcessed++;
            }
        }
    }
    
    if ((fileName == "") || (argsProcessed != argc - 1)) {
        usage(argv[0]);
        return -1;
    }

    streamInit avStr(fileName.c_str());
    if (avStr.init() == false) {
        std::cout << "Error initializing.." << std::endl;
        return -1;
    }

    if (dump == true) {
        avStr.dump();
        return 0;
    }

    AVFormatContext *formatCtx = avStr.getFormatContext();
    AVCodecContext* audioContext = avStr.getCodecContext();
    std::uint32_t audioIndex = avStr.getAudioStreamIndex();
    lockedQ<AVFrame*> frameQ("frame");

    reader rt(frameQ, audioIndex, formatCtx, audioContext);
    audioPlayer at(audioContext, avStr.getNumSamplesInStream(), 
                   avStr.getSamplingRate(), frameQ, plot);

    if (at.init() == false) {
        std::cout << "Error initializing audio player" << std::endl;
        return -1;
    }

    rt.startThread();
    at.startThread();

    at.joinThread();
    rt.joinThread();

    frameQ.printStats();
    return 0;
}

void usage(const char* progName)
{
    std::cout << "Usage: " << progName << " <fileName> " << "[-dump] "
              << "[-plot]" << std::endl;
}
