#ifndef _FADEAWAY_FILTER_H_
#define _FADEAWAY_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char fadeawayFilterName[] = "Fadeaway filter";

class CFadeawayFilter : public CVideoFilter
{
    int size, time, effect;
    BITMAP *tmp, *tmp2;

public:

    CFadeawayFilter() : tmp(NULL), tmp2(NULL) {}
    virtual ~CFadeawayFilter() { if(tmp) destroy_bitmap(tmp); }

    void Default()
    {
        size = 0;
        time = 0;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = fadeawayFilterName;
        info["time"] = time;
        info["effect"] = effect;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("time") != 0)
        {
            int ltime = time;
            time = (int)conf["time"];
            if(ltime != time) size = 0;
        }
        if(time > 1000) time = 1000;
        if(time < 0) time = 0;
        if(conf.count("effect") != 0) effect = (int)conf["effect"];
        if(effect > 1) effect = 1;
        if(effect < 0) effect = 0;
        if(tmp == NULL || w != tmp->w || h != tmp->h)
        {
            if(tmp) destroy_bitmap(tmp);
            if(tmp2) destroy_bitmap(tmp2);
            tmp = create_bitmap_ex(bpp, w, h);
            tmp2 = create_bitmap_ex(bpp, w, h);
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _FADEAWAY_FILTER_H_
