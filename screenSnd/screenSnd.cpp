// screenSnd.cpp : Defines the entry point for the console application.
//

#include <stdio.h>

#include "GrabberNode.h"

int main() {

    GrabberNode cGN (10, "GN1");
    GrabberNode cGN2(10, "GN2");
    ProcessingNode cPN(10, "PN");

    cGN.SetNextProcessingNode(&cGN2);
    cGN2.SetNextProcessingNode(&cPN);
    cPN.SetNextProcessingNode(&cGN);

    cGN.Init();
    cGN2.Init();
    cPN.Init();
    
    Sleep(2000);

    Message *pcMessage = new Message[1];

    cGN.ReceiveMessage(&pcMessage[0]);
    cGN.ReceiveMessage(&pcMessage[1]);
    cGN.ReceiveMessage(&pcMessage[2]);

    Sleep(10000);

    cGN.Stop();
    cGN2.Stop();
    cPN.Stop();

    cGN.DeInit();
    cGN2.DeInit();
    cPN.DeInit();

    delete [] pcMessage;
    
    return 0; 
}