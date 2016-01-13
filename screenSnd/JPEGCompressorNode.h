#pragma once

#include <jpeglib.h>

#include "ProcessingNode.h"

class JPEGCompressorNode : public ProcessingNode {
protected:
    bool m_bSaveToFile;

    int m_iQuality;
    int m_iValidBytes;
    char *m_pcStorage;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
public:
    JPEGCompressorNode(int _iMailboxSize, std::string _sName, int _iQuality, bool bSaveToFile);
    bool Init();
    bool DeInit();
    void ProcessMessage(Message *_pcMessage);
};