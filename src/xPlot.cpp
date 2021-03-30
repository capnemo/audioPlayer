#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include "xPlot.h"

/*
 * Initialize the xPlot object. Sets up the connection to the XServer and 
 * starts the graphing window. 
 */
bool xPlot::initX()
{
    if ((display = XOpenDisplay(0)) == 0)  {
        std::cout << "Error connecting to the X server" << std::endl;
        return false;
    }
    return true;
}

/* 
 * Draw a single window. Dimensions are a function of the size of the screen.
 * IN serial Serial number of the window. No Op for now.
 */
void xPlot::drawWindow(int serial)
{
    Screen* scrn = ScreenOfDisplay(display, screen);
    int screenWidth = scrn->width;
    int screenHeight = scrn->height;
    if (screenWidth > 2 * screenHeight)
        screenWidth = 1.5 * screenHeight;

    windowWidth = screenWidth - 0.2 * screenWidth;
    windowHeight = screenHeight * 0.33;

    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 0, 0, windowWidth, windowHeight, 1, 0,
                                 BlackPixel(display, screen));

    XSelectInput(display, window, ExposureMask);

    graphicsContext = XCreateGC(display, window, 0, &gcValues);
    XSetLineAttributes(display, graphicsContext, 1, LineSolid, CapRound,
                       JoinRound);

#if 0
    //XMoveWindow(display, window, 0, serial * windowHeight);

    if (serial == 1) {
        XWindowChanges ch;
        ch.y = 500;              
        XConfigureWindow(display, window, CWY, &ch);
    }

#endif
    XMapWindow(display, window);

    XEvent event;
    do {
        XNextEvent(display, &event);
    } while (event.type != Expose);
}

/*
 * Draw a white line from (x1,y1) to (x2, y2)
 * IN: x1, y2, x2, y2
 */
void xPlot::drawWhiteLine(std::uint32_t x1, std::uint32_t y1, 
                          std::uint32_t x2, std::uint32_t y2)
{
    XSetForeground(display, graphicsContext, WhitePixel(display, screen));
    XDrawLine(display, window, graphicsContext, x1, y1, x2, y2);
}

/*
 * Draw a line from (x1,y1) to (x2, y2)
 * IN: x1, y2, x2, y2  coordinates.
 * IN: colorIndex Index into lineColors.
 */
void xPlot::drawColoredLine(std::uint32_t x1, std::uint32_t y1, 
                            std::uint32_t x2, std::uint32_t y2, 
                            long colorIndex)
{
    XSetForeground(display, graphicsContext, lineColors[colorIndex]);
    XDrawLine(display, window, graphicsContext, x1, y1, x2, y2);
}

/*
 *  Destructor
 */
xPlot::~xPlot()
{
    XFreeGC(display, graphicsContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

