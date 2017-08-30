#ifndef _THRESHOLD_FILTER_H_
#define _THRESHOLD_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char treshFilterName[] = "Treshold filter";

class CTreshFilter : public CVideoFilter
{
    int tresh, color;

public:

    CTreshFilter()
    {
    }

    virtual ~CTreshFilter()
    {
    }

    void Default()
    {
        tresh = 0;
        color = 0;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = treshFilterName;
        info["treshold"] = tresh;
        info["color"] = color;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("treshold") != 0) tresh = (int)conf["treshold"];
        if(tresh > 255) tresh = 255;
        if(tresh < 0) tresh = 0;
        if(conf.count("color") != 0) color = (int)conf["color"];
        if(color > 1) color = 1;
        if(color < 0) color = 0;
    }

    void Apply(BITMAP *bmp);
};

#endif // _THRESHOLD_FILTER_H_
