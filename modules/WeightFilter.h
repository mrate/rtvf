#ifndef _WEIGHT_FILTER_H_
#define _WEIGHT_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char weightFilterName[] = "Weight filter";

class CWeightFilter : public CVideoFilter
{
    int vw, vh, speed;
    uint8_t *rweight;
    uint8_t *gweight;
    uint8_t *bweight;

    long p;
    uint8_t *rw;
    uint8_t *gw;
    uint8_t *bw;
    int32_t *i;
    int sp;

public:

    CWeightFilter() : vw(0), vh(0), rweight(NULL), gweight(NULL), bweight(NULL)
    {
    }

    virtual ~CWeightFilter()
    {
        if(rweight) delete[] rweight;
        if(gweight) delete[] gweight;
        if(bweight) delete[] bweight;
    }

    void Default()
    {
        speed = 1;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = weightFilterName;
        info["speed"] = speed;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("speed") != 0) speed = (int)conf["speed"];
        if(speed > 10) speed = 10;
        if(speed < 0) speed = 0;
        if(w != vw || h != vh)
        {
            vw = w;
            vh = h;
            if(rweight) delete[] rweight;
            if(gweight) delete[] gweight;
            if(bweight) delete[] bweight;
            rweight = new uint8_t[w*h];
            gweight = new uint8_t[w*h];
            bweight = new uint8_t[w*h];
            memset(rweight, 0, w*h);
            memset(gweight, 0, w*h);
            memset(bweight, 0, w*h);
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _WEIGHT_FILTER_H_
