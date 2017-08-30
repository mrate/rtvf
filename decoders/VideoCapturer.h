#ifndef _VIDEOCAPTURER_H_
#define _VIDEOCAPTURER_H_

#include "../decodersapi/VideoDecoder.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/opt.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

#include <libiec61883/iec61883.h>
#include <allegro.h>

class CVideoCapturer : public CVideoDecoder
{
    AVCodec             *pCodec;
    AVCodecContext      *pCodecCtx;
    AVFrame             *pFrame;

    int sws_flags;
    int width, height;

    SwsContext *sws_opts;
    SwsContext *img_convert_ctx;

    FILE *f;
    raw1394handle_t raw_handle;
    iec61883_dv_fb_t raw_frame;
    int channel;

    static int process_frame(unsigned char *data, int len, int complete, void *callback_data);
    int ProcessFrame(uint8_t *data, int len);

    BITMAP *tbmp;
    bool StartCapture(int node);
    void StopCapture();

public:

    CVideoCapturer();
    virtual ~CVideoCapturer();

    bool GetNextFrame(BITMAP *bmp);

    void DumpFormat();

    bool Init(char *line);
    void Close();

    int Width() { return width; }
    int Height() { return height; }
};

#endif // _VIDEOCAPTURER_H_
