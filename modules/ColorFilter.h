#ifndef _FILTERS_H_
#define _FILTERS_H_

#include "../modulesapi/VideoFilter.h"

const char colorFilterName[] = "Color filter";

class CColorFilter : public CVideoFilter
{
    double cr, cg, cb;
    long p;

public:

    void Default()
    {
        cr = 1.0;
        cg = 1.0;
        cb = 1.0;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = colorFilterName;
        info["red"] = cr;
        info["green"] = cg;
        info["blue"] = cb;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("red") != 0) cr = conf["red"];
        if(conf.count("green") != 0) cg = conf["green"];
        if(conf.count("blue") != 0) cb = conf["blue"];
    }

    void Apply(BITMAP *bmp);
};

#endif // _FILTERS_H_
