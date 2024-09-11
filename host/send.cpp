#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <stdint.h>

#include "send.hpp"
#include "target.hpp"

extern HANDLE handleSerial;
extern TargetRegistersStruct regs;
extern TargetMemoryStruct targetMem;

unsigned long memAddr = 0;

int compare_subarrays(const char* array1, const char* array2, int from, int to) {
    for (int i = from; i <= to; i++) {
        if (array1[i] != array2[i]) {
            return -1; // Arrays differ at index i
        }
    }
    return 0; // All elements from  to  are equal
}


bool SendSignal05(SOCKET sock, int num)
{
    char *sendBuff = "$S05#b8";
/*
    sendBuff[2] = (char)((num / 10) + '0');     // First digit
    sendBuff[3] = (char)((num % 10) + '0'); // Second digit
*/
    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}


bool SendThreadInfo(SOCKET sock)
{
    char *sendBuff = "$m0000dead#bb";
    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}


bool SendEmptyReponse(SOCKET sock)
{
    char *sendBuff = "$#00";
    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}

bool SendAck(SOCKET sock)
{
    const char sendChar = '+';
    if(send(sock, &sendChar, 1, 0) == -1)
    {
        return false;
    }

    return true;
    
}

bool SendFirstResponse(SOCKET sock)
{
    char *sendBuff = "$4f2e4b2e3a307845303046443030330D0A#d6";


    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}

bool SendSecondResponse(SOCKET sock)
{
    char *sendBuff = "$4f2e4b2e3a307830303030303030302c307830303030303030302c307830303030303030302c307830303030303030302c307830303030303034392c307830303030303030342c307830303030303031412c307830303030303030300D0A#4e";


    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}




bool SendNack(SOCKET sock)
{
    const char sendChar = '-';
    if(send(sock, &sendChar, 1, 0) == -1)
    {
        return false;
    }

    return true;
    
}
bool SendFeaturesInitially(SOCKET sock)
{
    char *sendBuff = "$PacketSize=4000;qXfer:memory-map:read-;\
QStartNoAckMode+;hwbreak+;qXfer:features:read+#b1";


    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}

bool SendMustReplyEmpty(SOCKET sock)
{
    char *sendBuff = "$vMustReplyEmpty#3a";

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}

bool SendOK(SOCKET sock)
{
    char *sendBuff = "$OK#9a";

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}

bool SendTargetRegisters(SOCKET sock)
{
    char sendBuff[1024] = {0};
    sendBuff[0] = '$';

    unsigned char chkSum = 0;

    sprintf(&sendBuff[1], "%08x", BSWAP(regs.r0) );
    sprintf(&sendBuff[9], "%08x", BSWAP(regs.r1) );
    sprintf(&sendBuff[17], "%08x", BSWAP(regs.r2) );
    sprintf(&sendBuff[25], "%08x", BSWAP(regs.r3) );
    sprintf(&sendBuff[33], "%08x", BSWAP(regs.r4) );
    sprintf(&sendBuff[41], "%08x", BSWAP(regs.r5) );
    sprintf(&sendBuff[49], "%08x", BSWAP(regs.r6) );
    sprintf(&sendBuff[57], "%08x", BSWAP(regs.r7) );
    sprintf(&sendBuff[65], "%08x", BSWAP(regs.r8) );
    sprintf(&sendBuff[73], "%08x", BSWAP(regs.r9) );
    sprintf(&sendBuff[81], "%08x", BSWAP(regs.r10) );
    sprintf(&sendBuff[89], "%08x", BSWAP(regs.r11) );
    sprintf(&sendBuff[97], "%08x", BSWAP(regs.r12) );
    sprintf(&sendBuff[105], "%08x", BSWAP(regs.sp) );
    sprintf(&sendBuff[113], "%08x", BSWAP(regs.lr) );
    sprintf(&sendBuff[121], "%08x", BSWAP(regs.pc) );
    sprintf(&sendBuff[129], "%08x", BSWAP(regs.xpsr) );

    sendBuff[137] = '#';

    for(int i = 1; i < 138; i++)
    {
        chkSum += sendBuff[i];
    }

    sprintf(&sendBuff[138], "%02x", chkSum);


    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}

bool SendEndOfList(SOCKET sock)
{
    char *sendBuff = "$l#6c";

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}


bool SendFeatures(SOCKET sock)
{
    static int sentCount = 1;

    char sendBuff[] = "$m<?xml version=\"1.0\"?> \n\
<!DOCTYPE feature SYSTEM \"gdb-target.dtd\"> \n\
<!-- sahin stub\n\
-->\n\
<target version=\"1.0\"> \n\
<feature name=\"org.gnu.gdb.arm.m-profile\"> \n\
<reg name=\"r0\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r1\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r2\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r3\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r4\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r5\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r6\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r7\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r8\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r9\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r10\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r11\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"r12\" bitsize=\"32\" type=\"uint32\"/>\n\
<reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>\n\
<reg name=\"lr\" bitsize=\"32\"/>\n\
<reg name=\"pc\" bitsize=\"32\" type=\"code_ptr\"/>\n\
<reg name=\"xpsr\" bitsize=\"32\" regnum=\"25\"/>\n\
</feature>\n </target>#aa";

    /* find checksum */
    int size = strlen(sendBuff);

    uint8_t chkSum = 0;

    for( int i = 0; i < size -3; i++)
    {
        chkSum += sendBuff[i];
    }

    sprintf(&sendBuff[size] -2, "%02x", chkSum);


    if(sentCount == 1)
    {
        if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
        {

        }
        
    }

    else if(sentCount == 2 )
    {
        if(SendEndOfList(sock) == false )
        {
            return false;
        }
    }

    sentCount++;
    return true;
    
}

bool SendTargetMemory(SOCKET sock, char *buff, int len)
{

    char sendBuff[1024] = {0};
    unsigned char chkSum = 0;


    for(int i = 0; i < len; i++)
    {
        sprintf(&sendBuff[1] +i*2, "%02x", *(unsigned char *) (buff+ i) );
    }

    sendBuff[0] = '$';

    for(int i = 1; i < (len * 2) + 1; i++)
    {
        chkSum += sendBuff[i];
    }
    
    sendBuff[1 + len *2] = '#';

    sprintf(&sendBuff[2 + len *2], "%02x", chkSum);

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}

bool SendTargetStoppedMsg(SOCKET sock)
{
    char *sendBuff = "$T05thread:0000DEAD;#74";

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}

bool SendError01(SOCKET sock)
{
    char *sendBuff = "$E01#a6";

    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;
}


bool ReportOneRegister(SOCKET sock, unsigned long value)
{
    
    char sendBuff[16] = {0};
    unsigned char *ptrToValue = (unsigned char *)&value;
    unsigned char chkSum = 0;

    sendBuff[0] = '$';

    for(int i = 0; i < 4; i++)
    {
        sprintf(&sendBuff[1] +i*2, "%02x", *(ptrToValue+ i) );
    }

    sendBuff[1 + 4 *2] = '#';

    for(int i = 1; i < (4 * 2) + 1; i++)
    {
        chkSum += sendBuff[i];
    }

    sprintf(&sendBuff[2 + 4 *2], "%02x", chkSum);


    if(send(sock, sendBuff, strlen(sendBuff), 0) == -1)
    {
        return false;
    }

    return true;

}







PacketContentEnum ParsePacket(const char *buff, uint16_t len, CommandContentsStruct *cmdContents)
{

    char *ptrFirst;
    char *ptrLast;

    if(3 == buff[0] && len == 1)
    {
        return HaltTarget;
    }

    switch(buff[1])
    {
        case 'Q':
            if(!strcmp(&buff[2], "StartNoAckMode#b0") )
            {
                return QStartNoAckMode;
            }
        break;

        case 'q':
            if(!strcmp(&buff[2], "fThreadInfo#bb") )
            {
                return qfThreadInfo;
            }

            else if(!strcmp(&buff[2], "ThreadExtraInfo,dead#13") )
            {
                return UNDEFINED;
            }


            else if(!strcmp(&buff[2], "sThreadInfo#c8") )
            {
                return qsThreadInfo;
            }

            else if(!strcmp(&buff[2], "Symbol::#5b") )
            {
                return ContButDoNothing;
            }


            else if(!strcmp(&buff[2], "Attached#8f") )
            {
                return qAttached;
            }

            else if(!strcmp(&buff[2], "TStatus#49") )
            {
                return qTStatus;
            }

            else if(!strcmp(&buff[2], "C#b4") )
            {
                return AskCurrentThreadId;
            }

            else if(!strcmp(&buff[2], "Offsets#4b") )
            {
                return Offsets;
            }

            else if(!compare_subarrays(&buff[2], "Supported", 0,8) )
            {
                return AskSupported;
            }

            else if(!compare_subarrays(&buff[2], "Xfer:features:read:target.xml:0,1000#0c", 0,20) )
            {
                return AskTargetXML;
            }

            else if(buff[2] == 'P')
            {
                return UNDEFINED;
            }

            //printf("ask for first password \n");
            else if(!strcmp(&buff[2], "Rcmd,5265616441504578203078302030784638#08") )
            {
                printf("ask for first password 2\n");
                    return FirstResponse;
            }
            //printf("ask for second password \n");
            else if(!strcmp(&buff[2], "Rcmd,526561644d656d41502030783020307845303046444644302030783820307830#64") )
            {
                printf("ask for second password 2 \n");
                    return SecondResponse;
            }




             

        break;

        case 'g':
            return ReadRegisters;
        break;

        case '?':
            return AskStopReason;
        break;

        case 'H':
            if(!strcmp(&buff[2], "c-1#09") )
            {
                return ContButDoNothing;
            }

            else if(!strcmp(&buff[2], "g0#df") )
            {
                return ContButDoNothing;
            }

            else if(!strcmp(&buff[2], "cdead#39") )
            {
                return ContButDoNothing;
            }

            else if(!strcmp(&buff[2], "c0#db") )
            {
                return ContButDoNothing;
            }


        break;

        case 'v':
            if(!strcmp(&buff[2], "MustReplyEmpty#3a") )
            {
                return MustReplyEmpty;
            }
            else
            {
                return UNDEFINED;
            }

        break;

        case 'm':
            ptrFirst = strchr(buff,',');
            ptrLast = strchr(buff, '#');
            cmdContents ->memAddr = GetUnsignedIntFromASCII((char *)&buff[2], ADDRESS_SIZE);

            cmdContents ->memSize = GetUnsignedIntFromASCII(ptrFirst +1, int(ptrLast - ptrFirst) -1);

            printf("Requested addr: 0x%08x, size : 0x%08x\r\n",cmdContents ->memAddr, cmdContents ->memSize);
            return ReadMemory;
        break;

        case 'p':

            if(!strcmp(&buff[2], "1a#02") )
            {
                    cmdContents->regToBeRead = 26;
                    return ReadOneRegister;
            }

            else if(!strcmp(&buff[2], "1b#03") )
            {
                    cmdContents->regToBeRead = 27;
                    return ReadOneRegister;
            }

        break;

        case 'c':

            if(!strcmp(&buff[2], "#63") )
            {
                return RunTheTarget;
            }

        break;

        case 's':

            if(!strcmp(&buff[2], "#73") )
            {
                return StepTheTarget;
            }

        break;

        case 'Z':
            ptrFirst = strchr(buff,',');
            ptrFirst += 1;

            cmdContents->bpAddr = GetUnsignedIntFromASCII(ptrFirst, 8); 

            return InsertBreakPointToTarget;
        break;
        
        case 'z':
            ptrFirst = strchr(buff, ',');
            ptrFirst += 1;

            cmdContents->bpAddr = GetUnsignedIntFromASCII(ptrFirst, 8);

            return RemoveBreakPointToTarget;
        break;


        case 'X':
            cmdContents->memAddr = GetUnsignedIntFromASCII( (char *)&buff[2], 8);

            ptrFirst = strchr(buff, ',');
            ptrLast = strchr(buff, ':');

            cmdContents->memSize = GetUnsignedIntFromASCII( (char *)ptrFirst +1, 
                ptrLast - ptrFirst -1);
            
            memcpy(cmdContents->buff, ptrLast +1, cmdContents->memSize);

            if(cmdContents->memSize > 0)
            {
            	ptrFirst++;
            	ptrFirst--;
            }

            return WriteToMemory;

        break;


        default:
        break;
    }
    
}

bool SendResponse(PacketContentEnum request,SOCKET sock, CommandContentsStruct *cmdContents)
{
    unsigned long value = 0xDEADBEEF;

    bool isRam, isRom, isSpecial;


    switch(request)
    {
        case UNDEFINED:
            SendEmptyReponse(sock);
        break;

        case AskTargetXML:
            SendFeatures(sock);
        break;

        case QStartNoAckMode:
            SendOK(sock);
        break;

        case qfThreadInfo:
            SendThreadInfo(sock);
        break;

        case qsThreadInfo:
            SendEndOfList(sock);
        break;

        case HaltAnyTarget:
            SendOK(sock);
        break;

        case qAttached:
            SendEmptyReponse(sock);
        break;

        case qTStatus:
            SendEmptyReponse(sock);
        break;

        case AskSupported:
            SendFeaturesInitially(sock);
        break;

        case AskStopReason:
            SendSignal05(sock, 5);
        break;

        case ContButDoNothing:
            SendOK(sock);
        break;

        case AskCurrentThreadId:
            SendEmptyReponse(sock);
        break;

        case Offsets:
            SendEmptyReponse(sock);
        break;

        case ReadRegisters:
            SendTargetRegisters(sock);
            PrintTargetRegisters(&regs);
        break;

        case MustReplyEmpty:
            SendEmptyReponse(sock);
        break;

        case HaltTarget:
            printf("Target stopped \r\n");
            ReadTargetRegisters(handleSerial, &regs, 0);
            SendTargetStoppedMsg(sock);
            PrintTargetRegisters(&regs);
        break;


        case ReadMemory:
            isRam = (cmdContents ->memAddr >= targetMem.ramStart) && 
                ( (cmdContents ->memAddr +  cmdContents ->memSize) <= targetMem.ramEnd);

            isRom = (cmdContents ->memAddr >= targetMem.romStart) && 
                ( (cmdContents ->memAddr +  cmdContents ->memSize) <= targetMem.romEnd);

            isSpecial = (cmdContents ->memAddr >= targetMem.specialStart) && 
                ( (cmdContents ->memAddr +  cmdContents ->memSize) <= targetMem.specialEnd);

            if(isRam || isRom ||isSpecial)
            {
                ReadTargetMemory(handleSerial, &cmdContents->buff[0], 
                cmdContents ->memSize, cmdContents ->memAddr);

                SendTargetMemory(sock, &cmdContents->buff[0], cmdContents ->memSize);
            }
            else
            {
                SendError01(sock);
            }
            
        break;

        case WriteToMemory:

            isRam = (cmdContents ->memAddr >= targetMem.ramStart) && 
                (cmdContents ->memAddr <= targetMem.ramEnd);
            
            isRom = (cmdContents ->memAddr >= targetMem.romStart) && 
                (cmdContents ->memAddr <= targetMem.romEnd);

            isSpecial = (cmdContents ->memAddr >= targetMem.specialStart) && 
                (cmdContents ->memAddr <= targetMem.specialEnd);


            if(isRam || isRom || isSpecial)
            {
                if(cmdContents ->memAddr != 0 && cmdContents ->memSize < MAX_MEM_REQUEST_LEN)
                {
                    WriteToTargetMemory(handleSerial, &cmdContents->buff[0],
                        cmdContents ->memSize, cmdContents ->memAddr);
                }
                

                SendOK(sock);
            }

            SendEmptyReponse(sock);
            
        break;


        case ReadOneRegister:
            value = ReadBufferedRegister(cmdContents->regToBeRead);
            ReportOneRegister(sock, value);
        break;


        case RunTheTarget:
            printf("Continue request asserted \r\n");
            ContinueTarget(handleSerial);
        break;

        case StepTheTarget:
            printf("Step request asserted \r\n");
            TargetStep(handleSerial, &regs);
            SendTargetStoppedMsg(sock);
            PrintTargetRegisters(&regs);
        break;

        case Error:
            SendError01(sock);
        break;

        case InsertBreakPointToTarget:
            printf("Insert bkpt request from addr 0x%08x\r\n",cmdContents->bpAddr);
            InsertBreakPoint(handleSerial , cmdContents->bpAddr);
            SendOK(sock);
        break;

        case RemoveBreakPointToTarget:
            printf("Remove bkpt request from addr 0x%08x\r\n",cmdContents->bpAddr);
            RemoveBreakPoint(handleSerial, cmdContents->bpAddr);
            SendOK(sock);
        break;

        case ReportTargetHasStopped:
            printf("Target stopped \r\n");
            SendTargetStoppedMsg(sock);
        break;


        case FirstResponse:
            SendFirstResponse(sock);

        break;


        case SecondResponse:
            SendSecondResponse(sock);
        break;


        default:

        break;


    }
}
