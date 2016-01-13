#include <iostream>
#include <string>

#include "GrabberNode.h"

GrabberNode::GrabberNode(int _iMailboxSize, std::string _sName) : ProcessingNode(_iMailboxSize, _sName)
{

}

bool GrabberNode::Init()
{
    if (ProcessingNode::Init() == FALSE)
        return false;

    m_hMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (m_hMutex == NULL)
    {
        std::cout << m_sName << " Init: CreateMutex error: " << GetLastError() << std::endl;
        return false;
    }

    m_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    m_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    m_hDesktopWnd = GetDesktopWindow();
    m_hDesktopDC = GetDC(m_hDesktopWnd);
    m_hCaptureDC = CreateCompatibleDC(m_hDesktopDC);
    m_hCaptureBitmap =
        CreateCompatibleBitmap(m_hDesktopDC, m_iScreenWidth, m_iScreenHeight);
    SelectObject(m_hCaptureDC, m_hCaptureBitmap);

    m_bmi.biSize = sizeof(BITMAPINFOHEADER);
    m_bmi.biPlanes = 1;
    m_bmi.biBitCount = 24;
    m_bmi.biWidth = m_iScreenWidth;
    m_bmi.biHeight = -m_iScreenHeight;
    m_bmi.biCompression = BI_RGB;
    m_bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

    return true;
}

bool GrabberNode::DeInit()
{
    bool bRetVal = ProcessingNode::DeInit();

    ReleaseDC(m_hDesktopWnd, m_hDesktopDC);
    DeleteDC(m_hCaptureDC);
    DeleteObject(m_hCaptureBitmap);

    CloseHandle(m_hMutex);

    return bRetVal;
}

void GrabberNode::ProcessMessage(Message * _pcMessage)
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
            //std::cout << m_sName << " ProcessMessage: ProcessMessage ..." << std::endl;
            int start = GetTickCount();
            _pcMessage->SetStartTime(GetTickCount());
            _pcMessage->SetEndTime(-1);
            _pcMessage->SetWidth(m_iScreenWidth);
            _pcMessage->SetHeight(m_iScreenHeight);
            _pcMessage->SetValidBytes(m_iScreenWidth * m_iScreenHeight * 3);
            BitBlt(m_hCaptureDC, 0, 0, m_iScreenWidth, m_iScreenHeight,
                m_hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);
            GetDIBits(m_hCaptureDC, m_hCaptureBitmap, 0, m_iScreenHeight,
                _pcMessage->GetPayloadAddress(), (BITMAPINFO*)&m_bmi, DIB_RGB_COLORS);
            int end = GetTickCount();
            std::cout << m_sName << " ProcessMessage: TotalGrabTime: " << end - start << "ms" << std::endl;
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

void GrabberNode::DisplayResolutionChanged()
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
            ReleaseDC(m_hDesktopWnd, m_hDesktopDC);
            DeleteDC(m_hCaptureDC);
            DeleteObject(m_hCaptureBitmap);

            m_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
            m_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
            m_hDesktopDC = GetDC(m_hDesktopWnd);
            m_hCaptureDC = CreateCompatibleDC(m_hDesktopDC);
            m_hCaptureBitmap =
                CreateCompatibleBitmap(m_hDesktopDC, m_iScreenWidth, m_iScreenHeight);
            SelectObject(m_hCaptureDC, m_hCaptureBitmap);

            m_bmi.biSize = sizeof(BITMAPINFOHEADER);
            m_bmi.biPlanes = 1;
            m_bmi.biBitCount = 24;
            m_bmi.biWidth = m_iScreenWidth;
            m_bmi.biHeight = -m_iScreenHeight;
            m_bmi.biCompression = BI_RGB;
            m_bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;
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
