#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "ProcessingNode.h"
#include "SocketInputNode.h"
#include "DecompressAndDisplayNode.h"

using namespace std;

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
    ungetc(ch, stdin);
    return 1;
    }

    return 0;
}

int main() {
    
    SocketInputNode *cSIN = new SocketInputNode(10, "SIN", 8888);
    DecompressAndDisplayNode *cDNDN = new DecompressAndDisplayNode(10, "DNDN");


    cSIN->SetNextProcessingNode(cDNDN);
    cDNDN->SetNextProcessingNode(cSIN);

    cSIN->Init();
    cDNDN->Init();

    Message *pcMessage = new Message[4];

    cSIN->ReceiveMessage(&pcMessage[0]);
    cSIN->ReceiveMessage(&pcMessage[1]);
    
    //sleep(20);
    while(!kbhit()){
        sleep(1);
    }
    cout << " Main: nothing happend." << endl;
    
    cSIN->Stop();
    cDNDN->Stop();

    cSIN->DeInit();
    cDNDN->DeInit();

    delete[] pcMessage;

    delete cSIN;
    delete cDNDN;
    
    cout << "Main: program completed. Exiting.\n" << endl;
    return 0;
}
