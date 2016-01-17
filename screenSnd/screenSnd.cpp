// screenSnd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>

#include "BasicGrabberNode.h"
#include "GDIGrabberNode.h"
#include "DXGIGrabberNode.h"
#include "JPEGCompressorNode.h"
#include "SocketOutputNode.h"
#include "SocketReceiverNode.h"

char szClassName[] = "WindowsApp";

//#define __GDI_GRABBER
//#define __SEND_TO_LOCAL_HOST

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BasicGrabberNode *cGN = (BasicGrabberNode*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

#ifdef __GDI_GRABBER
    GDIGrabberNode *cGN = new GDIGrabberNode(10, "GN");
#else //__GDI_GRABBER
    DXGIGrabberNode *cGN = new DXGIGrabberNode(10, "GN");
#endif //__GDI_GRABBER
    JPEGCompressorNode *cPN = new JPEGCompressorNode(10, "PN", 30, true);
#ifdef __SEND_TO_LOCAL_HOST
    SocketOutputNode *cSN = new SocketOutputNode(10, "SN", "127.0.0.1", 8888);
#else //__SEND_TO_LOCAL_HOST
    SocketOutputNode *cSN = new SocketOutputNode(10, "SN", "192.168.1.133", 8888);
#endif //__SEND_TO_LOCAL_HOST

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