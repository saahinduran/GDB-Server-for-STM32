#ifndef __TARGET_HPP_
#define __TARGET_HPP_

#include "send.hpp"


#define TOTAL_REG_NUM           sizeof(TargetRegistersStruct) / sizeof(uint32_t)
#define HALT_MSG_LEN            448U    
#define ADDRESS_SIZE            8        /* How many characters will an address occupy */
#define LENGTH_SIZE             4        /* How many characters will size in read memory command
                                         occupy */

#define GOT_CHAR_MSG_LEN        14          
#define MAX_MEM_REQUEST_LEN     0x100

typedef struct TargetMemory
{
    unsigned long ramStart;
    unsigned long ramEnd;

    unsigned long romStart;
    unsigned long romEnd;

    /* we do not expect user to modify special purpose
    registers of CPU, however client may ask to read these regions*/
    unsigned long specialStart;
    unsigned long specialEnd;

}TargetMemoryStruct;

uint32_t GetUnsignedIntFromASCII(char *ch, uint16_t len);
int IssueTargetCommand(HANDLE hSerial, TargetCommandEnum cmd);
int ReadTargetRegisters(HANDLE hSerial, TargetRegistersStruct *regs, int willContinue);
int ReadTargetMemory(HANDLE hSerial, char *buff, int size, unsigned long addr);
int WriteToTargetMemory(HANDLE hSerial, char *buff, int size, unsigned long addr);
void PrintTargetRegisters(TargetRegistersStruct *regs);
int ContinueTarget(HANDLE hSerial);
int InsertBreakPoint(HANDLE hSerial, unsigned long bpAddr);
int RemoveBreakPoint(HANDLE hSerial, unsigned long bpAddr);
int TargetStep(HANDLE hSerial, TargetRegistersStruct *regs);
int PollForStoppedMessage(HANDLE hSerial, unsigned long *isStopped, TargetRegistersStruct *regs);
unsigned long ReadBufferedRegister(int regNum);

#endif