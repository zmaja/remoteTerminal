// screenSnd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>

#include "GrabberNode.h"
#include "JPEGCompressorNode.h"
#include "SocketOutputNode.h"

int main() {

    GrabberNode cGN (10, "GN");
    JPEGCompressorNode cPN(10, "PN", 30, true);
    SocketOutputNode cSN(10, "SN", "192.168.178.33", 10000);

    cGN.SetNextProcessingNode(&cPN);
    cPN.SetNextProcessingNode(&cSN);
    cSN.SetNextProcessingNode(&cGN);

    cGN.Init();
    cPN.Init();
    cSN.Init();
    
    Message *pcMessage = new Message[10];

    cGN.ReceiveMessage(&pcMessage[0]);
    cGN.ReceiveMessage(&pcMessage[1]);
    cGN.ReceiveMessage(&pcMessage[2]);
    //cGN.ReceiveMessage(&pcMessage[3]);

    while (!_kbhit()) {
        Sleep(300);
    }

    cGN.Stop();
    cPN.Stop();
    cSN.Stop();

    cGN.DeInit();
    cPN.DeInit();
    cSN.DeInit();

    delete[] pcMessage;
    
    return 0; 
}