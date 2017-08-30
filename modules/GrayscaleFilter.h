#ifndef _GRAYSCALE_FILTER_H_
#define _GRAYSCALE_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char grayFilterName[] = "Grayscale filter";

class CGrayFilter : public CVideoFilter
{
    double perc;

public:

    void Default()
    {
        perc = 0.0f;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = grayFilterName;
        info["percent"] = perc;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("percent") != 0) perc = conf["percent"];
        if(perc > 1) perc = 1;
        if(perc < 0) perc = 0;
    }

    void Apply(BITMAP *bmp);
};

#endif // _GRAYSCALE_FILTER_H_
