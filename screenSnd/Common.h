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
    char *p_chPayload;
public:
    Message();
    ~Message();
};

class MessageQueue {
private:
    int m_iMailboxSize;
    std::queue<Message *> m_cQueue;
public:
    MessageQueue(int _iMailboxSize);
    bool QueueMessage(Message *_cMessage);
    Message* DequeueMessage();
    int MailboxSize();
};