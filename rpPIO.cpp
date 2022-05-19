
#include "rpPIO.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "stdlib.h"
#include "pico/stdio.h"


void rpPIO::encode_wrap()
{
    m_iWrap = m_iCurrentInstruction-1;
}

void rpPIO::encode_wrapTarget()
{
   m_iWrapTarget = m_iCurrentInstruction;
}

int rpPIO::returnAddressForLabel(char chLabel)
{
    int iCount;
    for (iCount=0;iCount< RP_PIO_INSTRUCTION_COUNT;iCount++)
    {
        if (chLabel == m_chLabels[iCount])
            return iCount;
    }
    return -1;
}

void rpPIO::encode_end()
{
    PIO pio;
    int iCount;

    if (m_iPIOIndex == 0)
        pio = pio0;
    else
        pio = pio1;

    pio_sm_set_wrap (pio,m_iPIOStateMachineIndex,m_iWrapTarget, m_iWrap);

    // setup the jump targets
    for (iCount=0;iCount< RP_PIO_INSTRUCTION_COUNT;iCount++)
    {
        if (m_chJumpLabels[iCount])
        {
            m_iInstructionMemoryCache[iCount] |= (returnAddressForLabel(m_chJumpLabels[iCount]) & 0x1f);
        }
    }

    // we have to do this becasue the instructions are write only
    for (iCount=0;iCount< RP_PIO_INSTRUCTION_COUNT;iCount++) // FIX THIS - it will clobber other statemacine instuctions
    {
        pio->instr_mem[iCount] = m_iInstructionMemoryCache[iCount];
    }

}

void rpPIO::encode_addLabel(char chLabel)
{
    m_chLabels[m_iCurrentInstruction] = chLabel;
}

void rpPIO::encode_begin()
{
    // reset the current instruction
    m_iCurrentInstruction = m_iFirstInstruction;
    // clear the address label
    int iCount;
    for (iCount=0;iCount< RP_PIO_INSTRUCTION_COUNT;iCount++)
    {
        m_chLabels[iCount]=0;
        m_chJumpLabels[iCount]=0;
        m_iInstructionMemoryCache[iCount] = 0;
    }

}

void rpPIO::SetCurrentInstruction(unsigned char btSideSet, unsigned char btDelay, int iInstruction)
{
  

    // encode the side set and delay
    // TODO compile option side set enable encoding
    if (m_iSideSetEnabledCount)
    {
        // side set
        iInstruction |= btSideSet << (13 - m_iSideSetEnabledCount);
        // delay 
        iInstruction |= (btDelay<< 8);
    }
    else
    {
        // all delay
        iInstruction |= (btDelay<< 8);
    }


    m_iInstructionMemoryCache[m_iCurrentInstruction] =  iInstruction;

    m_iCurrentInstruction++; // point to next instruction
}

void rpPIO::encode_nop(unsigned char btSideSet, unsigned char btDelay)
{
    // Assembles to mov y, y . "No operation", has no particular side effect, but a useful vehicle for a side-set
    // operation or an extra delay.

    encode_mov(btSideSet,btDelay,erpPIOMovDestination::y,erpPIOMovOperation::none,erpPIOMovSource::y);

}

void rpPIO::encode_mov(unsigned char btSideSet, unsigned char btDelay,erpPIOMovDestination eDestination,
                            erpPIOMovOperation eOperation, erpPIOMovSource eSource)
{
    unsigned int iTemp = 0xA000; // mov command

    switch (eDestination)
    {
        case erpPIOMovDestination::pins:// 000
            break;
        case erpPIOMovDestination::x:// 001
            iTemp |= (1 << 5);
            break;
        case erpPIOMovDestination::y:// 010
            iTemp |= (2 << 5);
            break;
        case erpPIOMovDestination::exec:// 100
            iTemp |= (4 << 5);
            break;
        case erpPIOMovDestination::pc:// 101
            iTemp |= (5 << 5);
            break;
        case erpPIOMovDestination::isr:// 110
            iTemp |= (6 << 5);
            break;
        case erpPIOMovDestination::osr:// 111
            iTemp |= (7 << 5);
            break;
    }

    switch (eOperation)
    {
        case erpPIOMovOperation::none: // 000
            break;
        case erpPIOMovOperation::invert: // 001
            iTemp |= (1 << 3);
            break;
        case erpPIOMovOperation::bit_reverse: // 010
            iTemp |= (2 << 3);
            break;
    }

    switch (eSource)
    {
        case erpPIOMovSource::pins: // 000
            break;
        case erpPIOMovSource::x: // 001
            iTemp |=  1;
            break;
        case erpPIOMovSource::y: // 010
            iTemp |=  2;
            break;
        case erpPIOMovSource::null: // 011
            iTemp |=  3;
            break;
        case erpPIOMovSource::status: // 011
            iTemp |=  5;
            break;
        case erpPIOMovSource::isr: // 011
            iTemp |=  6;
            break;
        case erpPIOMovSource::osr: // 111
            iTemp |=  7;
            break;
    }

    SetCurrentInstruction(btSideSet, btDelay, iTemp);

}

void rpPIO::encode_out(unsigned char btSideSet, unsigned char btDelay, erpPIOOutDestination eOutDestination, unsigned char bBitCount)
{
    unsigned int iTemp = 0x6000; // out command


    switch (eOutDestination)
    {
        case erpPIOOutDestination::pins:
            // 000
            break;
        case erpPIOOutDestination::x:
            // 001
            iTemp |= (1 << 5);
            break;
        case erpPIOOutDestination::y:
            // 010
            iTemp |= (2 << 5);
            break;
        case erpPIOOutDestination::null:
            // 011
            iTemp |= (3 << 5);
            break;
        case erpPIOOutDestination::pindirs:
            // 100
            iTemp |= (4 << 5);
            break;
        case erpPIOOutDestination::pc:
            // 101
            iTemp |= (5 << 5);
            break;
        case erpPIOOutDestination::isr:
        // 110
            iTemp |= (6 << 5);
            break;
        case erpPIOOutDestination::exec:
        // 111
            iTemp |= (7 << 5);
            break;
    }

    iTemp |= (0x1f & bBitCount);

    SetCurrentInstruction(btSideSet, btDelay, iTemp);
    
}



void rpPIO::encode_jmp(unsigned char btSideSet, unsigned char btDelay,  erpPIOJumpCondition eJumpCondition,  char chAddress)
{
    unsigned int iTemp = 0x0; // jump command

    
    switch (eJumpCondition)
    {
        case erpPIOJumpCondition::always:
            // condition is ALWAYS (000)
            break;
        case erpPIOJumpCondition::not_X:
            // 001
            iTemp |= (1 << 5);
            break;
        case erpPIOJumpCondition::XNotZeroPostDec:
            // 010
            iTemp |= (2 << 5);
            break;
        case erpPIOJumpCondition::not_Y:
            // 011
            iTemp |= (3 << 5);
            break;
        case erpPIOJumpCondition::YNotZeroPoseDec:
            // 100
            iTemp |= (4 << 5);
            break;
        case erpPIOJumpCondition::XNotEqualY:
            // 101
            iTemp |= (5 << 5);
            break;
        case erpPIOJumpCondition::pin:
            // 110
            iTemp |= (6 << 5);
            break;
        case erpPIOJumpCondition::NotOSRempty:
            // 111
            iTemp |= (7 << 5);
            break;
    }

    m_chJumpLabels[m_iCurrentInstruction] = chAddress;
    
    SetCurrentInstruction(btSideSet, btDelay, iTemp);
}

void rpPIO::encode_set_pins_direction(unsigned char btSideSet, unsigned char btDelay, unsigned char bDirection)
{
    unsigned int iTemp = 0xE000; // set command

    // TODO IMPLEMENT DELAY SIDE SET

    // destination is PINSDIR (100)
    iTemp |= 0x80;

    iTemp |= (0x1f & bDirection);

    SetCurrentInstruction(btSideSet, btDelay, iTemp);
}

void rpPIO::encode_set_pins(unsigned char btSideSet, unsigned char btDelay, unsigned char bData)
{
    unsigned int iTemp = 0xE000; // set command

    // TODO IMPLEMENT DELAY SIDE SET

    // destination is PINS (000)
    iTemp |= (0x1f & bData);

    SetCurrentInstruction(btSideSet, btDelay,iTemp);
}

void rpPIO::encode_wait(unsigned char btSideSet, unsigned char btDelay,  bool bWaitPolarityIsOne, erpPIOWaitSource eWaitSource, int iIndex)
{
    unsigned int iTemp = 0x2000; // wait command

    if (bWaitPolarityIsOne)
        iTemp |= 0x80;
    
    switch (eWaitSource)
    {
        case erpPIOWaitSource::gpio:
            // 00
            break;
        case erpPIOWaitSource::pin:
            // 01
            iTemp |= (1 << 5);
            break;
        case erpPIOWaitSource::irq:
            // 10
            iTemp |= (2 << 5);
            break;
        default: // reserved
            break;
    }

    // index 
    iTemp |= iIndex & 0x1F;

    SetCurrentInstruction(btSideSet, btDelay,iTemp);

}


void rpPIO::init(int iPIOModuleIndex, int iPIOStateMachineIndex, int iFirstInstruction, int iSideSetCount)
{

    PIO pio;
    
    m_iPIOIndex = iPIOModuleIndex;
    m_iFirstInstruction = iFirstInstruction;
    m_iPIOStateMachineIndex = iPIOStateMachineIndex;
    m_iSideSetEnabledCount = iSideSetCount;

    if (m_iPIOIndex == 0)
        pio = pio0;
    else
        pio = pio1;

    int iStartPin = 28;
    
    gpio_set_dir(iStartPin, GPIO_OUT);



    pio_gpio_init(pio, iStartPin);
    pio_sm_set_consecutive_pindirs(pio, m_iPIOStateMachineIndex, iStartPin, 1, true);

    pio_sm_config c = {0};// = ws2812_program_get_default_config(offset);
    
    sm_config_set_sideset_pins(&c, iStartPin);
    
    sm_config_set_out_shift(&c, false, true,  24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    int ws2812_T1 = 2;
    int ws2812_T2 = 5;
    int ws2812_T3 = 3;
    float freq = 800000; // 

    int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
    float div = clock_get_hz(clk_sys) / (freq * (float)cycles_per_bit);
    sm_config_set_clkdiv(&c, div);
    

    pio_sm_init(pio, m_iPIOStateMachineIndex, iFirstInstruction, &c);
   
//*
    //pio->sm[0].pinctrl =
    //    (1 << PIO_SM0_PINCTRL_SET_COUNT_LSB) |
    //    (iStartPin << PIO_SM0_PINCTRL_SET_BASE_LSB);
    pio->sm[0].pinctrl = (1 << 29) |  // PIO_SM0_SIDESET_COUNT
                         (28 << 10) ;  // SIDESET_BASE
                         

    gpio_set_function(iStartPin, GPIO_FUNC_PIO1);
    pio_gpio_init(pio,iStartPin);
//*/
}

void rpPIO::start()
{
    PIO pio;
    if (m_iPIOIndex == 0)
        pio = pio0;
    else
        pio = pio1;

    //hw_set_bits(&pio->ctrl, 1 << (PIO_CTRL_SM_ENABLE_LSB + 0));
    pio_sm_set_enabled(pio, m_iPIOStateMachineIndex, true);
}


void rpPIO::writeTxFIFO(unsigned int iValue)
{
    PIO pio;
    if (m_iPIOIndex == 0)
        pio = pio0;
    else
        pio = pio1;

    pio_sm_put_blocking(pio, m_iPIOStateMachineIndex, iValue);

}