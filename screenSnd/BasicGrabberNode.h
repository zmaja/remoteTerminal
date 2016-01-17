#pragma once

#include "ProcessingNode.h"

class BasicGrabberNode : public ProcessingNode {
protected:
public:
    BasicGrabberNode(int _iMailboxSize, std::string _sName);
    virtual void ProcessMessage(Message *_pcMessage);
    virtual void DisplayResolutionChanged();
};