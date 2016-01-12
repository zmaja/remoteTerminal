#pragma once

#include "Common.h"
#include <windows.h>

class ProcessingNode {
protected:
    ProcessingNode *m_cNextProcessingNode;
    MessageQueue m_cMessageQueue;

    HANDLE m_hSemaphore;
    HANDLE m_hStopEvent;
    HANDLE m_hWorkerThread;
    bool bInitialized;

    std::string m_sName;

    static DWORD WINAPI ThreadProc(LPVOID);
public:
    ProcessingNode(int _iMailboxSize, std::string _sName);
    bool Init();
    bool DeInit();
    void SetNextProcessingNode(ProcessingNode *_pcNextProcessingNode);
    ProcessingNode *GetNextProcessingNode();
    bool ReceiveMessage(Message *_pcMessage);
    bool Stop();
    virtual void ProcessMessage(Message *_pcMessage);

    std::string Name();
    HANDLE GetMessageQueueSemaphore();
    HANDLE GetStopEvent();
    Message *ConsumeMessage();
};