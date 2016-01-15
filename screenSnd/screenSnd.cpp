// screenSnd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>

#include "GrabberNode.h"
#include "JPEGCompressorNode.h"
#include "SocketOutputNode.h"
#include "SocketReceiverNode.h"
#include "screenSnd.h"

char szClassName[] = "WindowsApp";

GrabberNode *cGN;
JPEGCompressorNode *cPN;
SocketOutputNode *cSN;

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    GrabberNode *cGN = (GrabberNode*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (message)
    {
    case WM_KEYDOWN:
        PostQuitMessage(0);
        break;
    case WM_DISPLAYCHANGE:
        cGN->DisplayResolutionChanged();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_NCCREATE:
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int main()
{
	SocketReceiverNode *cSR = new SocketReceiverNode("SR", 8889);
	cSR->Init();

    GrabberNode *cGN = new GrabberNode(10, "GN");
    JPEGCompressorNode *cPN = new JPEGCompressorNode(10, "PN", 30, false);
    SocketOutputNode *cSN = new SocketOutputNode(10, "SN", "127.0.0.1", 8888);

    cGN->SetNextProcessingNode(cPN);
    cPN->SetNextProcessingNode(cSN);
    cSN->SetNextProcessingNode(cGN);

    cGN->Init();
    cPN->Init();
    cSN->Init();

    Message *pcMessage = new Message[10];

    cGN->ReceiveMessage(&pcMessage[0]);
    cGN->ReceiveMessage(&pcMessage[1]);
    cGN->ReceiveMessage(&pcMessage[2]);
    //cGN->ReceiveMessage(&pcMessage[3]);

    WNDCLASSEX wc = { sizeof(WNDCLASSEX),0,WindowProcedure,0,0,GetModuleHandle(NULL),NULL,NULL,0,NULL,szClassName,NULL };
    if (!RegisterClassEx(&wc))
    {
        return 0;
    }

    CreateWindowEx(0, szClassName, 0, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, HWND_DESKTOP, NULL, GetModuleHandle(NULL), cGN);

    MSG messages;
    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    cGN->Stop();
    cPN->Stop();
    cSN->Stop();

    cGN->DeInit();
    cPN->DeInit();
    cSN->DeInit();

    delete[] pcMessage;

    delete cGN;
    delete cSN;
    delete cPN;


	cSR->Stop();
	cSR->DeInit();
	delete cSR;
    
    return 0; 
}