#pragma once

#include <windows.h>
#include <string>

class SocketReceiverNode {
protected:
    HANDLE m_hStopEvent;
    HANDLE m_hWorkerThread;
    bool m_bInitialized;

    int m_iPort;
    WSADATA wsaData;
    SOCKET ReceiveSocket;

    std::string m_sName;

    void SocketReceiverNode::GenerateKeyPressEvent(int _iVirtualKeyCode);
    void SocketReceiverNode::GenerateKeyReleaseEvent(int _iVirtualKeyCode);
    void SocketReceiverNode::GenerateLeftMousePressEvent();
    void SocketReceiverNode::GenerateLeftMouseReleaseEvent();
    void SocketReceiverNode::GenerateRightMousePressEvent();
    void SocketReceiverNode::GenerateRightMouseReleaseEvent();
    void SocketReceiverNode::GenerateMouseMoveEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight);
    void SocketReceiverNode::GenerateLeftMouseClickEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight);
    void SocketReceiverNode::GenerateRightMouseClickEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight);

    int recv_timeout(SOCKET s, char *buf, int len, int flags, int timeout);

    void CheckSocketForIncommingMessages();

    static DWORD WINAPI ThreadProc(LPVOID);
public:
    SocketReceiverNode(int _iMailboxSize, std::string _sName, int _iPort);
    bool Init();
    bool DeInit();
    bool Stop();

    std::string Name();
    HANDLE GetStopEvent();
};