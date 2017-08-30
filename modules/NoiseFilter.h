#ifndef _NOISE_FILTER_H_
#define _NOISE_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char noiseFilterName[] = "Noise filter";

class CNoiseFilter : public CVideoFilter
{
    double noise;

public:

    CNoiseFilter()
    {
    }

    ~CNoiseFilter()
    {
    }

    void Default()
    {
        noise = 0.0f;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = noiseFilterName;
        info["noise"] = noise;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("noise") != 0) noise = conf["noise"];
        if(noise < 0.0f) noise = 0.0f;
        if(noise > 1.0f) noise = 1.0f;
    }

    void Apply(BITMAP *bmp);
};

#endif // _NOISE_FILTER_H_
