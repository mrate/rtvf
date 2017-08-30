#ifndef _VIDEOENCODER_H_
#define _VIDEOENCODER_H_

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <allegro.h>

class CVideoEncoder
{
public:
    /// Konstruktor
    CVideoEncoder();
    /// Destruktor
    virtual ~CVideoEncoder();

    /** Otevreni video vystupu
    *   @param name nazev souboru
    *   @param width_in sirka vstupniho obrazu
    *   @param height_in vyska vstupniho obrazu
    *   @param width_out sirka vystupniho obrazu
    *   @param height_out vyska vystupniho obrazu
    *   @return false pri chybe
    */
    bool Open(const char *name, int width_in, int height_in, int width_out, int height_out);

    /// Ukonceni vystupu
    void Close();

    /// Dump vystupniho formatu na stdout
    void DumpFormat();

    /** Ulozeni bitmapy do vystupu
    *   @param bmp bitmapa
    *   @return false pri chybe
    */
    bool EncodeFrame(BITMAP *bmp);

private:
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *video_st;
    AVFrame *picture, *tmp_picture;
    uint8_t *video_outbuf;
    int video_outbuf_size;
    SwsContext *img_convert_ctx;
    int sws_flags;
    char fname[256];
    int in_w, in_h, out_w, out_h;

    bool OpenVideo();
    void CloseVideo();
    AVFrame* AllocPicture(PixelFormat pix_fmt, int width, int height);
    AVStream* AddVideoStream(CodecID codec_id);

};

#endif // _VIDEOENCODER_H_
