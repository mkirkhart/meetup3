#pragma once

#include "rpPIO.h"

#define NUM_PIXELS          4
#define COLORS_PER_PIXEL    3 // rbg = 3 rgbw = 4

class rpNeoPixel 
{
    private:
        rpPIO obPIO;
        unsigned char m_btColors[COLORS_PER_PIXEL*NUM_PIXELS];

    public:

        void setColor(int iPixel, int iR, int iG, int iB);
        void getColor(int iPixel, int & iR, int & iG, int & iB);

        void init(int iPin, int iPIOModule, int iStateMachineIndex, bool bUseDMA);
        void process();

};

