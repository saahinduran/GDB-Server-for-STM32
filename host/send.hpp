#ifndef __SEND_HPP_
#define __SEND_HPP_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <iostream>


#define BSWAP(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | (x << 24))
#define READ_INTERVAL_TIMEOUT           50UL
#define READ_TOTAL_TIMEOUT_CONSTANT     50UL
#define READ_TOTAL_TIMOUT_MULTIPLIER    10UL
#define WRITE_TOTAL_TIMEOUT_CONSTANT    50UL
#define WRITE_TOTAL_TIMEOUT_MULTIPLIER  10UL

typedef struct CommandContents
{
    unsigned long memAddr;
    unsigned long memSize;
    char buff[4096];
    unsigned char regToBeRead;
    unsigned long bpAddr;

}CommandContentsStruct;




typedef enum PacketContent
{
    UNDEFINED,
    QStartNoAckMode,
    qfThreadInfo,
    qsThreadInfo,
    qAttached,
    qTStatus,
    AskStopReason,
    AskSupported,
    AskTargetXML,
    ContButDoNothing,
    AskCurrentThreadId,
    Offsets,
    ReadRegisters,
    MustReplyEmpty,
    HaltAnyTarget,
    ReadMemory,
    WriteToMemory,
    HaltTarget,
    ReadOneRegister,
    RunTheTarget,
    StepTheTarget,
    InsertBreakPointToTarget,
    RemoveBreakPointToTarget,
    Error,
    ReportTargetHasStopped,
    FirstResponse,
    SecondResponse,

}PacketContentEnum;

typedef enum TargetCommand
{
    HALT = 'h',
    CONTINUE = 'c',
    START_TARGET = 'e',
    STEP = 's',
    READ_MEMORY = 'm',
    WRITE_MEMORY = 'M',
    INSERT_BREAKPOINT = 'Z',
    REMOVE_BREAKPOINT = 'z',


}TargetCommandEnum;

typedef enum TargetState
{
    RUNNING_FREE,
    HALTED,

}TargetStateEnum;


typedef struct __attribute__((packed)) TargetRegisters
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
    uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
    uint32_t sp;
	uint32_t lr;
	uint32_t pc;
    uint32_t xpsr;

} TargetRegistersStruct;



int compare_subarrays(const char* array1, const char* array2, int from, int to);
bool SendSignal05(SOCKET sock, int num);
bool SendEmptyReponse(SOCKET sock);
bool SendAck(SOCKET sock);
bool SendNack(SOCKET sock);
bool SendFeaturesInitially(SOCKET sock);
bool SendMustReplyEmpty(SOCKET sock);
bool SendOK(SOCKET sock);
bool SendFeatures(SOCKET sock);
bool SendResponse(PacketContentEnum request,SOCKET sock, CommandContentsStruct *cmdContents);
PacketContentEnum ParsePacket(const char *buff, uint16_t len, CommandContentsStruct *cmdContents);


HANDLE CreateCOMPort(int portNum);
int SetCOMPortSettings(HANDLE hSerial, int baudrate, int parity, int stopBit, int byteSize);
int SetCOMPortTimeouts(HANDLE hSerial);
int SendCOMPortData(HANDLE hSerial, char *msg, int len, unsigned long* bytesWritten);
int ReadCOMPortData(HANDLE hSerial, char *buff, int len, unsigned long* bytesRead);
int ReadCOMPortAllData (HANDLE hSerial, char *buff, unsigned long* bytesRead);
int ReadCOMPortUntilEnd(HANDLE hSerial, char *buff, unsigned long* bytesRead);
int ReadCOMPortBlocking(HANDLE hSerial, char *buff, int len);
void CloseCOMPort(HANDLE hSerial);

#endif