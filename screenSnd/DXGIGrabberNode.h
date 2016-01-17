#pragma once
#include "BasicGrabberNode.h"
#include "DXGIManager.h"

class DXGIGrabberNode : public BasicGrabberNode {
protected:
    int m_iScreenWidth;
    int m_iScreenHeight;

    DXGIManager *m_pcDXGIManager;
    RECT m_rcDim;

    HANDLE m_hMutex;
public:
    DXGIGrabberNode(int _iMailboxSize, std::string _sName);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message *_pcMessage);
    void DisplayResolutionChanged();
    void ReInit();
};