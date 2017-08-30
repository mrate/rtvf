#ifndef _SQUARE_FILTER_H_
#define _SQUARE_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char squareFilterName[] = "Square filter";

class CSquareFilter : public CVideoFilter
{
    int size;
    BITMAP *tmp;

public:

    CSquareFilter() : tmp(NULL) {}
    virtual ~CSquareFilter() { if(tmp) destroy_bitmap(tmp); }

    void Default()
    {
        size = 1;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = squareFilterName;
        info["size"] = size;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("size") != 0) size = (int)conf["size"];
        if(size > 50) size = 50;
        if(size < 1) size = 1;
        if(tmp == NULL || w != tmp->w || h != tmp->h)
        {
            if(tmp) destroy_bitmap(tmp);
            tmp = create_bitmap_ex(bpp, w, h);
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _SQUARE_FILTER_H_
