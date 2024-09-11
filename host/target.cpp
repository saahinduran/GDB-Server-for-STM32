#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "send.hpp"
#include <windows.h>
#include <iostream>
#include "target.hpp"



extern TargetRegistersStruct regs;

TargetMemoryStruct targetMem = {.ramStart = 0x20000000, .ramEnd = 0x2004ffff,
    .romStart = 0x08000000, .romEnd = 0x080fffff, .specialStart = 0xE000E000,
    .specialEnd = 0xE000EDEF,
};

uint32_t GetUnsignedIntFromASCII(char *ch, uint16_t len)
{
    uint32_t result = 0;

    for (int i = 0; i < len; i++) {
        char c = ch[i];
        uint32_t value = 0;

        // Convert ASCII character to its hex value
        if (isdigit(c)) {
            value = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            value = 10 + (c - 'A');
        }

        // Shift the previous result left by 4 bits and add the new value
        result = (result << 4) | value;
    }

    return result;
}



int IssueTargetCommand(HANDLE hSerial, TargetCommandEnum cmd)
{
    char c = (char)cmd;

    unsigned long bytesWritten = 0;
    int retVal = 0;
    if(SendCOMPortData(hSerial, &c, 1, &bytesWritten) )
    {
        retVal = 1;
    }

    return retVal;
}


/* This function halts the target, and reads the registers where the exceution has stopped.
    if willContinue is set to 1: processor continues after sending registers, if not,
    it will wait for another debug message packets.*/
int ReadTargetRegisters(HANDLE hSerial, TargetRegistersStruct *regs, int willContinue)
{
    int retVal = 0;
    unsigned long bytesRead;
    char buff[1024];

    if(IssueTargetCommand(hSerial, HALT) ) 
    {
        retVal = 1;
    }

    else
    {
        ReadCOMPortBlocking(hSerial, buff, HALT_MSG_LEN);

        char *pos;
        unsigned int regVal;
        pos = buff;

        for(int i = 0; i < TOTAL_REG_NUM; i++)
        {
            pos = strchr( (const char *)pos, '=');

            pos += 2;

            regVal = GetUnsignedIntFromASCII(pos +1, 8);
            *(uint32_t *)( (uint32_t)&regs->r0 + (i*4) ) = regVal;
        }
                

        
        
    }
    Sleep(2);  // Sleep for 2000 milliseconds (2 seconds)

    if(willContinue)
    {
        IssueTargetCommand(hSerial, CONTINUE);
        ReadCOMPortBlocking(hSerial, buff, GOT_CHAR_MSG_LEN);
    }
    

    return retVal;
}

int ReadTargetMemory(HANDLE hSerial, char *buff, int size, unsigned long addr)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char msg[256] = {0};

    if(IssueTargetCommand(hSerial, READ_MEMORY) ) 
    {
        retVal |= 1;
    }

    else
    {
        sprintf(msg,"%08x",addr);
        if(SendCOMPortData(hSerial, msg, ADDRESS_SIZE, &bytesWritten) )
        {
            retVal |= 1;
        }

        sprintf(msg,"%04x",size);
        if(SendCOMPortData(hSerial, msg, LENGTH_SIZE, &bytesWritten) )
        {
            retVal |= 1;
        }

        ReadCOMPortBlocking(hSerial, buff, size);   
        
    }
    
    return retVal;

}

unsigned long ReadBufferedRegister(int regNum)
{
    if(regNum < 16)
    {
        return *(unsigned long *)( (unsigned char *)&regs + regNum *4);
    }

    else if (regNum >= 26)
    {
        if(regNum == 26)
        {   /* gdb client asked for MSP, we will tell SP in return, cuz we assume target
            always uses MAIN STACK POINTER (MSP) */
            return regs.sp;
        }

        //TODO: not implemented!
        else
        {
            return 0xDEADBEEF;
        }
        
    }

}


int WriteToTargetMemory(HANDLE hSerial, char *buff, int size, unsigned long addr)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char msg[256] = {0};

    if(IssueTargetCommand(hSerial, WRITE_MEMORY) ) 
    {
        retVal |= 1;
    }

    else
    {
        sprintf(msg,"%08x",addr);
        if(SendCOMPortData(hSerial, msg, ADDRESS_SIZE, &bytesWritten) )
        {
            retVal |= 1;
        }

        sprintf(msg,"%04x",size);
        if(SendCOMPortData(hSerial, msg, LENGTH_SIZE, &bytesWritten) )
        {
            retVal |= 1;
        }

        if(SendCOMPortData(hSerial, buff, size, &bytesWritten) )
        {
            retVal |= 1;
        }
        
    }
    
    return retVal;

}

int ContinueTarget(HANDLE hSerial)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char buff[256] = {0};

    if(IssueTargetCommand(hSerial, CONTINUE) ) 
    {
        retVal |= 1;
    }

    ReadCOMPortBlocking(hSerial, buff, GOT_CHAR_MSG_LEN);

    return retVal;
}

int TargetStep(HANDLE hSerial, TargetRegistersStruct *regs)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char buff[512] = {0};
    char *pos;
    unsigned int regVal;
    pos = buff;


    if(IssueTargetCommand(hSerial, STEP) ) 
    {
        retVal |= 1;
    }

    ReadCOMPortBlocking(hSerial, buff, GOT_CHAR_MSG_LEN);

    ReadCOMPortBlocking(hSerial, buff, HALT_MSG_LEN);

    

    for(int i = 0; i < TOTAL_REG_NUM; i++)
    {
        pos = strchr( (const char *)pos, '=');

        pos += 2;

        regVal = GetUnsignedIntFromASCII(pos +1, 8);
        *(uint32_t *)( (uint32_t)&regs->r0 + (i*4) ) = regVal;
    }
    

    return retVal;
}

int InsertBreakPoint(HANDLE hSerial, unsigned long bpAddr)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char msg[256] = {0};

    if(IssueTargetCommand(hSerial, INSERT_BREAKPOINT) ) 
    {
        retVal |= 1;
    }


    sprintf(msg,"%08x",bpAddr);

    if(SendCOMPortData(hSerial, msg, ADDRESS_SIZE, &bytesWritten) )
    {
        retVal |= 1;
    }

    return retVal;

}

int RemoveBreakPoint(HANDLE hSerial, unsigned long bpAddr)
{
    int retVal = 0;
    unsigned long bytesWritten;
    char msg[256] = {0};

    if(IssueTargetCommand(hSerial, REMOVE_BREAKPOINT) ) 
    {
        retVal |= 1;
    }


    sprintf(msg,"%08x",bpAddr);

    if(SendCOMPortData(hSerial, msg, ADDRESS_SIZE, &bytesWritten) )
    {
        retVal |= 1;
    }

    return retVal;
    
}

int PollForStoppedMessage(HANDLE hSerial, unsigned long *isStopped, TargetRegistersStruct *regs)
{
    int retVal = 0;
    COMMTIMEOUTS timeouts = {0};
    char buff[512] = {0};
    unsigned long bytesRead = 0;

    *isStopped = 0;
    
    timeouts.ReadIntervalTimeout = 50;          // 50 ms timeout between characters
    timeouts.ReadTotalTimeoutConstant = 50;     // 50 ms total timeout
    timeouts.ReadTotalTimeoutMultiplier = 10;   // Multiplier per character
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting COM port timeouts" << std::endl;
        retVal = 1;
    }

    retVal |= ReadCOMPortData(hSerial, buff, 1, &bytesRead);

    timeouts.ReadIntervalTimeout = READ_INTERVAL_TIMEOUT;
    timeouts.ReadTotalTimeoutConstant = READ_TOTAL_TIMEOUT_CONSTANT;
    timeouts.ReadTotalTimeoutMultiplier = READ_TOTAL_TIMOUT_MULTIPLIER;
    timeouts.WriteTotalTimeoutConstant = WRITE_TOTAL_TIMEOUT_CONSTANT;
    timeouts.WriteTotalTimeoutMultiplier = WRITE_TOTAL_TIMEOUT_MULTIPLIER;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting COM port timeouts" << std::endl;
        retVal = 1;
    }

    if(bytesRead == 1)
    {
        retVal |= ReadCOMPortBlocking(hSerial, buff, HALT_MSG_LEN -1);

        char *pos;
        unsigned int regVal;
        pos = buff;

        for(int i = 0; i < TOTAL_REG_NUM; i++)
        {
            pos = strchr( (const char *)pos, '=');

            pos += 2;

            regVal = GetUnsignedIntFromASCII(pos +1, 8);
            *(uint32_t *)( (uint32_t)&regs->r0 + (i*4) ) = regVal;
        }

        *isStopped = 1;
    }

    return retVal;
}






void PrintTargetRegisters(TargetRegistersStruct *regs)
{

    printf("Register Dump\r\n");
    printf(" r0  =0x%08x\r\n", regs->r0);
    printf(" r1  =0x%08x\r\n", regs->r1);
    printf(" r2  =0x%08x\r\n", regs->r2);
    printf(" r3  =0x%08x\r\n", regs->r3);
    printf(" r4  =0x%08x\r\n", regs->r4);
    printf(" r5  =0x%08x\r\n", regs->r5);
    printf(" r6  =0x%08x\r\n", regs->r6);
    printf(" r7  =0x%08x\r\n", regs->r7);
    printf(" r8  =0x%08x\r\n", regs->r8);
    printf(" r9  =0x%08x\r\n", regs->r9);
    printf(" r10 =0x%08x\r\n", regs->r10);
    printf(" r11 =0x%08x\r\n", regs->r11);
    printf(" r12 =0x%08x\r\n", regs->r12);
    printf(" lr  =0x%08x\r\n", regs->lr);
    printf(" sp  =0x%08x\r\n", regs->sp);
    printf(" pc  =0x%08x\r\n", regs->pc);
    printf(" xpsr=0x%08x\r\n", regs->xpsr);

}


