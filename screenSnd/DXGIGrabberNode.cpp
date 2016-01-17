#include <iostream>
#include <string>

#include "DXGIGrabberNode.h"

DXGIGrabberNode::DXGIGrabberNode(int _iMailboxSize, std::string _sName) : BasicGrabberNode(_iMailboxSize, _sName)
{
    m_iScreenWidth = 0;
    m_iScreenHeight = 0;

    m_pcDXGIManager = NULL;
}

bool DXGIGrabberNode::Init()
{
    if (ProcessingNode::Init() == FALSE)
        return false;

    m_hMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (m_hMutex == NULL)
    {
        DEBUG_MSG(m_sName << " Init: CreateMutex error: " << GetLastError());
        return false;
    }

    CoInitialize(NULL);

    m_pcDXGIManager = new DXGIManager();

    m_pcDXGIManager->SetCaptureSource(CSDesktop);

    m_pcDXGIManager->GetOutputRect(m_rcDim);

    m_iScreenWidth = m_rcDim.right - m_rcDim.left;
    m_iScreenHeight = m_rcDim.bottom - m_rcDim.top;

    return true;
}

bool DXGIGrabberNode::DeInit()
{
    bool bRetVal = ProcessingNode::DeInit();

    CloseHandle(m_hMutex);

    delete m_pcDXGIManager;

    m_iScreenWidth = 0;
    m_iScreenHeight = 0;

    return bRetVal;
}

void DXGIGrabberNode::ReInit()
{
    m_pcDXGIManager->ReInit();
    m_pcDXGIManager->GetOutputRect(m_rcDim);
    m_iScreenWidth = m_rcDim.right - m_rcDim.left;
    m_iScreenHeight = m_rcDim.bottom - m_rcDim.top;
}

void DXGIGrabberNode::ProcessMessage(Message * _pcMessage)
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        m_hMutex,    // handle to mutex
        INFINITE);  // no time-out interval

    switch (dwWaitResult)
    {
        // The thread got ownership of the mutex
    case WAIT_OBJECT_0:
        __try
        {
            //DEBUG_MSG(m_sName << " ProcessMessage: ProcessMessage ...");
            int start = GetTickCount();
            
            HRESULT hr;
            do
            {
                hr = m_pcDXGIManager->GetOutputBits((BYTE *)_pcMessage->GetPayloadAddress(), m_rcDim);
                if ((hr != S_OK) || (m_iScreenWidth == 0) || (m_iScreenHeight == 0))
                {
                    DEBUG_MSG(m_sName << " ProcessMessage: DuplicateDesktop lost");
                    Sleep(300);
                    ReInit();
                }
            } while (hr != S_OK);
            _pcMessage->SetStartTime(start);
            _pcMessage->SetEndTime(-1);
            _pcMessage->SetWidth(m_iScreenWidth);
            _pcMessage->SetHeight(m_iScreenHeight);
            _pcMessage->SetValidBytes(m_iScreenWidth * m_iScreenHeight * 4);
            int end = GetTickCount();
            DEBUG_MSG(m_sName << " ProcessMessage: TotalGrabTime: " << end - start << "ms");
        }

        __finally
        {
            // Release ownership of the mutex object
            if (!ReleaseMutex(m_hMutex))
            {
                // Handle error.
            }
        }
        break;

        // The thread got ownership of an abandoned mutex
    case WAIT_ABANDONED:
        return;
    }
}

void DXGIGrabberNode::DisplayResolutionChanged()
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        m_hMutex,    // handle to mutex
        INFINITE);  // no time-out interval

    switch (dwWaitResult)
    {
        // The thread got ownership of the mutex
    case WAIT_OBJECT_0:
        __try {
        }

        __finally {
            // Release ownership of the mutex object
            if (!ReleaseMutex(m_hMutex))
            {
                // Handle error.
            }
        }
        break;

        // The thread got ownership of an abandoned mutex
    case WAIT_ABANDONED:
        return;
    }
}
