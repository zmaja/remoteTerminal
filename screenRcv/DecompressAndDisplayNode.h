#pragma once

#include <jpeglib.h>

#include "ProcessingNode.h"

using namespace std;

class DecompressAndDisplayNode : public ProcessingNode {
protected:
    unsigned char* m_puchImageWithoutAlpha;
	struct jpeg_decompress_struct m_cInfo;
    struct jpeg_error_mgr m_jErr;
    int m_iFramebuffFD;
    
    void addAlphaChannel(unsigned char* _imgArr, int _imgWidth, int _imgHeight, unsigned char* _imgArrWithAlpha);

public:
    DecompressAndDisplayNode(int _iMailboxSize, string _sName);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message* _pcMessage);
};
