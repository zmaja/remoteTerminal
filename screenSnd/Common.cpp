#include "Common.h"

Message::Message()
{
    m_ePixelFormat = PF_UNDEF;
    m_iWidth = -1;
    m_iHeight = -1;
    m_iValidBytes = 0;
    m_pchPayload = new char[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*MAX_BPP >> 3];
}

void Message::SetWidth(int _iWidth)
{
    m_iWidth = _iWidth;
}

void Message::SetHeight(int _iHeight)
{
    m_iHeight = _iHeight;
}

void Message::SetStartTime(int _iStartTime)
{
    m_iStartTime = _iStartTime;
}

void Message::SetEndTime(int _iEndTime)
{
    m_iEndTime = _iEndTime;
}

void Message::SetValidBytes(int _iValidBytes)
{
    m_iValidBytes = _iValidBytes;
}

int Message::GetWidth()
{
    return m_iWidth;
}

int Message::GetHeight()
{
    return m_iHeight;
}

int Message::GetStartTime()
{
    return m_iStartTime;
}

int Message::GetEndTime()
{
    return m_iEndTime;
}

int Message::GetValidBytes()
{
    return m_iValidBytes;
}

void * Message::GetPayloadAddress()
{
    return m_pchPayload;
}

int Message::TotalProcessingTime()
{
    return m_iEndTime - m_iStartTime;
}

Message::~Message()
{
    m_ePixelFormat = PF_UNDEF;
    delete[] m_pchPayload;
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
