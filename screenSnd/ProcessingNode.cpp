#include <iostream>
#include <string>

#include "ProcessingNode.h"

ProcessingNode::ProcessingNode(int _iMailboxSize, std::string _sName):m_cMessageQueue(_iMailboxSize)
{
    m_sName = _sName;

    m_hSemaphore = NULL;
    m_hStopEvent = NULL;
    m_hWorkerThread = NULL;

    bInitialized = false;
}

bool ProcessingNode::Init()
{
    if (bInitialized)
        return false;
    m_hSemaphore = CreateSemaphore(
        NULL,                            // default security attributes
        0,                               // initial count
        m_cMessageQueue.MailboxSize(),   // maximum count
        NULL);                           // unnamed semaphore
    if (m_hSemaphore == NULL)
    {
        std::cout << m_sName << " Init: CreateSemaphore error" << std::endl;
        return false;
    }
    m_hStopEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("StopEvent")   // object name
        );
    if (m_hStopEvent == NULL)
    {
        std::cout << m_sName << " Init: CreateEvent error" << std::endl;
        CloseHandle(m_hSemaphore);
        return false;
    }

    m_hWorkerThread = CreateThread(
        NULL,         // default security attributes
        0,            // default stack size
        (LPTHREAD_START_ROUTINE)ThreadProc,
        this,         // no thread function arguments
        0,            // default creation flags
        0); // receive thread identifier
    if (m_hWorkerThread == NULL)
    {
        std::cout << m_sName << " Init: CreateThread error" << std::endl;
        CloseHandle(m_hSemaphore);
        CloseHandle(m_hStopEvent);
        return false;
    }

    bInitialized = true;

    return true;
}

bool ProcessingNode::DeInit()
{
    if (!bInitialized)
        return false;
    if (m_hWorkerThread)
    {
        WaitForSingleObject(m_hWorkerThread, INFINITE);
        CloseHandle(m_hWorkerThread);
    }
    if (m_hSemaphore)
    {
        CloseHandle(m_hSemaphore);
    }
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    m_hSemaphore = NULL;
    m_hStopEvent = NULL;
    m_hWorkerThread = NULL;

    bInitialized = false;

    return true;
}

void ProcessingNode::SetNextProcessingNode(ProcessingNode *_pcNextProcessingNode)
{
    m_cNextProcessingNode = _pcNextProcessingNode;
}

ProcessingNode * ProcessingNode::GetNextProcessingNode()
{
    return m_cNextProcessingNode;
}

bool ProcessingNode::ReceiveMessage(Message *_pcMessage)
{
    if (!bInitialized)
        return false;
    if (m_cMessageQueue.QueueMessage(_pcMessage))
    {
        if (!ReleaseSemaphore(
            m_hSemaphore,  // handle to semaphore
            1,             // increase count by one
            NULL))         // not interested in previous count
        {
            std::cout << m_sName << " ReceiveMessage: ReleaseSemaphore error" << std::endl;
            return false;
        }
    }
    else {
        std::cout << m_sName << " ReceiveMessage: MessageQueue is full. This shouldn't happen" << std::endl;
        return false;
    }

    return true;
}

bool ProcessingNode::Stop()
{
    if (!bInitialized)
        return false;
    if (!SetEvent(m_hStopEvent))
    {
        std::cout << m_sName << " Stop: SetEvent failed " + GetLastError() << std::endl;
        return false;
    }
    return true;
}

void ProcessingNode::ProcessMessage(Message * _pcMessage)
{
    if (bInitialized)
    {
        std::cout << m_sName << " ProcessMessage: Sleeping ..." << std::endl;
        Sleep(2000);
    }
}

std::string ProcessingNode::Name()
{
    return m_sName;
}

HANDLE ProcessingNode::GetMessageQueueSemaphore()
{
    return m_hSemaphore;
}

HANDLE ProcessingNode::GetStopEvent()
{
    return m_hStopEvent;
}

Message * ProcessingNode::ConsumeMessage()
{
    if (bInitialized)
    {
        Message *pcMessage = m_cMessageQueue.DequeueMessage();
        if (pcMessage != NULL)
        {
            return pcMessage;
        }
        else {
            std::cout << m_sName << " ConsumeMessage: MessageQueue is empty. This shouldn't happen" << std::endl;
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

DWORD WINAPI ProcessingNode::ThreadProc(LPVOID lpParam)
{
    ProcessingNode *pcProcessingNode = (ProcessingNode *)lpParam;
    if (pcProcessingNode == NULL)
    {
        std::cout << pcProcessingNode->Name() << " ThreadProc: pcProcessingNode == NULL" << std::endl;
    }
    std::cout << pcProcessingNode->Name() << " ThreadProc: Entered" << std::endl;

    bool bContinue = true;

    while (bContinue)
    {
        DWORD dwEvent;
        HANDLE hEvents[2];
        hEvents[0] = pcProcessingNode->GetStopEvent();
        hEvents[1] = pcProcessingNode->GetMessageQueueSemaphore();

        dwEvent = WaitForMultipleObjects(
            2,           // number of objects in array
            hEvents,     // array of objects
            FALSE,       // wait for any object
            INFINITE);   // blocking wait

        // The return value indicates which event is signaled

        switch (dwEvent)
        {
            // hEvents[0] was signaled (Stop event signalled)
        case WAIT_OBJECT_0 + 0:
            //std::cout << pcProcessingNode->Name() << " ThreadProc: Received stop" << std::endl;
            return 0;
            break;

            // hEvents[1] was signaled (MessageQueue semaphore)
        case WAIT_OBJECT_0 + 1:
        {
            //std::cout << pcProcessingNode->Name() << " ThreadProc: Received message" << std::endl;
            Message *pcMessage = pcProcessingNode->ConsumeMessage();
            if (pcMessage != NULL)
            {
                pcProcessingNode->ProcessMessage(pcMessage);
                pcProcessingNode->GetNextProcessingNode()->ReceiveMessage(pcMessage);
            }
            else {
                std::cout << pcProcessingNode->Name() << " ThreadProc: MessageQueue is empty. This shouldn't happen" << std::endl;
            }
            break;
        }

        case WAIT_TIMEOUT:
            std::cout << pcProcessingNode->Name() << " ThreadProc: Wait timeout" << std::endl;
            break;

            // Return value is invalid.
        default:
            std::cout << pcProcessingNode->Name() << " ThreadProc: Wait error: " + GetLastError() << std::endl;
            ExitProcess(0);
        }

    };

    return 0;
}