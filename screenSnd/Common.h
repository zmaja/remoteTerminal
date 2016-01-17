#pragma once

#include <queue>
#include <iostream>

#ifndef NDEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

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
    char *m_pchPayload;
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
    void *GetPayloadAddress();
    int TotalProcessingTime();
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

enum MessageIdentificator { MSG_FRAME = 2, MSG_KEYBOARD, MSG_MOUSE };
enum MouseEventType { MOUSE_LEFT = 1, MOUSE_RIGHT, MOUSE_MOVE };

typedef struct _tPosition
{
    short x;
    char y;
} tPosition;

typedef struct _tKeyboardEvent
{
    MessageIdentificator identificator;
    char pressed;
    char virtual_key_code;
} tKeyboardEvent;

typedef struct _tMouseEvent
{
    MessageIdentificator identificator;
    MouseEventType event;
    union {
        tPosition pos;
        char pressed;
    };
} tMouseEvent;
