#include <iostream>
#include <errno.h> // Required for errno (duuuuhh!)
#include <unistd.h> // Required for sleep()

#include "ProcessingNode.h"

ProcessingNode::ProcessingNode(int _iMailboxSize, string _sName):m_cMessageQueue(_iMailboxSize)
{
    m_sName = _sName;

    bInitialized = false;
}

bool ProcessingNode::Init()
{
    if (bInitialized)
        return false;
     
    if (sem_init(&m_hSemaphore, 0, 0) == -1)
    {
        cout << m_sName << " Init: CreateSemaphore error" << endl;
        return false;
    }
    
    if (sem_init(&m_hStopEvent, 0, 0) == -1) {
        cout << m_sName << " Init: CreateEvent error" << std::endl;
        sem_destroy(&m_hSemaphore);
        return false;
    }

    int iRC = pthread_create(&m_hWorkerThread, 0, ThreadProc, (void*)this);
    if (iRC) {
        cout << "Init: CreateThread error; return code from pthread_create() is" << iRC << endl;
        sem_destroy(&m_hSemaphore);
        sem_destroy(&m_hStopEvent);
        return false;
    }

    bInitialized = true;
    return true;
}

bool ProcessingNode::DeInit()
{
    if (!bInitialized)
        return false;
    
    void* vJoinStatus;
    int iRC = pthread_join(m_hWorkerThread, &vJoinStatus);
    if (iRC) {
        cout << "ERROR; return code from pthread_join() is" << iRC << endl;
        return -1;
    }

    sem_destroy(&m_hSemaphore);

    sem_destroy(&m_hStopEvent);

    bInitialized = false;

    return true;
}

void ProcessingNode::SetNextProcessingNode(ProcessingNode* _pcNextProcessingNode)
{
    m_cNextProcessingNode = _pcNextProcessingNode;
}

ProcessingNode* ProcessingNode::GetNextProcessingNode()
{
    return m_cNextProcessingNode;
}

bool ProcessingNode::ReceiveMessage(Message* _pcMessage)
{
    if (!bInitialized)
        return false;
    if (m_cMessageQueue.QueueMessage(_pcMessage)) {
        if(sem_post(&m_hSemaphore) == -1) {
            cout << m_sName << " ReceiveMessage: ReleaseSemaphore error" << endl;
            return false;
        }
    } else {
        cout << m_sName << " ReceiveMessage: MessageQueue is full. This shouldn't happen" << std::endl;
        return false;
    }

    return true;
}

bool ProcessingNode::Stop()
{
    if (!bInitialized)
        return false;
    if(sem_post(&m_hStopEvent) == -1) {
        cout << m_sName << " Stop: SetEvent failed " << endl;
        return false;
    }
    return true;
}

void ProcessingNode::ProcessMessage(Message* _pcMessage)
{
    if (bInitialized)
    {
        cout << m_sName << " ProcessMessage: Sleeping ..." << endl;
        sleep(1);
    }
}

string ProcessingNode::Name()
{
    return m_sName;
}

sem_t* ProcessingNode::GetMessageQueueSemaphore()
{
    return &m_hSemaphore;
}

sem_t* ProcessingNode::GetStopEvent()
{
    return &m_hStopEvent;
}

Message* ProcessingNode::ConsumeMessage()
{
    if (bInitialized)
    {
        Message* pcMessage = m_cMessageQueue.DequeueMessage();
        if (pcMessage != 0)
        {
            return pcMessage;
        }
        else {
            cout << m_sName << " ConsumeMessage: MessageQueue is empty. This shouldn't happen" << endl;
            return 0;
        }
    }
    else {
        return 0;
    }
}

void* ProcessingNode::ThreadProc(void* _pThreadArg)
{
    int iRC;
    int errsv;
    ProcessingNode* pcProcessingNode = (ProcessingNode*)_pThreadArg;
    sem_t* hSemStopEvent = pcProcessingNode->GetStopEvent();
    sem_t* hMsgQueueSem = pcProcessingNode->GetMessageQueueSemaphore();
    
    if (pcProcessingNode == 0)
    {
        cout << pcProcessingNode->Name() << " ThreadProc: pcProcessingNode == null" << endl;
    }
    cout << pcProcessingNode->Name() << " ThreadProc: Entered" << endl;

    bool bContinue = true;
    struct timespec semWaitTime;
    
    while (bContinue) {
        iRC = sem_trywait(hSemStopEvent); // Check if someone stopped me.
        errsv = errno;
        if((iRC == -1) && (errsv == EAGAIN)) {
            // Stop signal didn't came, wait 0.5s for incoming message.
            clock_gettime(CLOCK_REALTIME, &semWaitTime);
            semWaitTime.tv_nsec += 500000000;
            if (semWaitTime.tv_nsec >= 1000000000) {
                semWaitTime.tv_sec += 1;
                semWaitTime.tv_nsec -= 1000000000;
            }
            iRC = sem_timedwait(hMsgQueueSem, &semWaitTime);
            errsv = errno;
            if((iRC == -1) && (errsv == ETIMEDOUT)) {
                // If no message came in 0.5s, check if someone stopped me.
                continue;
            } else if(iRC == 0) {
                // Message came, go deal with it.
                cout << pcProcessingNode->Name() << " ThreadProc: Received message" << endl;
                Message* pcMessage = pcProcessingNode->ConsumeMessage();
                if (pcMessage != 0) {
                    pcProcessingNode->ProcessMessage(pcMessage);
                    pcProcessingNode->GetNextProcessingNode()->ReceiveMessage(pcMessage);
                } else {
                    cout << pcProcessingNode->Name() << " ThreadProc: MessageQueue is empty. This shouldn't happen" << endl;
                }
                continue;
            }
        } else if(iRC == 0) {
            cout << pcProcessingNode->Name() << " ThreadProc: Received STOP" << endl;
            bContinue = false;
            continue;
        }
    }

    return 0;
}
