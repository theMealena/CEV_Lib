#ifndef PARALLAX_H_INCLUDED
#define PARALLAX_H_INCLUDED

#include <SDL.h>
#include "rwtypes.h"
#include "stdbool.h"
#include "CEV_gif.h"


typedef struct S_ParaLayer
{
    SDL_Texture *texture; //displayed texture
    SDL_Rect picSize; //picture size
    float x, y;
    float ratio, velX, velY;
    bool isGif; //is animated as gif file
    CEV_GifAnim *anim;
}
ParaLayer;



typedef struct S_Parallax
{
    int numOfLayer;
    SDL_Rect *scrollPos;
    ParaLayer *layers;
}
CEV_Parallax;


CEV_Parallax* CEV_parallaxCreate(int numOfLayer, SDL_Rect *scroll);

void CEV_parallaxDestroy(CEV_Parallax *in);

void CEV_parallaxUpdate(CEV_Parallax *in);

int CEV_convertParallaxCSVToData(const char* srcName, const char* dstName);

CEV_Parallax* CEV_parallaxLoad(const char* fileName);

CEV_Parallax* CEV_parallaxLoad_RW(SDL_RWops* ops, uint8_t freeSrc);



#endif // PARALLAX_H_INCLUDED
