
#include "rpNeoPixel.h"



 void rpNeoPixel::setColor(int iPixel, int iR, int iG, int iB)
 {
     int iFirstByte = iPixel * COLORS_PER_PIXEL;
     m_btColors[iFirstByte++] = iR;
     m_btColors[iFirstByte++] = iG;
     m_btColors[iFirstByte] = iB;
 }


void rpNeoPixel::getColor(int iPixel, int & iR, int & iG, int & iB)
{

}

#define LED_T1  2
#define LED_T2  5
#define LED_T3  3


void rpNeoPixel::init(int iPin, int iPIOModule, int iStateMachineIndex, bool bUseDMA)
{

    obPIO.init(iPIOModule,iStateMachineIndex, 0, 1);

    // create the program
    obPIO.encode_begin();
    obPIO.encode_wrapTarget();
    obPIO.encode_addLabel('a');
    //obPIO.encode_set_pins(0,10,0);
    //obPIO.encode_set_pins(0,5,0);
    
    //obPIO.encode_nop(0, LED_T2-1);
    //obPIO.encode_nop(1, LED_T2-1);
    //obPIO.encode_nop(1, LED_T2-1);

   obPIO.encode_out(0,LED_T3-1,erpPIOOutDestination::x,1);
   obPIO.encode_jmp(1,LED_T1-1,erpPIOJumpCondition::not_X,'d');
    obPIO.encode_jmp(1,LED_T2-1,erpPIOJumpCondition::always,'a');
    obPIO.encode_addLabel('d');
    obPIO.encode_nop(0, LED_T2-1);
    obPIO.encode_wrap();
    obPIO.encode_end();
    
    obPIO.start();
}

void rpNeoPixel::process()
{
    unsigned int iWord;
    

    for (int iCount = 0; iCount < NUM_PIXELS; iCount++)
    {
        int iPixelByte = iCount * COLORS_PER_PIXEL;
        iWord = ((unsigned int)m_btColors[iPixelByte]) << 24 | 
                ((unsigned int)m_btColors[iPixelByte+1]) << 16 | 
                ((unsigned int)m_btColors[iPixelByte+2]) << 8;

        //iWord = 0x00FFFFFF;

        obPIO.writeTxFIFO(iWord);
    }
 
}


