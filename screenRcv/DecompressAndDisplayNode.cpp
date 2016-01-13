#include <iostream>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "DecompressAndDisplayNode.h"

DecompressAndDisplayNode::DecompressAndDisplayNode(int _iMailboxSize, string _sName) : ProcessingNode(_iMailboxSize, _sName)
{
    // Allocate enough place for a HD image.
    m_puchImageWithoutAlpha = new unsigned char[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*MAX_BPP >> 3];
}

// Function adds alpha channel to a BGR image and changes BGR to RGB
void DecompressAndDisplayNode::addAlphaChannel(unsigned char* _imgArr, int _imgWidth, int _imgHeight, unsigned char* _imgArrWithAlpha) {
    for(int i = 0; i < _imgWidth*_imgHeight; i++) {
        *(_imgArrWithAlpha+2) = *(_imgArr+0);   // first channel
        *(_imgArrWithAlpha+1) = *(_imgArr+1);   // second channel
        *(_imgArrWithAlpha+0) = *(_imgArr+2);   // third channel
        *(_imgArrWithAlpha+3) = 255;            // alpha channel
        _imgArrWithAlpha += 4;
        _imgArr += 3;
    }
}


bool DecompressAndDisplayNode::Init()
{
    if (ProcessingNode::Init() == FALSE)
        return false;
    
    // Open the framebuffer device file for reading and writing.
    m_iFramebuffFD = open("/dev/fb0", O_RDWR);
    if (m_iFramebuffFD == -1) {
        cout << m_sName << " Init: Error, cannot open framebuffer device." << endl;
        return 0;
    }
    
    // Set grounds for decompression.
    m_cInfo.err = jpeg_std_error(&m_jErr);
    jpeg_create_decompress(&m_cInfo);

    return true;
}

bool DecompressAndDisplayNode::DeInit()
{
    bool bRetVal = ProcessingNode::DeInit();

    jpeg_finish_decompress(&m_cInfo);

    delete[] m_puchImageWithoutAlpha;

    return bRetVal;
}

void DecompressAndDisplayNode::ProcessMessage(Message* _pcMessage)
{
    struct fb_var_screeninfo varInfo; // Variable screen info
    unsigned char* puchRowPtr[1];     // pointer to an array
    struct timeval  tv;
    
    char* p_chTmpPayload = (char*)_pcMessage->GetPayloadAddress();
    
    // Send decompression&display start time to receiving thread.
    gettimeofday(&tv, NULL);
    double dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
    _pcMessage->SetStartTime(dTimeInMill);
    
    
    // Get variable screen information.
    if (ioctl(m_iFramebuffFD, FBIOGET_VSCREENINFO, &varInfo)) {
        cout << m_sName << " ProcessMessage: Error reading variable screen info." << endl;
    }
    jpeg_mem_src(&m_cInfo, (unsigned char*)p_chTmpPayload,
                MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*MAX_BPP >> 3);
            
    jpeg_read_header(&m_cInfo, TRUE);
    jpeg_start_decompress(&m_cInfo);
    
    // Get image data.
    int iImgW = m_cInfo.output_width;
    int iImgH = m_cInfo.output_height;
    int iImgSize = _pcMessage->GetValidBytes();
    
    varInfo.xres = iImgW;
    varInfo.yres = iImgH;
    varInfo.bits_per_pixel = MAX_BPP;
    
    // Set framebuffer to the resolution of received image.
    ioctl(m_iFramebuffFD, FBIOPUT_VSCREENINFO, &varInfo );
    // Map framebuffer to user memory.
    char* pchFramebuf = (char*)mmap(0, iImgW*iImgH*MAX_BPP>>3, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, m_iFramebuffFD, 0);
    if (pchFramebuf == MAP_FAILED) {
        cout << m_sName << " ProcessMessage: Failed to mmap framebuffer. " << endl;
    } else {
        while (m_cInfo.output_scanline < iImgH) // loop through image
        {
            // Enable jpeg_read_scanlines() to fill our jdata array
            puchRowPtr[0] = (unsigned char *)m_puchImageWithoutAlpha +
                            3 * iImgW * m_cInfo.output_scanline; 

            jpeg_read_scanlines(&m_cInfo, puchRowPtr, 1);
        }
        jpeg_finish_decompress(&m_cInfo);

        addAlphaChannel(m_puchImageWithoutAlpha, iImgW, iImgH, (unsigned char*)pchFramebuf);
        // Cleanup framebuffer.
        munmap(pchFramebuf, iImgW*iImgH*MAX_BPP>>3);
    }
    
    // Send decompression&display end time to receiving thread.
    gettimeofday(&tv, NULL);
    dTimeInMill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to milliseconds
    _pcMessage->SetEndTime(dTimeInMill);
}
