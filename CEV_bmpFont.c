#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <SDL.h>
#include "CEV_mixSystem.h"
#include "CEV_api.h"
#include "CEV_bmpFont.h"
#include "CEV_types.h"
#include "CEV_dataFile.h"
#include "rwtypes.h"


void CEV_BMPFont_Test(void)
{
    bool reload = 0,
         save   = 1;

    SDL_Renderer* render    = CEV_videoSystemGet()->render;
    CEV_BmpFont* font       = NULL;
    SDL_Texture* tex        = SDL_CreateTexture(render, CEV_PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);


    if(!reload)
        font = CEV_bmpFontCreate( CEV_surfaceLoad("dialog/nfo-font_6x8.png"), 223);
    else
        font = CEV_bmpFontLoad("testFont.fnt");

    CEV_bmpFontDispText(font, tex, "Coucou c'est cedric !!", (SDL_Rect){20, 20, 0, 0});

    for(int i=SDL_ALPHA_TRANSPARENT; i<= SDL_ALPHA_OPAQUE; i+=5)
    {
        //printf("%d\n", i);
        SDL_RenderClear(render);
        SDL_SetTextureAlphaMod(tex, i);
        SDL_RenderCopy(render, tex, NULL, NULL);
        SDL_RenderPresent(render);
        SDL_Delay(20);
    }

    SDL_Delay (1000);

    if(save)
        CEV_bmpFontSave(font, "testFont.fnt");

    CEV_bmpFontDestroy(font);
    SDL_DestroyTexture(tex);

}


void CEV_bmpFontDump(CEV_BmpFont* this)
{//dumps structure content

    puts("*** BEGIN CEV_BmpFont ***");

    if(IS_NULL(this))
	{
		puts("This BmpFont is NULL");
        goto end;
	}

    printf("\tis at %p\n", this);
    printf("\tid : %08X\n", this->id);
    //printf("\tpicture ID : %08X\n", this->picId);
    printf("\tNum of characters : %d\n", this->numOfChar);

    printf("\tsingle char dimension is : ");
    CEV_rectDump(this->charDim);
    printf("\tpicture dimension is : ");
    CEV_rectDump(this->picDim);

    printf("\tsurface is at %p\n", this->pic);
	printf("\town sprite's content is : \n");

end:
    puts("***END CEV_BmpFont ***");
}


CEV_BmpFont* CEV_bmpFontCreateFrom(SDL_Surface* fonts, int which, CEV_BmpFontSize size)
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

    result->pic = SDL_CreateRGBSurface(0, surfaceW, surfaceH, 32, rmask, gmask, bmask, amask);

    if(!result->pic)
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
    SDL_BlitSurface(fonts, &clip, result->pic, NULL);

    //updating informations
    result->charDim = (SDL_Rect){0, 0, size, size};
    result->picDim = (SDL_Rect){0, 0, size*CEV_BMPFONT_CHAR_NUM, size};


    return result;

err:

    free(result);

    return NULL;
}


CEV_BmpFont* CEV_bmpFontCreate(SDL_Surface* pic, uint32_t numOfChar)
{//allocates / fill new structure

    if(IS_NULL(pic) || !numOfChar)
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    CEV_BmpFont* result = calloc(1, sizeof(CEV_BmpFont));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    CEV_bmpFontSurfaceAttach(pic, numOfChar, result);

    return result;
}


void CEV_bmpFontDestroy(CEV_BmpFont* this)
{//destroying bmp font structure;

    if(IS_NULL(this))
        return;

    CEV_bmpFontClear(this);
    free(this);
}


void CEV_bmpFontClear(CEV_BmpFont* this)
{//clearing bmp font content

    if(IS_NULL(this))
        return;

    SDL_FreeSurface(this->pic);

    this->pic       = NULL;
    this->id        = CEV_ftypeToId(IS_BMPFONT);
    this->picDim    = CLEAR_RECT;
    this->charDim   = CLEAR_RECT;
}


CEV_BmpFont* CEV_bmpFontLoad(const char* fileName)
{//loads bmp font from file

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    SDL_RWops* file = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : unable to open %s : %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
        return NULL;
    }

    return CEV_bmpFontLoad_RW(file, true);
}


CEV_BmpFont* CEV_bmpFontLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads from RWops

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    CEV_BmpFont *result = calloc(1, sizeof(CEV_BmpFont));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    CEV_bmpFontTypeRead_RW(src, result, false);

end:
    if(freeSrc)
        SDL_RWclose(src);

    return result;

}


int CEV_bmpFontSave(CEV_BmpFont* src, const char* fileName)
{//saves bmp font into file

    if(IS_NULL(src) || IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    FILE *fileDst = fopen(fileName, "wb");

    if(IS_NULL(fileDst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    funcSts = CEV_bmpFontTypeWrite(src, fileDst);

    fclose(fileDst);

    return funcSts? FUNC_ERR : FUNC_OK;
}


int CEV_bmpFontTypeRead(FILE* src, CEV_BmpFont* dst)
{//reads bmp front from file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_Capsule caps = {0};

    dst->id         = read_u32le(src);
    //dst->picId      = read_u32le(src);
    dst->numOfChar  = read_u32le(src);

    CEV_capsuleTypeRead(src, &caps);

    dst->pic = CEV_surfaceFromCapsule(&caps, true);

    if(NOT_NULL(dst->pic))
    {
        dst->picDim     = (SDL_Rect){0, 0, dst->pic->w, dst->pic->h};
        dst->charDim    = (SDL_Rect){0, 0, dst->pic->w / dst->numOfChar, dst->pic->h};
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : BmpFont provided does not contain surface.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_bmpFontTypeWrite(CEV_BmpFont* src, FILE* dst)
{//writes bmp font into file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    CEV_Capsule caps = {0};

    write_u32le(src->id, dst);
    //write_u32le(src->picId, dst);
    write_u32le(src->numOfChar, dst);

    funcSts = CEV_surfaceToCapsule(src->pic, &caps);

    if(funcSts != FUNC_OK)
        return funcSts;

    CEV_capsuleTypeWrite(&caps, dst);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}


int CEV_bmpFontTypeRead_RW(SDL_RWops* src, CEV_BmpFont* dst, bool freeSrc)
{//reads bmp font from rwops

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_Capsule caps = {0};

    dst->id         = SDL_ReadLE32(src);
    //dst->picId      = read_u32le(src);
    dst->numOfChar  = SDL_ReadLE32(src);

    CEV_capsuleTypeRead_RW(src, &caps);

    dst->pic = CEV_surfaceFromCapsule(&caps, true);

    if(NOT_NULL(dst->pic))
    {
        dst->picDim     = (SDL_Rect){0, 0, dst->pic->w, dst->pic->h};
        dst->charDim    = (SDL_Rect){0, 0, dst->pic->w / dst->numOfChar, dst->pic->h};
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : BmpFont provided does not contain surface.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    if(freeSrc)
        SDL_RWclose(src);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_bmpFontTypeWrite_RW(CEV_BmpFont* src, SDL_RWops* dst)
{//writes bmp font into rwops

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    CEV_Capsule caps = {0};

    SDL_WriteLE32(dst, src->id);
    //SDL_WriteLE32(dst, src->picId);
    SDL_WriteLE32(dst, src->numOfChar);

    funcSts = CEV_surfaceToCapsule(src->pic, &caps);

    if(funcSts != FUNC_OK)
        return funcSts;

    CEV_capsuleTypeWrite_RW(&caps, dst);

    CEV_capsuleClear(&caps);

    return funcSts;
}


int CEV_bmpFontSurfaceAttach(SDL_Surface *src, uint32_t numOfChar, CEV_BmpFont *dst)
{//attaches surface / any previous existing one is destroyed

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_bmpFontClear(dst);

    dst->pic        = src;
    dst->numOfChar  = numOfChar;
    dst->picDim     = (SDL_Rect){0, 0, src->w, src->h};
    dst->charDim    = (SDL_Rect){0, 0, src->w / numOfChar, src->h};

    return FUNC_OK;
}


int CEV_bmpFontDispValue(CEV_BmpFont* src, SDL_Texture* dst, int value, SDL_Rect pos)
{//displays value

    char text[CEV_BMPFONT_MAX_DIGIT]    = {0};
    int funcSts                         = FUNC_OK;

    //on formate le texte à afficher
    sprintf(text, "%d", value);

    //on crée la surface texte
    funcSts = CEV_bmpFontDispText(src, dst, text, pos);

    return(funcSts);
}


int CEV_bmpFontDispText(CEV_BmpFont* src, SDL_Texture* dst, const char* text, SDL_Rect pos)
{//displays text from bmp font

    int funcSts = FUNC_OK;

    if(IS_NULL(src) || IS_NULL(dst) || IS_NULL(text))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    SDL_Rect clip = src->charDim;
    CEV_rectDimCopy(clip, &pos);

    int index = 0;

    while((text[index] != '\0') && !funcSts)
    {
        clip.x = (text[index++] - ' ') * clip.w;
        funcSts = CEV_blitSurfaceToTexture(src->pic, dst, &clip, &pos);
        pos.x += clip.w;
    }

    return funcSts;
}




