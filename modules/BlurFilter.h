#ifndef _BLUR_FILTER_H_
#define _BLUR_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char blurFilterName[] = "Blur filter";

#define BLUR_FRAMES     20
class CBlurFilter : public CVideoFilter
{
    int vw, vh;
    int start_index, size, blur;
    BITMAP *dbmp[BLUR_FRAMES];

public:

    CBlurFilter() : vw(0), vh(0), start_index(0), size(0), blur(0)
    {
    }

    virtual ~CBlurFilter()
    {
        for(int i=0;i<BLUR_FRAMES;i++)
            if(dbmp[i]) destroy_bitmap(dbmp[i]);
    }

    void Default()
    {
        blur = 0;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = blurFilterName;
        info["blur"] = blur;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("blur") != 0) blur = (int)conf["blur"];
        if(blur >= BLUR_FRAMES) blur = BLUR_FRAMES-1;
        if(blur < 0) blur = 0;
        if(w != vw || h != vh)
        {
            start_index = 0;
            size = 0;
            vw = w;
            vh = h;
            for(int i=0;i<BLUR_FRAMES;i++)
                dbmp[i] = create_bitmap_ex(bpp, w, h);
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _BLUR_FILTER_H_
