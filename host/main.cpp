#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "send.hpp"
#include <windows.h>
#include <iostream>
#include "target.hpp"


#define PORT 8080
#define BUFSIZE 4000

void InitializeConnection(SOCKET sock);
HANDLE handleSerial;
TargetRegistersStruct regs;

int main() {

    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize;


    handleSerial = CreateCOMPort(13);
    if(handleSerial != INVALID_HANDLE_VALUE)
    {
        SetCOMPortSettings(handleSerial, 115200, NOPARITY, ONESTOPBIT, 8);
        SetCOMPortTimeouts(handleSerial);
    }
    /* We have to stop the target and read registers */

    ReadTargetRegisters(handleSerial, &regs, 0);
    PrintTargetRegisters(&regs);





    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Prepare sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    listen(serverSocket, 3);

    printf("Waiting for incoming connections...\n");

    // Accept an incoming connection
    clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Connection accepted.\n");


    InitializeConnection(clientSocket);
    
    // Close sockets
    closesocket(clientSocket);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}

void InitializeConnection(SOCKET sock)
{
    char buffer[BUFSIZE];
    PacketContentEnum packetState = UNDEFINED;
    int recvSize;
    CommandContentsStruct cmdContents = {0};
    unsigned long isStopped = 0;
    u_long mode = 1;  // 1 to enable non-blocking mode


    /* receive '+' from client  */ 
    recvSize = recv(sock, buffer, BUFSIZE, 0);

    /* this code block handles supported features part*/
    if(recvSize == 1 && buffer[0] == '+')
    {
        recvSize = recv(sock, buffer, BUFSIZE, 0);
        packetState = ParsePacket(buffer ,recvSize, &cmdContents);
        SendAck(sock);
        SendResponse(packetState, sock, &cmdContents);
    }

    /* We have sent supported features, we expect the client to accept them*/
    recvSize = recv(sock, buffer, BUFSIZE, 0);

    /* Client will ask us to send an empty response, we will*/
    if(recvSize == 1 && buffer[0] == '+')
    {
        recvSize = recv(sock, buffer, BUFSIZE, 0);
        buffer[recvSize] = 0;
        packetState = ParsePacket(buffer ,recvSize, &cmdContents);
        SendAck(sock);
        SendResponse(packetState, sock, &cmdContents);

    }

    /* We have sent empty response features, we expect the client to ack it*/
    recvSize = recv(sock, buffer, BUFSIZE, 0);

    /* We expect client to send noack mode */
    if(recvSize == 1 && buffer[0] == '+')
    {
        recvSize = recv(sock, buffer, BUFSIZE, 0);
        buffer[recvSize] = 0;
        packetState = ParsePacket(buffer ,recvSize, &cmdContents);
        SendAck(sock);
        SendResponse(packetState, sock, &cmdContents);

    }

    /* We have sent empty response features, we expect the client to ack it*/
    recvSize = recv(sock, buffer, BUFSIZE, 0);
    if(recvSize == 1 && buffer[0] == '+')
    {
        while(1)
        {
            printf("Poll for receive \n");
            recvSize = recv(sock, buffer, BUFSIZE, 0);
            buffer[recvSize] = 0;
            packetState = ParsePacket(buffer ,recvSize, &cmdContents);
            SendResponse(packetState, sock, &cmdContents);


            if(packetState == RunTheTarget)
            {
                printf("Run the target is asked \n");
                mode = 1;
                if (ioctlsocket(sock, FIONBIO, &mode) != 0) 
                {
                    printf("ioctlsocket failed: %d\n", WSAGetLastError());
                    closesocket(sock);
                    WSACleanup();
                }

                while(packetState == RunTheTarget)
                {
                
                    recvSize = recv(sock, buffer, BUFSIZE, 0);
                    /* Check if we received a SIGINT to halt the target */
                    if(recvSize != -1)
                    {
                        buffer[recvSize] = 0;
                        packetState = ParsePacket(buffer ,recvSize, &cmdContents);
                        printf("New packet available \n");

                    }
                    else /* if recvSize == -1 */
                    {   
                        
                        PollForStoppedMessage(handleSerial, &isStopped, &regs);
                        if(isStopped == 1)
                        {
                            packetState = ReportTargetHasStopped;
                        }
                        else
                        {
                            //InsertBreakPoint(handleSerial, 0x20000b02);
                        }
                    }


                }

                mode = 0;
                if (ioctlsocket(sock, FIONBIO, &mode) != 0) 
                {
                    printf("ioctlsocket failed: %d\n", WSAGetLastError());
                    closesocket(sock);
                    WSACleanup();
                }
                SendResponse(packetState, sock, &cmdContents);

            }
            
        }

    }
    
}
