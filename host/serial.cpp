#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "send.hpp"
#include <windows.h>
#include <iostream>

/*
#define READ_INTERVAL_TIMEOUT           50UL
#define READ_TOTAL_TIMEOUT_CONSTANT     50UL
#define READ_TOTAL_TIMOUT_MULTIPLIER    10UL
#define WRITE_TOTAL_TIMEOUT_CONSTANT    50UL
#define WRITE_TOTAL_TIMEOUT_MULTIPLIER  10UL
*/

#define READ_INTERVAL_TIMEOUT           MAXDWORD
#define READ_TOTAL_TIMEOUT_CONSTANT     0
#define READ_TOTAL_TIMOUT_MULTIPLIER    0
#define WRITE_TOTAL_TIMEOUT_CONSTANT    50UL
#define WRITE_TOTAL_TIMEOUT_MULTIPLIER  10UL


HANDLE CreateCOMPort(int portNum)
{
    char portNumArray[] = "\\\\.\\COMxx";

    sprintf(&portNumArray[strlen(portNumArray) -2], "%2d", portNum);

    HANDLE hSerial = CreateFile(portNumArray, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening COM port" << std::endl;
    }

    return hSerial;
}

int SetCOMPortSettings(HANDLE hSerial, int baudrate, int parity, int stopBit, int byteSize)
{
    int retVal = 0;
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting COM port state" << std::endl;
        retVal = 1;
    }

    dcbSerialParams.BaudRate = (DWORD)baudrate;
    dcbSerialParams.ByteSize = (BYTE)byteSize;
    dcbSerialParams.StopBits = (BYTE)stopBit;
    dcbSerialParams.Parity   = (BYTE)parity;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting COM port state" << std::endl;
        retVal = 1;
    }

    return retVal;
}

int SetCOMPortTimeouts(HANDLE hSerial)
{
    int retVal = 0;
    COMMTIMEOUTS timeouts = {0};

    timeouts.ReadIntervalTimeout = READ_INTERVAL_TIMEOUT;
    timeouts.ReadTotalTimeoutConstant = READ_TOTAL_TIMEOUT_CONSTANT;
    timeouts.ReadTotalTimeoutMultiplier = READ_TOTAL_TIMOUT_MULTIPLIER;
    timeouts.WriteTotalTimeoutConstant = WRITE_TOTAL_TIMEOUT_CONSTANT;
    timeouts.WriteTotalTimeoutMultiplier = WRITE_TOTAL_TIMEOUT_MULTIPLIER;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting COM port timeouts" << std::endl;
        retVal = 1;
    }

    return retVal;
}

int SendCOMPortData(HANDLE hSerial, char *msg, int len, unsigned long* bytesWritten)
{
    int retVal = 0;
    if (!WriteFile(hSerial, msg, len, bytesWritten, NULL)) {
        std::cerr << "Error writing to COM port" << std::endl;
        retVal = 1;
    }

    return retVal;
}

int ReadCOMPortData(HANDLE hSerial, char *buff, int len, unsigned long* bytesRead)
{
    int retVal = 0;


    if (!ReadFile(hSerial, buff, len, bytesRead, NULL)) {
        std::cerr << "Error reading from COM port" << std::endl;
        retVal = 1;
    }

    return retVal;
}

int ReadCOMPortAllData (HANDLE hSerial, char *buff, unsigned long* bytesRead) 
{
    int readIndexer = 0;
    int retVal = 0;
    unsigned long bytesReadTemp;
    while(1)
    {
        if (ReadFile(hSerial, buff + readIndexer, 1024, &bytesReadTemp, NULL) ) 
        {
            if (bytesReadTemp > 0) 
            {
                readIndexer += bytesReadTemp;
            }
            else 
            {
                *bytesRead = readIndexer;
                break; // No more data available
            }
        } 

        else
        {
            retVal = 1;
        }
        

    }
    
    return retVal;
}


int ReadCOMPortUntilEnd(HANDLE hSerial, char *buff, unsigned long* bytesRead)
{
    int retVal = 0;
    int isFinished = 0;
    int indexer = 0;
    unsigned long tempBytesRead;

    while(!isFinished)
    {

        if (!ReadFile(hSerial, buff + indexer, 1, &tempBytesRead, NULL)) 
        {
            
        }
        else
        {
            if(tempBytesRead != 0)
            {
                indexer += 1;
                *bytesRead += 1;

                if( *buff == 0 )
                {
                    isFinished = 1;
                }
            }
            

        }

        

    }


    return retVal;
}

int ReadCOMPortBlocking(HANDLE hSerial, char *buff, int len)
{
    int retVal = 0;
    int isFinished = 0;
    int indexer = 0;
    unsigned long tempBytesRead = 0;
    unsigned long tempTotalRead = 0;

    while(tempTotalRead < len)
    {

        if (!ReadFile(hSerial, buff + indexer, 1, &tempBytesRead, NULL)) 
        {
            
        }
        else
        {
            if(tempBytesRead != 0)
            {
                indexer += 1;
                tempTotalRead += 1;

            }
            
        }


    }

    return retVal;
}


void CloseCOMPort(HANDLE hSerial)
{
    CloseHandle(hSerial);
}
