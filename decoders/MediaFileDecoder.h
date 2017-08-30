#ifndef _MEDIAFILEDECODER_H_
#define _MEDIAFILEDECODER_H_

#include "../decodersapi/VideoDecoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/opt.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

#include <string>
#include <allegro.h>
#include <iostream>

class CMediaFileDecoder : public CVideoDecoder
{
    AVFormatContext     *pFmtCtx;
    AVPacket            pkt;
    AVFrame             *pFrame;
    AVStream            *video_stream;
    AVCodec             *pCodec;
    AVCodecContext      *pCodecCtx;

    int video_index;
    int sws_flags;

    SwsContext *sws_opts;
    SwsContext *img_convert_ctx;

    bool fileOpened;
    std::string fileName;

    float pts;
    long toWait;
    long lastFrameTime;

    bool OpenFile(std::string fname);
    void CloseFile();

public:
    CMediaFileDecoder();
    virtual ~CMediaFileDecoder();

    bool Init(char *line);
    void Close();

    bool GetNextFrame(BITMAP *bmp);

    void DumpFormat();

    int Width() { return (fileOpened ? pCodecCtx->width : 0); }
    int Height() { return (fileOpened ? pCodecCtx->height : 0); }
};

#endif // _MEDIAFILEDECODER_H_
