#pragma once
#include "ProcessingNode.h"

class SocketOutputNode : public ProcessingNode {
protected:
    std::string m_sIPAddress;
    int m_iPort;
    int m_iMaxMsgSize;

    WSADATA wsaData;
    SOCKET SendSocket;
    sockaddr_in RecvAddr;
public:
    SocketOutputNode(int _iMailboxSize, std::string _sName, std::string _sIPAddress, int _iPort);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message *_pcMessage);
};