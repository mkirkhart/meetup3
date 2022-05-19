#pragma once

enum class erpPIOWaitSource 
{
    gpio,
    pin,
    irq,
};

enum class erpPIOJumpCondition 
{
    always,
    not_X,
    XNotZeroPostDec,
    not_Y,
    YNotZeroPoseDec,
    XNotEqualY,
    pin,
    NotOSRempty,
};

enum class erpPIOOutDestination
{
    pins,
    x,
    y,
    null,
    pindirs,
    pc,
    isr,
    exec,
};

enum class erpPIOMovDestination
{
    pins,
    x,
    y,
    exec,
    pc,
    isr,
    osr,
};

enum class erpPIOMovOperation
{
    none,
    invert,
    bit_reverse,
};

enum class erpPIOMovSource
{
    pins,
    x,
    y,
    null,
    status,
    isr,
    osr,

};

#define RP_PIO_INSTRUCTION_COUNT 32

// wrapper for PIO
class rpPIO
{
    private:
        int m_iSideSetEnabledCount = 0;
        int m_iPIOIndex=0;
        int m_iPIOStateMachineIndex=0;
        int m_iFirstInstruction=0;
        int m_iCurrentInstruction=0 ;
        int m_iWrap=0;
        int m_iWrapTarget=0;


        int m_iInstructionMemoryCache[RP_PIO_INSTRUCTION_COUNT] = {0}; // NEED THIS BECAUSE MEMORY IS WRITE ONLY
        char m_chJumpLabels[RP_PIO_INSTRUCTION_COUNT] = {0};
        char m_chLabels[RP_PIO_INSTRUCTION_COUNT] = {0};

        int returnAddressForLabel(char chLabel);
        void SetCurrentInstruction(unsigned char btSideSet, unsigned char btDelay, int iInstruction);

    public:

        void init(int iPIOModuleIndex, int iStateMachineIndex, int iFirstInstruction, int iSideSetCount);
        void start();
        void writeTxFIFO(unsigned int iValue);

        // instruction encoding 
        void encode_begin();
        void encode_end();
        void encode_addLabel(char chLabel);
        void encode_wrapTarget();
        void encode_wrap();
        void encode_out(unsigned char btSideSet, unsigned char btDelay, erpPIOOutDestination eOutDestination, unsigned char bBitCount);
        void encode_jmp(unsigned char btSideSet, unsigned char btDelay, erpPIOJumpCondition eJumpCondition, char chLabel);
        void encode_set_pins_direction(unsigned char btSideSet, unsigned char btDelay, unsigned char bDirection);
        void encode_set_pins(unsigned char btSideSet, unsigned char btDelay, unsigned char bData);
        void encode_wait(unsigned char btSideSet, unsigned char btDelay, bool bWaitPolarityIsOne, erpPIOWaitSource eWaitSource, int iIndex);
        void encode_nop(unsigned char btSideSet, unsigned char btDelay);
        void encode_mov(unsigned char btSideSet, unsigned char btDelay,erpPIOMovDestination eDestination,
                            erpPIOMovOperation eOperation, erpPIOMovSource eSource);


};




