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


SDL_Surface* CEV_textureToSurface(SDL_Texture* src)
{/*convert texture into surface*/

    SDL_Renderer* render = CEV_videoSystemGet()->render;

    uint32_t format = SDL_PIXELFORMAT_RGBA8888;
    int w, h;

    SDL_QueryTexture(src, NULL, NULL, &w, &h);

    SDL_Texture* srcCpy = SDL_CreateTexture(render, format, SDL_TEXTUREACCESS_TARGET, w, h);

    SDL_SetRenderTarget(render, srcCpy);
    SDL_RenderCopy(render, src, NULL, NULL);

    uint32_t* pix = malloc(w*h*32);

    SDL_RenderReadPixels(render, NULL, format, pix, w*4);

    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormatFrom(pix, w, h, 32, w*4, format);

    SDL_SetRenderTarget(render, NULL);
    SDL_DestroyTexture(srcCpy);
    //free(pix); //added 20-02-2022

    return result;
}



int CEV_textureSavePNG(SDL_Texture *src, char* fileName)
{// TODO (drx#1#): sécuriser
    SDL_Surface *surf = CEV_textureToSurface(src);
    int funcSts = IMG_SavePNG(surf, fileName);
    SDL_FreeSurface(surf);

    return funcSts;
}


int CEV_textureSavePNG_RW(SDL_Texture *src, SDL_RWops* dst)
{// TODO (drx#1#): sécuriser
    SDL_Surface *surf = CEV_textureToSurface(src);
    int funcSts = IMG_SavePNG_RW(surf, dst, 0);
    SDL_FreeSurface(surf);

    return funcSts;
}


int CEV_textureToCapsule(SDL_Texture* src, CEV_Capsule* dst)
{// TODO (drx#1#): sécuriser

    SDL_RWops* vFile = SDL_AllocRW();

    CEV_Capsule caps;

    CEV_textureSavePNG_RW(src, vFile);

    size_t size = SDL_RWsize(vFile);

    caps.size = size;
    caps.type = 4;
    caps.data = malloc(size);

    SDL_RWread(vFile, caps.data, size, 1);

    SDL_RWclose(vFile);

    return FUNC_OK;
}





