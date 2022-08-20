//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  20-02-2022   |   1.0    | added missing free


#include <stdlib.h>
#include <SDL_Mixer.h>
#include <SDL_ttf.h>
#include <SDL.h>
#include "CEV_types.h"
#include "CEV_mixSystem.h"
#include "CEV_dataFile.h"


void CEV_fontClose(CEV_Font* font)
{/*closes overlay font*/

    if(IS_NULL(font))
        return;

    TTF_CloseFont(font->font);
    free(font->virtualFile);

    free(font);
}


void CEV_waveClose(CEV_Chunk* chunk)
{/*closes overlay sound*/

    if(IS_NULL(chunk))
        return;

    if(chunk->sound != NULL)
        Mix_FreeChunk(chunk->sound);

    if(chunk->virtualFile != NULL)
        free(chunk->virtualFile);

    free(chunk);
}


void CEV_musicClose(CEV_Music* music)
{/*closes overlay music*/

    if(IS_NULL(music))
        return;

    if(music->music != NULL)
        Mix_FreeMusic(music->music);

    if(music->virtualFile != NULL)
        free(music->virtualFile);

    if(music != NULL)
        free(music);

    CEV_soundSystemGet()->loadedMusic = NULL;
}


SDL_Surface* CEV_textureToSurface(SDL_Texture* src, void** ptr)
{//convert texture into surface

    if IS_NULL(src)
    {
        fprintf(stderr, "Err at %s / %d : src arg is NULL.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    SDL_Surface* result = NULL;
    uint32_t* pix = NULL;

    SDL_Renderer* render = CEV_videoSystemGet()->render;

    uint32_t format = SDL_PIXELFORMAT_RGBA8888;
    int w, h;

    //quering dimension
    SDL_QueryTexture(src, NULL, NULL, &w, &h);

    //texture as copy of src
    SDL_Texture* srcCpy = SDL_CreateTexture(render, format, SDL_TEXTUREACCESS_TARGET, w, h);

    if IS_NULL(srcCpy)
    {
        fprintf(stderr, "Err at %s / %d : unable to create texture : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto end;
    }

    SDL_SetRenderTarget(render, srcCpy);    //copy as render
    SDL_RenderCopy(render, src, NULL, NULL);//blitting src on srcCpy

    //allocating pixel field
    pix = malloc(w * h * 32);

    if IS_NULL(pix)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate pixel field : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    //fetching pixel field
    SDL_RenderReadPixels(render, NULL, format, pix, w*4);

    result = SDL_CreateRGBSurfaceWithFormatFrom(pix, w, h, 32, w*4, format);

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create surface : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_2;
    }

    SDL_SetRenderTarget(render, NULL);
    SDL_DestroyTexture(srcCpy);
    *ptr = pix;

end:
    return result;

err_2:
    free(pix);

err_1:
    SDL_SetRenderTarget(render, NULL);
    SDL_DestroyTexture(srcCpy);

    return result;
}


int CEV_textureSavePNG(SDL_Texture *src, char* fileName)
{//save texture as png file

    int funcSts = FUNC_OK;
    void* ptr   = NULL; //holds pxl field

    SDL_Surface *surf = CEV_textureToSurface(src, &ptr);

    if IS_NULL(surf)
    {
        fprintf(stderr, "Err at %s / %d : unable to create surface :%s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    if(IMG_SavePNG(surf, fileName))
    {
        fprintf(stderr, "Err at %s / %d : save failed : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        funcSts = FUNC_ERR;
    }

    SDL_FreeSurface(surf);
    free(ptr);

    return funcSts;
}


int CEV_textureSavePNG_RW(SDL_Texture *src, SDL_RWops* dst)
{//saves texture as png virtual file

    int funcSts = FUNC_OK;
    void* ptr = NULL;

    SDL_Surface *surf = CEV_textureToSurface(src, &ptr);

    if IS_NULL(surf)
    {
        fprintf(stderr, "Err at %s / %d : unable to create surface :%s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    if(IMG_SavePNG_RW(surf, dst, 0))
    {
        fprintf(stderr, "Err at %s / %d : save failed : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        funcSts = FUNC_ERR;
    }

    SDL_FreeSurface(surf);
    free(ptr);

    return funcSts;
}


int CEV_textureToCapsule(SDL_Texture* src, CEV_Capsule* dst)
{//texture into png file's CEV_Capsule

    int funcSts = FUNC_OK;
    SDL_RWops* vFile = SDL_AllocRW();

    if IS_NULL(vFile)
    {
        fprintf(stderr, "Err at %s / %d :   .\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }


    if(CEV_textureSavePNG_RW(src, vFile))
    {
        fprintf(stderr, "Err at %s / %d :   .\n", __FUNCTION__, __LINE__ );
        funcSts = FUNC_ERR;
        goto end;
    }

    size_t size = SDL_RWsize(vFile);

    CEV_capsuleClear(dst);

    dst->size = size;
    dst->type = IS_PNG;
    dst->data = malloc(size);

    if IS_NULL(dst->data)
    {
        fprintf(stderr, "Err at %s / %d : unable to alloc data field : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    SDL_RWread(vFile, dst->data, size, 1);

end:
    SDL_RWclose(vFile);

    return funcSts;
}


SDL_Rect CEV_textureDimGet(SDL_Texture* src)
{//texture size to SDL_Rect
    SDL_Rect result = CLEAR_RECT;

    SDL_QueryTexture(src, NULL, NULL, &result.w, &result.h);

    return result;
}


int CEV_blitSurfaceToTexture(SDL_Surface *src, SDL_Texture* dst, SDL_Rect* srcRect, SDL_Rect* dstRect)
{//blits surface onto texture

    if (IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : One arg at least is NULL.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

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

    CEV_constraint(0, &clip.w, srcDim.w);
    CEV_constraint(0, &clip.h, srcDim.h);
    CEV_constraint(0, &blit.w, dstDim.w);
    CEV_constraint(0, &blit.h, dstDim.h);


    if (access != SDL_TEXTUREACCESS_STREAMING)
    {
        fprintf(stderr, "Err at %s / %d : Texture provided is not SDL_TEXTUREACCESS_STREAMING.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    /*correcting source rect if out of dimensions*/
    if(clip.x<0)
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

    /*correcting dst rect if out of dimensions*/
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
        blit.h = dstDim.h - blit.y;


    int minW = MIN(clip.w, blit.w),
        minH = MIN(clip.h, blit.h),
        dstPitch;

    uint32_t *srcPixel = src->pixels,
             *dstPixel = NULL;

    SDL_LockTexture(dst, &blit, (void**)&dstPixel, &dstPitch);
    dstPitch /= 4;

    for(int ix = 0; ix < minW; ix++)
        for(int iy = 0; iy < minH; iy++)
        {
            uint32_t color = *(srcPixel + (clip.y + iy)*(src->pitch/4) + (clip.x + ix));

            if(color & 0xFF)
                *(dstPixel + iy*dstPitch + ix)=color ;
        }


    SDL_UnlockTexture(dst);

    return FUNC_OK;
}


