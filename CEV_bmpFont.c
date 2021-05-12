#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <SDL.h>
#include "CEV_api.h"
#include "CEV_bmpFont.h"
#include "CEV_blit.h"



CEV_BmpFont* CEV_bmpFontCreate(SDL_Surface* fonts, int which, CEV_BmpFontSize size)
{//allocation bmp font structure

        /*---PRL---*/
    if(which >= CEV_BMPFONT_NUM)
    {
        fprintf(stderr,"Err at %s / %d : 'which' argument is out of range.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

        /*---EXECUTION---*/

    //allocating structure
    CEV_BmpFont* result = calloc(1, sizeof(CEV_BmpFont));

    if(!result)
    {
        fprintf(stderr,"Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    //preparing font surface
    int surfaceH = size,
        surfaceW = size * CEV_BMPFONT_CHAR_NUM;

    Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    result->font = SDL_CreateRGBSurface(0, surfaceW, surfaceH, 32, rmask, gmask, bmask, amask);

    if(!result->font)
    {
        fprintf(stderr,"Err at %s / %d Surface creation: %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err;
    }

    //calculating clip pos from source
    SDL_Rect clip = {.x = 0, .y = 0, .w = surfaceW, .h = surfaceH};

    switch(size)
    {
        case CEV_BMPFONT_SMALL :
            clip.y = CEV_BMPFONT_NUM *(CEV_BMPFONT_LARGE + CEV_BMPFONT_MED) + CEV_BMPFONT_SMALL * which;
        break;

        case CEV_BMPFONT_MED :
            clip.y = CEV_BMPFONT_NUM * CEV_BMPFONT_LARGE + CEV_BMPFONT_MED * which;
        break;

        case CEV_BMPFONT_LARGE :
            clip.y = CEV_BMPFONT_LARGE * which;
        break;
    }

    //copying font to destination surface
    SDL_BlitSurface(fonts, &clip, result->font, NULL);

    //updating informations
    result->dimChar = (SDL_Rect){0, 0, size, size};
    result->dimFont = (SDL_Rect){0, 0, size*CEV_BMPFONT_CHAR_NUM, size};


    return result;

err:

    free(result);

    return NULL;
}


void CEV_bmpFontFree(CEV_BmpFont* font)
{//destroying bmp font structure;

    SDL_FreeSurface(font->font);
    free(font);
}


int CEV_bmpFontDispValue(CEV_BmpFont* font, SDL_Texture* dst, int value, SDL_Rect pos)
{//displays value

    /*DECLARATIONS*****/

    char text[15]   = {0};   /*le texte*/
    int funcSts     = FUNC_OK;

    /*on formate le texte à afficher*/
    sprintf(text, "%d", value);

    /*on crée la surface texte*/
    funcSts = CEV_bmpFontDispText(font, dst, text, pos);

    return(funcSts);
}


int CEV_bmpFontDispText(CEV_BmpFont* font, SDL_Texture* dst, const char* text, SDL_Rect pos)
{//displays text

    if(IS_NULL(font))
    {
        fprintf(stderr, "Err at %s / %d : font arg is NULL  .\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : dst arg is NULL  .\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    if(IS_NULL(text))
    {
        fprintf(stderr, "Err at %s / %d : text arg is NULL  .\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    SDL_Rect clip = font->dimChar;
    CEV_rectDimCopy(clip, &pos);

    int index = 0;

    while(text[index] != '\0')
    {
        clip.x = (toupper(text[index++]) - ' ') * clip.w;
        CEV_blitSurfaceToTexture(font->font, &clip, dst, &pos);
        pos.x += clip.w;
    }

    return FUNC_OK;
}




