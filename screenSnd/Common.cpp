#include "Common.h"

Message::Message()
{
    m_ePixelFormat = PF_UNDEF;
    p_chPayload = new char[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*MAX_BPP >> 3];
}

Message::~Message()
{
    m_ePixelFormat = PF_UNDEF;
    delete[] p_chPayload;
}

MessageQueue::MessageQueue(int _iMailboxSize)
{
    if (_iMailboxSize > MAX_MAILBOX_SIZE)
    {
        m_iMailboxSize = MAX_MAILBOX_SIZE;
    }
    else if (_iMailboxSize <= 0)
    {
        m_iMailboxSize = 1;
    }
    else {
        m_iMailboxSize = _iMailboxSize;
    }
}

bool MessageQueue::QueueMessage(Message *_pcMessage)
{
    if (m_cQueue.size() == m_iMailboxSize)
    {
        return false;
    }
    m_cQueue.push(_pcMessage);
    return true;
}

Message *MessageQueue::DequeueMessage()
{
    if (m_cQueue.empty())
    {
        return NULL;
    }
    Message *p_cRetVal = m_cQueue.front();
    m_cQueue.pop();
    return p_cRetVal;
}

int MessageQueue::MailboxSize()
{
    return m_iMailboxSize;
}
