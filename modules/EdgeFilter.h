#ifndef _EDGE_FILTER_H_
#define _EDGE_FILTER_H_

#include "../modulesapi/VideoFilter.h"

const char edgeFilterName[] = "Edge filter";

class CEdgeFilter : public CVideoFilter
{
    BITMAP *nbmp;
    int vw, vh;
    int method;
    int color;
    int treshold;
    double alpha;

    int	MASK[5][5];
    int GX[3][3];
    int	GY[3][3];

public:

    CEdgeFilter() : nbmp(NULL), vw(0), vh(0)
    {

        MASK[0][0] = -1; MASK[0][1] = -1; MASK[0][2] = -1; MASK[0][3] = -1; MASK[0][4] = -1;
        MASK[1][0] = -1; MASK[1][1] = -1; MASK[1][2] = -1; MASK[1][3] = -1; MASK[1][4] = -1;
        MASK[2][0] = -1; MASK[2][1] = -1; MASK[2][2] = 24; MASK[2][3] = -1; MASK[2][4] = -1;
        MASK[3][0] = -1; MASK[3][1] = -1; MASK[3][2] = -1; MASK[3][3] = -1; MASK[3][4] = -1;
        MASK[4][0] = -1; MASK[4][1] = -1; MASK[4][2] = -1; MASK[4][3] = -1; MASK[4][4] = -1;

        GX[0][0] = -1; GX[0][1] = 0; GX[0][2] = 1;
        GX[1][0] = -2; GX[1][1] = 0; GX[1][2] = 2;
        GX[2][0] = -1; GX[2][1] = 0; GX[2][2] = 1;

        GY[0][0] =  1; GY[0][1] =  2; GY[0][2] =  1;
        GY[1][0] =  0; GY[1][1] =  0; GY[1][2] =  0;
        GY[2][0] = -1; GY[2][1] = -2; GY[2][2] = -1;
    }

    ~CEdgeFilter()
    {
        if(nbmp) delete nbmp;
    }

    void Default()
    {
        method = 0;
        color = 0;
        treshold = 0;
        alpha = 1.0f;
    }

    void GetInfo(std::string &name, FilterConfig_t &info)
    {
        name = edgeFilterName;
        info["method"] = method;
        info["color"] = color;
        info["treshold"] = treshold;
        info["alpha"] = alpha;
    }

    void Config(int bpp, int w, int h, FilterConfig_t &conf)
    {
        Default();
        if(conf.count("method") > 0) method = (int)conf["method"];
        if(method < 0) method = 0;
        if(method > 1) method = 1;
        if(conf.count("color") > 0) color = (int)conf["color"];
        if(color < 0) color = 0;
        if(color > 1) color = 1;
        if(conf.count("alpha") > 0) alpha = conf["alpha"];
        if(alpha < 0.0f) alpha = 0.0f;
        if(alpha > 1.0f) alpha = 1.0f;
        if(conf.count("treshold") > 0) treshold = (int)conf["treshold"];
        if(treshold < 0) treshold = 0;
        if(treshold > 255) treshold = 255;
        if(vw != w || vh != h)
        {
            if(nbmp) destroy_bitmap(nbmp);
            nbmp = create_bitmap_ex(bpp, w, h);
            vw = w;
            vh = h;
        }
    }

    void Apply(BITMAP *bmp);
};

#endif // _EDGE_FILTER_H_
