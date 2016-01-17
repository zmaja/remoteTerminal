#include <iostream>
#include <string.h>
#include <unistd.h>
#include "SocketInputNode.h"

SocketInputNode::SocketInputNode(int _iMailboxSize, string _sName, int _iPort) : ProcessingNode(_iMailboxSize, _sName)
{
    m_iPort = _iPort;
    m_iMyMaxMsgSize = 0;
    m_iSourceMaxMsgSize = 0;
    m_iSocket = 0;
}

bool SocketInputNode::Init()
{
    // Create a UDP socket.
    if((m_iSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        DEBUG_MSG(m_sName << " Init: creating socket failed.");
    }
    
    // Zero out the structure.
    memset((char*) &m_myAddr, 0, sizeof(m_myAddr));
        
    m_myAddr.sin_family = AF_INET;
    m_myAddr.sin_port = htons(m_iPort);
    m_myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind the socket to a predefined port.
    if(bind(m_iSocket, (struct sockaddr*)&m_myAddr, sizeof(m_myAddr) ) == -1) {
        DEBUG_MSG(m_sName << " Init: binding socket failed.");
    }
    
    unsigned int uiOptVal, uiOptLen = sizeof(unsigned int);
    
    // Read maximum message (packet) size on this type of socket.
    if(getsockopt(m_iSocket, SOL_SOCKET, SO_RCVBUF, (char*)&uiOptVal, &uiOptLen) != -1) {
        m_iMyMaxMsgSize = uiOptVal;
    }

    // Initialize parent members at the end because ProcessingNode::Init()
    // signals that the initialisation is finished.
    if (ProcessingNode::Init() == false)
        return false;

    return true;
}

bool SocketInputNode::DeInit()
{
    bool bRetVal = ProcessingNode::DeInit();

    int iResult = close(m_iSocket);
    if (iResult == -1)
    {
        DEBUG_MSG(m_sName << " Deinit: closesocket failed. ");
        bRetVal = false;
    }

    return bRetVal;
}

#define FRAME 2
typedef int tMsgTypeField;
typedef int tMaxPacketSizeField;
typedef int tImgSizeField;

void SocketInputNode::ProcessMessage(Message* _pcMessage)
{
    int iRecvLen;
    unsigned int uiSrcLen = sizeof(m_sourceAddr);
    tImgSizeField imgSize = 0;
    double dTimeInMill;
    char* pchTmpBuf;
    bool bPacketsLost = false;
    struct timeval  tv;
    
    while(!bPacketsLost) {
        gettimeofday(&tv, NULL);
        dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
        _pcMessage->SetStartTime(dTimeInMill);
        
        int iType1;
        int iType2;
        int iType3;
        int iType4;
        pchTmpBuf = (char*)_pcMessage->GetPayloadAddress();
        
        do {
            // Wait for the message type field. It will be in the first 16 bytes.
            iRecvLen = recvfrom_timeout(m_iSocket,
                                pchTmpBuf,
                                4*sizeof(tMsgTypeField),
                                0,
                                (struct sockaddr*) &m_sourceAddr,
                                &uiSrcLen,
                                300);
            DEBUG_MSG(m_sName << " ProcessMessage: RecvLen: " << iRecvLen);
            if(iRecvLen == -1) {
                DEBUG_MSG(m_sName << " ProcessMessage: Error reading TYPE field. ");
                continue; // Packets lost, go wait for next frame.
            }
            if(iRecvLen != 4*sizeof(tMsgTypeField)) {
                continue;
            }

            // Read the type from the first 16 bytes.
            iType1 = *(((int*)pchTmpBuf)+0);
            iType2 = *(((int*)pchTmpBuf)+1);
            iType3 = *(((int*)pchTmpBuf)+2);
            iType4 = *(((int*)pchTmpBuf)+3);

        } while(!(iType1 == FRAME && iType2 == 0 && iType3 == 0 && iType4 == 0));
        
        
        // If the type is FRAME:
        //     - get the max packet size on the other machine
        //     - get frame (image) size, and
        //     - continue collecting packets until the image is received.
        iRecvLen = recvfrom_timeout(m_iSocket,
                            pchTmpBuf,
                            sizeof(tMaxPacketSizeField),
                            0,
                            (struct sockaddr*) &m_sourceAddr,
                            &uiSrcLen,
                            300);
        if(iRecvLen == -1) {
            DEBUG_MSG(m_sName << " ProcessMessage: Error reading max packet size on the source machine. ");
            continue; // Packets lost, go wait for next frame.
        }
        if(iRecvLen != sizeof(tMaxPacketSizeField)){
            continue; // Packets lost, go wait for next frame.
        }
        m_iSourceMaxMsgSize = *((int*)pchTmpBuf);
        iRecvLen = recvfrom_timeout(m_iSocket,
                            pchTmpBuf,
                            sizeof(tImgSizeField),
                            0,
                            (struct sockaddr*) &m_sourceAddr,
                            &uiSrcLen,
                            300);
        if(iRecvLen == -1) {
            DEBUG_MSG(m_sName << " ProcessMessage: Error reading image size. ");
            continue; // Packets lost, go wait for next frame.
        }
        if(iRecvLen != sizeof(tImgSizeField)){
            continue; // Packets lost, go wait for next frame.
        }
        imgSize = *((int*)pchTmpBuf);
        _pcMessage->SetValidBytes(imgSize);
        DEBUG_MSG(m_sName << " ProcessMessage: payload size. " << imgSize);
        // Read the image.
        iRecvLen = 0;
        
        while(imgSize > m_iSourceMaxMsgSize){
            // While there is enough data, receive
            // the maximum packet size the other machine can send.
            iRecvLen = recvfrom_timeout(m_iSocket,
                                pchTmpBuf,
                                m_iSourceMaxMsgSize,
                                0,
                                (struct sockaddr*) &m_sourceAddr,
                                &uiSrcLen,
                                300);
            if(iRecvLen == -1) {
                DEBUG_MSG(m_sName << " ProcessMessage: Error reading image (max packet size). ");
            } else {
                if(iRecvLen < m_iSourceMaxMsgSize)
                {
                    bPacketsLost = true;
                    break;
                }
                DEBUG_MSG(m_sName << " ProcessMessage: Primljeno: " << iRecvLen);
                pchTmpBuf += iRecvLen;
                imgSize -= iRecvLen;
            }
        }
        if(bPacketsLost) {
            bPacketsLost = false;
            continue;
        }
        // When there is no enough data to fill the max packet size,
        // receive what is left.
        iRecvLen = recvfrom_timeout(m_iSocket,
                            pchTmpBuf,
                            imgSize,
                            MSG_WAITALL,
                            (struct sockaddr*) &m_sourceAddr,
                            &uiSrcLen,
                            300);
        if (iRecvLen == -1) {
            DEBUG_MSG(m_sName << " ProcessMessage: Error reading image leftovers. ");
            continue; // Packets lost, go wait for next frame.
        }
        if((imgSize - iRecvLen) != 0){
            continue; // Packets lost, go wait for next frame.
        }                   

        gettimeofday(&tv, NULL);
        dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
        _pcMessage->SetEndTime(dTimeInMill);
        break;
    } // Don't signal the decoding thread, unless the whole image is received
}
