#include <iostream>
#include <string>

#include "BasicGrabberNode.h"

BasicGrabberNode::BasicGrabberNode(int _iMailboxSize, std::string _sName) : ProcessingNode(_iMailboxSize, _sName)
{
}

void BasicGrabberNode::ProcessMessage(Message * _pcMessage)
{
    if (m_bInitialized)
    {
        DEBUG_MSG(m_sName << " ProcessMessage: Sleeping ...");
        Sleep(2000);
    }
}

void BasicGrabberNode::DisplayResolutionChanged()
{
    DEBUG_MSG(m_sName << " DisplayResolutionChanged: Sleeping ...");
}