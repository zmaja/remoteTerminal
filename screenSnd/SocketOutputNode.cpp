#include <winsock2.h>

#include <iostream>
#include <string>

#include "SocketOutputNode.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

SocketOutputNode::SocketOutputNode(int _iMailboxSize, std::string _sName, std::string _sIPAddress, int _iPort) : ProcessingNode(_iMailboxSize, _sName)
{
    m_sIPAddress = _sIPAddress;
    m_iPort = _iPort;
    m_iMaxMsgSize = 0;

    SendSocket = INVALID_SOCKET;
}

bool SocketOutputNode::Init()
{
    //----------------------
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        std::cout << m_sName << " Init: WSAStartup failed with error: " << iResult << std::endl;
        return false;
    }

    //---------------------------------------------
    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET)
    {
        std::cout << m_sName << " Init: socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "192.168.1.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(m_iPort);
    RecvAddr.sin_addr.s_addr = inet_addr(m_sIPAddress.c_str());

    int iOptVal;
    int iOptLen = sizeof(int);

    if (getsockopt(SendSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR)
    {
        m_iMaxMsgSize = iOptVal;
    }
    else
    {
        std::cout << m_sName << " Init: getsockopt failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }


    if (ProcessingNode::Init() == FALSE)
        return false;

    return true;
}

bool SocketOutputNode::DeInit()
{
    bool bRetVal = ProcessingNode::DeInit();

    int iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << m_sName << " Deinit: closesocket failed with error: " << WSAGetLastError() << std::endl;
        bRetVal = false;
    }

    WSACleanup();

    return bRetVal;
}

void SocketOutputNode::ProcessMessage(Message * _pcMessage)
{
    //std::cout << m_sName << " ProcessMessage: ProcessMessage ..." << std::endl;

    int start = GetTickCount();

    int iResult;
    int iPayloadSize = _pcMessage->GetValidBytes();
    char *pchPayloadAddress = (char *)_pcMessage->GetPayloadAddress();
    if (iPayloadSize > m_iMaxMsgSize)
    {
        while (iPayloadSize > m_iMaxMsgSize)
        {
            iResult = sendto(SendSocket,
                (const char *)pchPayloadAddress, m_iMaxMsgSize, 0, (SOCKADDR *)& RecvAddr, sizeof(RecvAddr));
            if (iResult == SOCKET_ERROR)
            {
                std::cout << m_sName << " ProcessMessage: sendto failed with error: " << WSAGetLastError() << std::endl;
            }
            pchPayloadAddress += m_iMaxMsgSize;
            iPayloadSize -= m_iMaxMsgSize;
        }

    }
    iResult = sendto(SendSocket,
        (const char *)pchPayloadAddress, iPayloadSize, 0, (SOCKADDR *)& RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR)
    {
        std::cout << m_sName << " ProcessMessage: sendto failed with error: " << WSAGetLastError() << std::endl;
    }

    int end = GetTickCount();
    std::cout << m_sName << " ProcessMessage: TotalSendTime: " << end - start << "ms" << std::endl;

    _pcMessage->SetEndTime(GetTickCount());

    std::cout << m_sName << " ProcessMessage: TotalProcessingTime: " << _pcMessage->TotalProcessingTime() << "ms" << std::endl;
}