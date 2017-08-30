#ifndef _DELAY_FILTER_H_
#define _DELAY_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char delayFilterName[] = "Delay filter";

#define MAX_FRAMES  52
class CDelayFilter : public CVideoFilter
{
    int vw, vh;
    int delay;
    int start_index, size;
    BITMAP *dbmp[MAX_FRAMES];

public:

    CDelayFilter() : vw(0), vh(0), start_index(0), size(0)
    {
    }

    virtual ~CDelayFilter()
    {
        for(int i=0;i<MAX_FRAMES;i++)
            if(dbmp[i]) destroy_bitmap(dbmp[i]);
    }

    void Default()
    {
        delay = 0;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = delayFilterName;
        info["delay"] = delay;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("delay") != 0) delay = (int)conf["delay"];
        if(delay >= MAX_FRAMES-1) delay = MAX_FRAMES-2;
        if(w != vw || h != vh)
        {
            start_index = 0;
            size = 0;
            vw = w;
            vh = h;
            for(int i=0;i<MAX_FRAMES;i++)
                dbmp[i] = create_bitmap_ex(bpp, w, h);
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _DELAY_FILTER_H_
