#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include "ProcessingNode.h"

using namespace std;

class SocketInputNode : public ProcessingNode {
protected:
    int m_iPort;
    int m_iMyMaxMsgSize;
    int m_iSourceMaxMsgSize;
    int m_iSocket;
    sockaddr_in m_sourceAddr;
    sockaddr_in m_myAddr;
public:
    SocketInputNode(int _iMailboxSize, string _sName, int _iPort);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message* _pcMessage);
};
