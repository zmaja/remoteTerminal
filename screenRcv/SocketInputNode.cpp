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

void SocketInputNode::ProcessMessage(Message* _pcMessage)
{
    int iRecvLen;
    unsigned int uiSrcLen = sizeof(m_sourceAddr);
    int iImgSize = 0;
    double dTimeInMill;
    char* pchTmpBuf;
    bool bPacketsLost = false;
    struct timeval  tv;
    
    while(!bPacketsLost) {
        gettimeofday(&tv, NULL);
        dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
        _pcMessage->SetStartTime(dTimeInMill);
        
        tFrameMessage tImgHeader;

        pchTmpBuf = (char*)_pcMessage->GetPayloadAddress();
        
        do {
            // Wait for the message type field. It will be in the first 16 bytes.
            iRecvLen = recvfrom_timeout(m_iSocket,
                                (char*)&tImgHeader,
                                sizeof(tFrameMessage),
                                0,
                                (struct sockaddr*) &m_sourceAddr,
                                &uiSrcLen,
                                300);
            DEBUG_MSG(m_sName << " ProcessMessage: RecvLen: " << iRecvLen);
            if(iRecvLen == -1) {
                DEBUG_MSG(m_sName << " ProcessMessage: Error reading TYPE field. ");
                continue; // Packets lost, go wait for next frame.
            }
            if(iRecvLen != sizeof(tFrameMessage)) {
                continue;
            }
            
        } while(!((tImgHeader.identificator == MSG_FRAME) && (tImgHeader.padding == 0)));
        iImgSize = tImgHeader.iPayloadSize;
        _pcMessage->SetValidBytes(iImgSize);
        m_iSourceMaxMsgSize = tImgHeader.iMaxMsgSize;
        DEBUG_MSG(m_sName << " ProcessMessage: payload size. " << iImgSize << "maxMsgSize size. " << m_iSourceMaxMsgSize);
        
        
        // Read the image.
        iRecvLen = 0;
        
        while(iImgSize > m_iSourceMaxMsgSize){
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
                iImgSize -= iRecvLen;
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
                            iImgSize,
                            MSG_WAITALL,
                            (struct sockaddr*) &m_sourceAddr,
                            &uiSrcLen,
                            300);
        if (iRecvLen == -1) {
            DEBUG_MSG(m_sName << " ProcessMessage: Error reading image leftovers. ");
            continue; // Packets lost, go wait for next frame.
        }
        if((iImgSize - iRecvLen) != 0){
            continue; // Packets lost, go wait for next frame.
        }                   

        gettimeofday(&tv, NULL);
        dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
        _pcMessage->SetEndTime(dTimeInMill);
        break;
    } // Don't signal the decoding thread, unless the whole image is received
}
