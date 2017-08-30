#ifndef _PIPEDECODER_H_
#define _PIPEDECODER_H_

#include <iostream>
#include <allegro.h>
#include "../decodersapi/VideoDecoder.h"

class CPipeDecoder : public CVideoDecoder
{
    int fd;
    int width, height;

    bool OpenPipe(const char *fname);
    void ClosePipe();

public:

    CPipeDecoder();
    virtual ~CPipeDecoder();

    bool GetNextFrame(BITMAP *bmp);

    void DumpFormat() {};

    bool Init(char *line);
    void Close();

    int Width() { return width; }
    int Height() { return height; }

};

#endif // _PIPEDECODER_H_
