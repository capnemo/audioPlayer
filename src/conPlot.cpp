#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include "conPlot.h"

/*
 * Initialize the conPlot object. Sets up the connection to the XServer and 
 * starts the graphing window. 
 */
bool conPlot::init()
{
    if (initX() == false)
        return false;

    if (initResampler() == false)
        return false;
    
    setColors();
    drawWindow(0);
    initializeWindowData();
    
    return true;
}

/* 
 *  Initialize and start the resampler if required.
 */
bool conPlot::initResampler() 
{
    bool rc = true;

    if (inputFormat != plotFormat) {
        resampler = new audioResampler(codecCtx, inputFormat, plotFormat);
        if (resampler == nullptr)  {
            rc = false;
        } else if (resampler->init() == false) {
            delete resampler;
            resampler = nullptr;
            rc = false;
        }
    }

    if (rc == false)
        std::cout << "Error allocating plotter" << std::endl;

    return rc;
}

/*
 *  Fill the color array.
 */
void conPlot::setColors()
{
    screen = DefaultScreen(display);
    Colormap screenColorMap = DefaultColormap(display, screen);
    std::vector<std::string> colors = {"red","blue","green","yellow",
                                       "white"};

    for (auto col:colors) {
        XColor sCol;
        int rc = XAllocNamedColor(display, screenColorMap, col.c_str(),
                                  &sCol, &sCol);
        if (rc != 0)
            lineColors.push_back(sCol.pixel);
    }
}

/*
 * Initializing graphing window data.
 */ 
void conPlot::initializeWindowData()
{
    xAxisBegin = 10;
    xAxisEnd = windowWidth - 20;
    yAxisBegin = 10;
    yAxisEnd = windowHeight - 20;
    yRange = yAxisEnd - yAxisBegin;
    xRange = xAxisEnd - xAxisBegin;
    for (std::uint32_t i = 0; i < numChannels; i++)  {
        planarData.push_back(std::vector<short>());
        points.push_back(std::vector<short>());
        xCoord.push_back(xAxisBegin);
        yCoord.push_back(yRange/2);
    }
    currentXPos = xAxisBegin;
    samplesPerPoint = (totalSamples/(double)xRange) + 0.5;
    dataRange = std::numeric_limits<short>::max() 
                - std::numeric_limits<short>::min();
    drawAxes();
}

/*  
 *  Draws the X and Y axes on the graphing window.
 *
 */
void conPlot::drawAxes() 
{
    drawWhiteLine(xAxisBegin, yAxisBegin, xAxisBegin, yAxisEnd);
    drawWhiteLine(xAxisBegin, yAxisEnd, xAxisEnd, yAxisEnd);

    int interval = 100;
    for (std::uint32_t i = xAxisBegin + interval; i < xAxisEnd; 
                                                    i += interval) 
        drawWhiteLine(i, yAxisEnd, i, yAxisEnd + 5); 
        
    XFlush(display);
}

/*
 * Draws a line from the previous point to the current one.
 * IN channel is the channel number.
 * IN currentY is the data point. 
 */
void conPlot::plotLine(std::uint32_t channel, std::int32_t currentY)
{
    drawColoredLine(xCoord[channel], yCoord[channel], 
                    xCoord[channel] + 1, currentY, channel);

    xCoord[channel]++;
    yCoord[channel] = currentY;
    XFlush(display);
}

/* 
 * Class entry point. Plots the data in a frame. Calls the resampler 
 * if necessary.
 * IN: inFrame. Audio frame. Data to be graphed is in AVFrame->data
 */

void conPlot::plotData(const AVFrame* inFrame)
{
    if (resampler != nullptr) {
        AVFrame plotFrame;
        bzero((void *)&plotFrame, sizeof(AVFrame));
        if (resampler->resampleFrame(inFrame, &plotFrame) == true)  {
            appendData(&plotFrame);
            av_frame_unref(&plotFrame);
            while (resampler->flushable() == true) {
                bzero((void *)&plotFrame, sizeof(AVFrame));
                resampler->resampleFrame(nullptr, &plotFrame);
                appendData(&plotFrame);
                av_frame_unref(&plotFrame);
            }
        } else  {
            std::cout << "Plot resample fail" << std::endl;
            return;
        }
    } else {
        appendData(inFrame);
    }
}

/*
 * Adds data from inFrame to the graphing data structures.
 * IN inFrame. Frame to be graphed.
 */
void conPlot::appendData(const AVFrame* inFrame)
{
    short* dataPtr = (short *)(inFrame->data[0]);
    for (std::uint32_t i = 0; i < inFrame->nb_samples * numChannels;
                                                            i++) {
        planarData[i%numChannels].push_back(dataPtr[i]);
        if (planarData[i%numChannels].size() == samplesPerPoint) {
            int avg = maxAggregate(planarData[i%numChannels]);
            int y = yRange/2 - (avg * yRange)/dataRange;
            plotLine(i%numChannels, y);
            planarData[i%numChannels].clear();
        }
    }
}

/*
 *  Returns the max of a set of points.
 *  IN input. Set of points to be aggregated.
 *  OUT  Max of input.
 */
std::int16_t conPlot::maxAggregate(const std::vector<std::int16_t>& input) const
{
    std::int16_t max = std::abs(input[0]);
    int index = 0;
    for (std::uint32_t i = 0; i < input.size(); i++) {
        if (max < std::abs(input[i])) {
            index = i;
            max = std::abs(input[i]);
        }
    }
    return input[index];
}

#if 0 //Keep this!!! Change to use std::?int??_t
//Returns the average of the input.
int conPlot::avgUnique(const std::vector<short>& input)
{
    
    std::set<short> uniques;
    for (auto m:input) 
        uniques.insert(m);
    
    int avg = 0;
    for (auto u:uniques)
        avg += u;

    return avg/(int)uniques.size();
}
#endif

/* 
 * Removes duplicates from the input.
 * IN inVec. 
 * OUT inVec Unique set from the input.
 */

void conPlot::removeDuplicates(std::vector<int>& inVec)
{
    std::vector<int> tmp;
    std::uint32_t i = 0;
    while (i < inVec.size()) {
        tmp.push_back(inVec[i]);
        while ((tmp[tmp.size() - 1] == inVec[i]) && (i < inVec.size()))
            i++;
    }
    inVec = tmp;
}

/*
 * Destructor.
 */
conPlot::~conPlot()
{
    if (resampler != nullptr)
        delete resampler;
}
