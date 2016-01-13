#pragma once
#include "ProcessingNode.h"

class GrabberNode : public ProcessingNode {
protected:
    int m_iScreenWidth;
    int m_iScreenHeight;
    HWND m_hDesktopWnd;
    HDC m_hDesktopDC;
    HDC m_hCaptureDC;
    HBITMAP m_hCaptureBitmap;
    BITMAPINFOHEADER m_bmi;

    HANDLE m_hMutex;
public:
    GrabberNode(int _iMailboxSize, std::string _sName);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message *_pcMessage);
    void DisplayResolutionChanged();
};