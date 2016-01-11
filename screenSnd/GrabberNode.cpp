#include "GrabberNode.h"
#include <iostream>
#include <string>

GrabberNode::GrabberNode(int _iMailboxSize, std::string _sName) : ProcessingNode(_iMailboxSize, _sName)
{

}

void GrabberNode::ProcessMessage(Message * _pcMessage)
{
    std::cout << m_sName << " ProcessMessage: Sleeping ..." << std::endl;
    Sleep(2000);
}
