#pragma once

#include "Common.h"
#include <sys/time.h>   // Required for measuring time
#include <pthread.h>
#include <semaphore.h>
#include <string>

using namespace std;

class ProcessingNode {
protected:
    ProcessingNode* m_cNextProcessingNode;
    MessageQueue m_cMessageQueue;

    sem_t m_hSemaphore;
    sem_t m_hStopEvent;
    pthread_t m_hWorkerThread;
    bool bInitialized;

    string m_sName;

    static void* ThreadProc(void* _pThreadArg);
public:
    ProcessingNode(int _iMailboxSize, string _sName);
    bool Init();
    bool DeInit();
    void SetNextProcessingNode(ProcessingNode* _pcNextProcessingNode);
    ProcessingNode* GetNextProcessingNode();
    bool ReceiveMessage(Message* _pcMessage);
    bool Stop();
    virtual void ProcessMessage(Message* _pcMessage);

    string Name();
    sem_t* GetMessageQueueSemaphore();
    sem_t* GetStopEvent();
    Message* ConsumeMessage();
};
