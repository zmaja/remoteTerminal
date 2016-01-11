#pragma once
#include "ProcessingNode.h"

class GrabberNode : public ProcessingNode {
private:
public:
    GrabberNode(int _iMailboxSize, std::string _sName);
    void ProcessMessage(Message *_pcMessage);
};
