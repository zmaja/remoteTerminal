#include <winsock2.h>

#include <iostream>

#include "Common.h"
#include "SocketReceiverNode.h"

SocketReceiverNode::SocketReceiverNode(std::string _sName, int _iPort)
{
    m_sName = _sName;

    m_hStopEvent = NULL;
    m_hWorkerThread = NULL;

	m_iMaxMsgSize = 0;
    m_iPort = _iPort;

    SOCKET ReceiveSocket = INVALID_SOCKET;

	SenderAddrSize = sizeof(SenderAddr);

    m_bInitialized = false;
}

bool SocketReceiverNode::Init()
{
    if (m_bInitialized)
        return false;

    //----------------------
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
		DEBUG_MSG(m_sName << " Init: WSAStartup failed with error: " << iResult);
        return false;
    }

    //---------------------------------------------
    // Create a socket for sending data
    ReceiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ReceiveSocket == INVALID_SOCKET)
    {
		DEBUG_MSG(m_sName << " Init: socket failed with error: " << WSAGetLastError());
        WSACleanup();
        return false;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "192.168.1.1")
    // and the specified port number.
    sockaddr_in myAddr;
    memset((char*)&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(m_iPort);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket.
    iResult = bind(ReceiveSocket, (SOCKADDR *)&myAddr, sizeof(myAddr));
    if (iResult == SOCKET_ERROR) {
		DEBUG_MSG(m_sName << " Init: bind failed with error: " << WSAGetLastError());
        closesocket(ReceiveSocket);
        WSACleanup();
        return false;
    }

	int iOptVal;
	int iOptLen = sizeof(int);

	if (getsockopt(ReceiveSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&iOptVal, &iOptLen) != SOCKET_ERROR)
	{
		m_iMaxMsgSize = iOptVal;
	}
	else
	{
		DEBUG_MSG(m_sName << " Init: getsockopt failed with error: " << WSAGetLastError());
		closesocket(ReceiveSocket);
		WSACleanup();
		return false;
	}

    m_hStopEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("StopEvent")   // object name
        );
    if (m_hStopEvent == NULL)
    {
		DEBUG_MSG(m_sName << " Init: CreateEvent error");
		closesocket(ReceiveSocket);
		WSACleanup();
        return false;
    }

    m_hWorkerThread = CreateThread(
        NULL,         // default security attributes
        0,            // default stack size
        (LPTHREAD_START_ROUTINE)ThreadProc,
        this,         // no thread function arguments
        0,            // default creation flags
        0); // receive thread identifier
    if (m_hWorkerThread == NULL)
    {
		DEBUG_MSG(m_sName << " Init: CreateThread error");
        CloseHandle(m_hStopEvent);
		closesocket(ReceiveSocket);
		WSACleanup();
        return false;
    }

    m_bInitialized = true;

    return true;
}

bool SocketReceiverNode::DeInit()
{
    if (!m_bInitialized)
        return false;
    if (m_hWorkerThread)
    {
        WaitForSingleObject(m_hWorkerThread, INFINITE);
        CloseHandle(m_hWorkerThread);
    }
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    m_hStopEvent = NULL;
    m_hWorkerThread = NULL;

    m_bInitialized = false;

    return true;
}

bool SocketReceiverNode::Stop()
{
    if (!m_bInitialized)
        return false;
    if (!SetEvent(m_hStopEvent))
    {
		DEBUG_MSG(m_sName << " Stop: SetEvent failed " + GetLastError());
        return false;
    }
    return true;
}

std::string SocketReceiverNode::Name()
{
    return m_sName;
}

HANDLE SocketReceiverNode::GetStopEvent()
{
    return m_hStopEvent;
}

void SocketReceiverNode::GenerateKeyPressEvent(int _iVirtualKeyCode)
{
    INPUT ip;
    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = _iVirtualKeyCode; // virtual-key code
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateKeyReleaseEvent(int _iVirtualKeyCode)
{
    INPUT ip;
    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = KEYEVENTF_KEYUP; // virtual-key code
    ip.ki.dwFlags = 0; // 0 for key press

    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateLeftMousePressEvent()
{
    INPUT ip;
    // Set up a generic mouse event.
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    ip.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
    ip.mi.time = 0;
    ip.mi.dwExtraInfo = 0;
    ip.mi.dx = 0;
    ip.mi.dy = 0;

    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateLeftMouseReleaseEvent()
{
    INPUT ip;
    // Set up a generic mouse event.
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    ip.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
    ip.mi.time = 0;
    ip.mi.dwExtraInfo = 0;
    ip.mi.dx = 0;
    ip.mi.dy = 0;

    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateRightMousePressEvent()
{
    INPUT ip;
    // Set up a generic mouse event.
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    ip.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN);
    ip.mi.time = 0;
    ip.mi.dwExtraInfo = 0;
    ip.mi.dx = 0;
    ip.mi.dy = 0;

    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateRightMouseReleaseEvent()
{
    INPUT ip;
    // Set up a generic mouse event.
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    ip.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP);
    ip.mi.time = 0;
    ip.mi.dwExtraInfo = 0;
    ip.mi.dx = 0;
    ip.mi.dy = 0;

    SendInput(1, &ip, sizeof(INPUT));
}

void SocketReceiverNode::GenerateMouseMoveEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight)
{
    INPUT ip;
    // Set up a generic mouse event.
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    ip.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
    ip.mi.time = 0;
    ip.mi.dwExtraInfo = 0;
    ip.mi.dx = (_iXpos * (0xFFFF / _iDisplayWidth));
    ip.mi.dy = (_iYpos * (0xFFFF / _iDisplayHeight));

    SendInput(1, &ip, sizeof(INPUT));
}


void SocketReceiverNode::GenerateLeftMouseClickEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight)
{
    GenerateMouseMoveEvent(_iXpos, _iYpos, _iDisplayWidth, _iDisplayHeight);
    GenerateLeftMousePressEvent();
    GenerateLeftMouseReleaseEvent();
}

void SocketReceiverNode::GenerateRightMouseClickEvent(int _iXpos, int _iYpos, int _iDisplayWidth, int _iDisplayHeight)
{
    GenerateMouseMoveEvent(_iXpos, _iYpos, _iDisplayWidth, _iDisplayHeight);
    GenerateRightMousePressEvent();
    GenerateRightMouseReleaseEvent();
}

int SocketReceiverNode::recvfrom_timeout(SOCKET s, char *buf, int len, int flags, int timeout)
{
    struct timeval timeout_value;
    timeout_value.tv_usec = timeout*1000;
    fd_set readFDs;

    FD_ZERO(&readFDs);
    FD_SET(s, &readFDs);
    int rv = select(0, &readFDs, NULL, NULL, &timeout_value);
    if (rv == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }
    else if (rv == 0)
    {
        return 0;
    }
    else
    {
        return recvfrom(s, buf, len, flags, (SOCKADDR *)& SenderAddr, &SenderAddrSize);
    }
}



void SocketReceiverNode::CheckSocketForIncommingMessages()
{
	//DEBUG_MSG(m_sName << " CheckSocketForIncommingMessages: Entered");
	char *pchRecvBuffer = new char[m_iMaxMsgSize];
	int iRet = recvfrom_timeout(ReceiveSocket, pchRecvBuffer, m_iMaxMsgSize, 0, 500); //wait on recvfrom for 100ms

	if (iRet > 0)
	{
		DEBUG_MSG(m_sName << " CheckSocketForIncommingMessages: Got something");
		ProcessReceivedMessage(pchRecvBuffer, iRet);
	}

	delete[]pchRecvBuffer;
}

void SocketReceiverNode::ProcessKeyboardEvent(tKeyboardEvent *_psKeyboardEvent)
{
	DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received keyboard event");
	if (_psKeyboardEvent->identificator != MSG_KEYBOARD)
	{
		DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad keyboard event");
	}
	else {
		switch (_psKeyboardEvent->pressed)
		{
		case 0:
			GenerateKeyReleaseEvent(_psKeyboardEvent->virtual_key_code);
			break;
		case 1:
			GenerateKeyPressEvent(_psKeyboardEvent->virtual_key_code);
			break;
		default:
			DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad keyboard press event");
			break;
		}
	}
}

void SocketReceiverNode::ProcessMouseEvent(tMouseEvent *_psMouseEvent) {
	DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received mouse event");
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	if (_psMouseEvent->identificator != MSG_MOUSE)
	{
		DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad mouse event");
	}
	else {
		switch (_psMouseEvent->event)
		{
		case MOUSE_LEFT:
			switch (_psMouseEvent->pressed)
			{
			case 0:
				GenerateLeftMouseReleaseEvent();
				break;
			case 1:
				GenerateLeftMousePressEvent();
				break;
			default:
				DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad left mouse event");
				break;
			}
			break;
		case MOUSE_RIGHT:
			switch (_psMouseEvent->pressed)
			{
			case 0:
				GenerateRightMouseReleaseEvent();
				break;
			case 1:
				GenerateRightMousePressEvent();
				break;
			default:
				DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad right mouse event");
				break;
			}
			break;
		case MOUSE_MOVE:
			iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
			if ((_psMouseEvent->pos.x > iScreenWidth) || (_psMouseEvent->pos.y > iScreenHeight))
			{
				DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad mouse move event");
			}
			else {
				GenerateMouseMoveEvent(_psMouseEvent->pos.x, _psMouseEvent->pos.y, iScreenWidth, iScreenHeight);
			}
			break;
		default:
			DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received bad mouse event");
			break;
		}
	}
}

void SocketReceiverNode::ProcessReceivedMessage(char *_pcIncommingMessage, int _iSize)
{
	switch (_iSize)
	{
	case sizeof(tKeyboardEvent) :
		ProcessKeyboardEvent((tKeyboardEvent *)_pcIncommingMessage);
		break;

	case sizeof(tMouseEvent) :
		ProcessMouseEvent((tMouseEvent *)_pcIncommingMessage);
		break;

	default:
		DEBUG_MSG(m_sName << " ProcessReceivedMessage: Received unknown message: ");
		break;
	}
}


DWORD WINAPI SocketReceiverNode::ThreadProc(LPVOID lpParam)
{
    SocketReceiverNode *pcSocketReceiverNode = (SocketReceiverNode *)lpParam;
    if (pcSocketReceiverNode == NULL)
    {
		DEBUG_MSG(pcSocketReceiverNode->Name() << " ThreadProc: pcSocketReceiverNode == NULL");
    }
	DEBUG_MSG(pcSocketReceiverNode->Name() << " ThreadProc: Entered");

    bool bContinue = true;

    while (bContinue)
    {
        HANDLE hStopEvent = pcSocketReceiverNode->GetStopEvent();

        DWORD dwEvent = WaitForSingleObject(hStopEvent, 0); //check and exit immediately

        switch (dwEvent)
        {
            // Stop event signalled
        case WAIT_OBJECT_0:
			DEBUG_MSG(pcSocketReceiverNode->Name() << " ThreadProc: Received stop");
            bContinue = false;
            break;

            // hEvents[1] was signaled (MessageQueue semaphore)
        case WAIT_TIMEOUT:
			//DEBUG_MSG(pcSocketReceiverNode->Name() << " ThreadProc: Wait timeout");
            break;

            // Return value is invalid.
        default:
			DEBUG_MSG(pcSocketReceiverNode->Name() << " ThreadProc: Wait error: " + GetLastError());
            ExitProcess(0);
        }

		pcSocketReceiverNode->CheckSocketForIncommingMessages();
    };

    return 0;
}