#include <iostream>

#include "ProcessingNode.h"

ProcessingNode::ProcessingNode(int _iMailboxSize, std::string _sName):m_cMessageQueue(_iMailboxSize)
{
    m_sName = _sName;

    m_hSemaphore = NULL;
    m_hStopEvent = NULL;
    m_hWorkerThread = NULL;

    m_bInitialized = false;
}

bool ProcessingNode::Init()
{
    if (m_bInitialized)
        return false;
    m_hSemaphore = CreateSemaphore(
        NULL,                            // default security attributes
        0,                               // initial count
        m_cMessageQueue.MailboxSize(),   // maximum count
        NULL);                           // unnamed semaphore
    if (m_hSemaphore == NULL)
    {
        DEBUG_MSG(m_sName << " Init: CreateSemaphore error");
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
        DEBUG_MSG(m_sName << " Init: CreateEvent error");
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
        DEBUG_MSG(m_sName << " Init: CreateThread error");
        CloseHandle(m_hSemaphore);
        CloseHandle(m_hStopEvent);
        return false;
    }

    m_bInitialized = true;

    return true;
}

bool ProcessingNode::DeInit()
{
    if (!m_bInitialized)
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

    m_bInitialized = false;

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
    if (!m_bInitialized)
        return false;
    if (m_cMessageQueue.QueueMessage(_pcMessage))
    {
        if (!ReleaseSemaphore(
            m_hSemaphore,  // handle to semaphore
            1,             // increase count by one
            NULL))         // not interested in previous count
        {
            DEBUG_MSG(m_sName << " ReceiveMessage: ReleaseSemaphore error");
            return false;
        }
    }
    else {
        DEBUG_MSG(m_sName << " ReceiveMessage: MessageQueue is full. This shouldn't happen");
        return false;
    }

    return true;
}

bool ProcessingNode::Stop()
{
    if (!m_bInitialized)
        return false;
    if (!SetEvent(m_hStopEvent))
    {
        DEBUG_MSG(m_sName << " Stop: SetEvent failed " + GetLastError());
        return false;
    }
    return true;
}

void ProcessingNode::ProcessMessage(Message * _pcMessage)
{
    if (m_bInitialized)
    {
        DEBUG_MSG(m_sName << " ProcessMessage: Sleeping ...");
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
    if (m_bInitialized)
    {
        Message *pcMessage = m_cMessageQueue.DequeueMessage();
        if (pcMessage != NULL)
        {
            return pcMessage;
        }
        else {
            DEBUG_MSG(m_sName << " ConsumeMessage: MessageQueue is empty. This shouldn't happen");
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
        DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: pcProcessingNode == NULL");
    }
    DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: Entered");

    bool bContinue = true;

    DWORD dwEvent;
    HANDLE hEvents[2];
    hEvents[0] = pcProcessingNode->GetStopEvent();
    hEvents[1] = pcProcessingNode->GetMessageQueueSemaphore();

    while (bContinue)
    {
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
            //DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: Received stop");
            bContinue = false;
            break;

            // hEvents[1] was signaled (MessageQueue semaphore)
        case WAIT_OBJECT_0 + 1:
        {
            //DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: Received message");
            Message *pcMessage = pcProcessingNode->ConsumeMessage();
            if (pcMessage != NULL)
            {
                pcProcessingNode->ProcessMessage(pcMessage);
                pcProcessingNode->GetNextProcessingNode()->ReceiveMessage(pcMessage);
            }
            else {
                DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: MessageQueue is empty. This shouldn't happen");
            }
            break;
        }

        case WAIT_TIMEOUT:
            DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: Wait timeout");
            break;

            // Return value is invalid.
        default:
            DEBUG_MSG(pcProcessingNode->Name() << " ThreadProc: Wait error: " + GetLastError());
            ExitProcess(0);
        }

    };

    return 0;
}