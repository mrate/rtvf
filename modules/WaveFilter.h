#ifndef _WAVE_FILTER_H_
#define _WAVE_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char waveFilterName[] = "Wave filter";

class CWaveFilter : public CVideoFilter
{
    double speed, offset;
    int width, height, nwidth;
    int vw;

    BITMAP *tmp;

    time_t ltime;
public:

    CWaveFilter() : vw(0), tmp(NULL)
    {
    }

    ~CWaveFilter()
    {
        if(tmp) destroy_bitmap(tmp);
    }

    void Default()
    {
        speed = 0.0;
        height = 25;
        width = 5;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = waveFilterName;
        info["speed"] = speed;
        info["height"] = height;
        info["width"] = width;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("speed") != 0) speed = conf["speed"];
        if(conf.count("height") != 0) height = (int)conf["height"];
        if(conf.count("width") != 0) width = (int)conf["width"];
        if(width < 0) width = 0;
        if(height < 0) height = 0;
        if(speed < 0) speed = 0;
        if(speed > 50) speed = 50;
        if(vw != w)
        {
            if(tmp) destroy_bitmap(tmp);
            tmp = create_bitmap_ex(bpp, w, 2);
            vw = w;
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _WAVE_FILTER_H_
