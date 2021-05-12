#include <stdio.h>
#include <SDL.h>
#include "CEV_mixSystem.h"
#include "CEV_api.h"
#include "CEV_blit.h"

static void L_blitRectCorrect(SDL_Rect* srcDim, SDL_Rect* srcClip, SDL_Rect* dstDim, SDL_Rect* dstBlit);


void CEV_blitTexture(SDL_Texture* src, int mode)
{
    SDL_Rect clip = CLEAR_RECT,
             blit = CLEAR_RECT;


    SDL_Renderer *render = CEV_videoSystemGet()->render;

    SDL_RenderGetLogicalSize(render, &blit.w, &blit.h);
    SDL_QueryTexture(src, NULL, NULL, &clip.w, &clip.h);









}


void CEV_blitSurfaceToTexture(SDL_Surface *src, SDL_Rect* srcRect, SDL_Texture* dst, SDL_Rect* dstRect)
{
        /*---DECLARATIONS---*/

    int access;

    SDL_Rect    clip,
                blit,
                srcDim    = {0, 0, src->w, src->h},
                dstDim    = CLEAR_RECT;

    SDL_QueryTexture(dst, NULL, &access, &dstDim.w, &dstDim.h);

    if(IS_NULL(srcRect))
        clip = srcDim;
    else
        clip = *srcRect;

    if(IS_NULL(dstRect))
        blit = dstDim;
    else
        blit = *dstRect;

    /*CEV_constraint(0, &clip.w, srcDim.w);
    CEV_constraint(0, &clip.h, srcDim.h);
    CEV_constraint(0, &blit.w, dstDim.w);
    CEV_constraint(0, &blit.h, dstDim.h);*/


    if (access != SDL_TEXTUREACCESS_STREAMING)
    {
        fprintf(stderr, "Err at %s / %d : Texture provided is not SDL_TEXTUREACCESS_STREAMING.\n", __FUNCTION__, __LINE__ );
        return;
    }

   L_blitRectCorrect(&srcDim, &clip, &dstDim, &blit);

    //correcting source rect if out of dimensions
/*    if(clip.x<0)
    {
        clip.w += clip.x;
        clip.x = 0;
    }

    if(clip.x + clip.w > srcDim.w)
        clip.w = srcDim.w - clip.x;

    if(clip.y<0)
    {
        clip.h += clip.y;
        clip.y = 0;
    }

    if(clip.y + clip.h > srcDim.h)
        clip.h = srcDim.h - clip.y;

    //correcting dst rect if out of dimensions
    if(blit.x<0)
    {
        clip.w += blit.x;
        clip.x -= blit.x;
        blit.x = 0;
    }

    if(blit.x + blit.w > dstDim.w)
    {
        blit.w = dstDim.w - blit.x;
    }

    if(blit.y<0)
    {
        clip.h += blit.y;
        clip.y -= blit.y;
        blit.y = 0;
    }

    if(blit.y + blit.h > dstDim.h)
        blit.h = dstDim.h - blit.y;*/


    int minW = MIN(clip.w, blit.w),
        minH = MIN(clip.h, blit.h),
        dstPitch;

    uint32_t *srcPixel = src->pixels,
             *dstPixel = NULL;

    if(SDL_LockTexture(dst, &blit, (void**)&dstPixel, &dstPitch))
        printf("119 lock err");

    dstPitch /= 4;

    //printf("clip is %d, %d, %d, %d\n", clip.x, clip.y, clip.w, clip.h);
    //printf("blit is %d, %d, %d, %d\n", blit.x, blit.y, blit.w, blit.h);

    for(int ix = 0; ix < minW; ix++)
        for(int iy = 0; iy < minH; iy++)
        {
            uint32_t color = *((uint32_t*)srcPixel + (clip.y + iy)*(src->pitch/4) + (clip.x + ix));

            if(color &0xFF000000)//not alpha pixel
                *(dstPixel + iy*dstPitch + ix)= color;
        }


    SDL_UnlockTexture(dst);
}


void CEV_textureClear(SDL_Texture* dst, uint32_t color)
{
    uint32_t *pixels;

    int pitch,
        access;

    SDL_Rect dim = CLEAR_RECT;

    SDL_QueryTexture(dst, NULL, &access, &dim.w, &dim.h);

    if(access != SDL_TEXTUREACCESS_STREAMING)
    {
        fprintf(stderr, "Err at %s / %d : Texture provided is not SDL_TEXTUREACCESS_STREAMING  .\n", __FUNCTION__, __LINE__ );
        return;
    }

    SDL_LockTexture(dst, NULL, (void**)&pixels, &pitch);
    pitch /= 4;

    for(int iy = 0; iy<dim.h; iy++)
    {
        uint32_t* pxlPtr = (uint32_t*)pixels + iy*pitch;

        for(int ix = 0; ix<dim.w; ix++)
            *(pxlPtr + ix) = color;
    }

    SDL_UnlockTexture(dst);
}



static void L_blitRectCorrect(SDL_Rect* srcDim, SDL_Rect* srcClip, SDL_Rect* dstDim, SDL_Rect* dstBlit)
{

    CEV_constraint(0, &srcClip->w, srcDim->w);
    CEV_constraint(0, &srcClip->h, srcDim->h);
    CEV_constraint(0, &dstBlit->w, dstDim->w);
    CEV_constraint(0, &dstBlit->h, dstDim->h);

    /*correcting source rect if out of dimensions*/
    if(srcClip->x<0)
    {
        srcClip->w += srcClip->x;
        srcClip->x = 0;
    }

    if(srcClip->x + srcClip->w > srcDim->w)
        srcClip->w = srcDim->w - srcClip->x;

    if(srcClip->y<0)
    {
        srcClip->h += srcClip->y;
        srcClip->y = 0;
    }

    if(srcClip->y + srcClip->h > srcDim->h)
        srcClip->h = srcDim->h - srcClip->y;

    /*correcting dst rect if out of dimensions*/
    if(dstBlit->x<0)
    {
        srcClip->w += dstBlit->x;
        srcClip->x -= dstBlit->x;
        dstBlit->x = 0;
    }

    if(dstBlit->x + dstBlit->w > dstDim->w)
    {
        dstBlit->w = dstDim->w - dstBlit->x;
    }

    if(dstBlit->y<0)
    {
        srcClip->h += dstBlit->y;
        srcClip->y -= dstBlit->y;
        dstBlit->y = 0;
    }

    if(dstBlit->y + dstBlit->h > dstDim->h)
        dstBlit->h = dstDim->h - dstBlit->y;
}
