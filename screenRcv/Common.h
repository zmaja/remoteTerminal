#pragma once

#include <queue>

#define MAX_MAILBOX_SIZE 20
#define MAX_IMAGE_WIDTH 1920
#define MAX_IMAGE_HEIGHT 1080
#define MAX_BPP 32

enum ePixelFormat { PF_UNDEF, PF_ARGB, PF_JPEG };

class Message {
private:
    ePixelFormat m_ePixelFormat;
    int m_iWidth;
    int m_iHeight;
    int m_iStartTime;
    int m_iEndTime;
    char* m_pchPayload;
    int m_iValidBytes;
public:
    Message();
    void SetWidth(int _iWidth);
    void SetHeight(int _iHeight);
    void SetStartTime(int _iStartTime);
    void SetEndTime(int _iEndTime);
    void SetValidBytes(int _iValidBytes);
    int GetWidth();
    int GetHeight();
    int GetStartTime();
    int GetEndTime();
    int GetValidBytes();
    void* GetPayloadAddress();
    int TotalProcessingTime();
    ~Message();
};

class MessageQueue {
private:
    int m_iMailboxSize;
    std::queue<Message*> m_cQueue;
public:
    MessageQueue(int _iMailboxSize);
    bool QueueMessage(Message* _cMessage);
    Message* DequeueMessage();
    int MailboxSize();
};
