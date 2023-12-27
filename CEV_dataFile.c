//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  05-2016      |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**   CEV    |  05-2018      |   1.0.2  | map added      **/
//**   CEV    |  07-2019      |   1.0.3  | parallax added **/
//**   CEV    |  01-2020      |   1.0.4  | weather added  **/
//**   CEV    |  02-2022      |   2.0.0  | ID compliant   **/
//**********************************************************/
//- CEV 2021 05 20- removed capsule data free from L_capsuleToXxx functions -> capsule cleared in calling functions.
//- CEV 2022 07 24- Added Capsule functions set
//- CEV 2023 03 19- V2.0, now works with capsules IDs or index.

/** \file   CEV_dataFile.c
 * \author  CEV
 * \version 2.0.0
 * \date    March 2023
 * \brief   Multi file in one resources.
 *
 * \details
 */


// TODO (drx#2#04/23/23): Finir les fonctions en version fetchtByIndex, fetchByIdFromFile, fetchByIndexFromFile.
// TODO (drx#2#04/23/23): Peut-être passer en lecture directe du fichier et éviter de passer par la capsule puis le Rwops
// TODO (drx#2#08/26/23): Ajouter le BmpFont

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CEV_gif.h"
#include "CEV_file.h"
#include "project_def.h"
#include "CEV_mixSystem.h"
#include "rwtypes.h"
#include "CEV_dataFile.h"
#include "CEV_animator.h"
#include "CEV_scroll.h"
#include "CEV_texts.h"
#include "CEV_menu.h"
#include "CEV_maps.h"
#include "CEV_parallax.h"
#include "CEV_weather.h"
#include "CEV_aniMini.h"
#include "CEV_objects.h"

/***** Local functions****/

//raw mem into CEV_Text
static CEV_Text* L_capsuleToTxt(CEV_Capsule* caps);

//raw mem into SDL_Texture
static SDL_Texture* L_capsuleToPic(CEV_Capsule* caps);

//raw mem into CEV_Gif
static CEV_GifAnim* L_capsuleToGif(CEV_Capsule *caps);

//raw mem into CEV_Chunk (wav)
static CEV_Chunk* L_capsuleToWav(CEV_Capsule *caps);

//raw mem into CEV_Music (mp3)
static CEV_Music* L_capsuleToMusic(CEV_Capsule *caps);

//raw mem into CEV_Font (TTF_Font)
static CEV_Font* L_capsuleToFont(CEV_Capsule* caps);

//raw mem into animations set
static SP_Anim* L_capsuleToAnim(CEV_Capsule* caps);

//raw mem into menu
static CEV_Menu* L_capsuleToMenu(CEV_Capsule* caps);

//raw to CEV_TileMap
static CEV_TileMap* L_capsuleToMap(CEV_Capsule* caps);

//raw to CEV_ScrollText
static CEV_ScrollText* L_capsuleToScroll(CEV_Capsule* caps);

//raw to parallax background
static CEV_Parallax* L_capsuleToParallax(CEV_Capsule* caps);

//raw to weather
static CEV_Weather* L_capsuleToWeather(CEV_Capsule* caps);

//raw to CEV_Animini
static CEV_AniMini* L_capsuleToAniMini(CEV_Capsule* caps);

//raw to object / performs selection
static void* L_capsuleToObject(CEV_Capsule *caps);


/*file manipulation*/
//static size_t L_gotoDataIndex(unsigned int index, FILE* file);  //unused in v2 ?
//static void L_gotoAndDiscard(unsigned int index, FILE *file);   //unused in v2 ?

////reads capsule from file
//static void L_capsuleDataTypeRead(FILE* src, CEV_Capsule* dst);
//
////reads capsule from RWpos
//static void L_capsuleDataTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst);

//return offset pos in file from id
static size_t L_rsrcIdToOffset(uint32_t id, CEV_RsrcFile* src);

//return offset pos in file from index
static size_t L_rsrcIndexToOffset(uint32_t index, CEV_RsrcFile* src);

//reads resource file header
static void L_rsrcFileHeaderRead(FILE* src, CEV_RsrcFileHeader* dst);


//Ressource file handling

int CEV_rsrcLoad(const char* fileName, CEV_RsrcFile* dst)
{//loads file & fills resource holder _v2

    int funcSts = FUNC_OK;
    FILE* src   = NULL;

    readWriteErr = 0;

    src = fopen(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    dst->fSrc = src;

    rewind(src);

    dst->numOfFiles = read_u32le(src);

    dst->list = calloc(dst->numOfFiles, sizeof(CEV_RsrcFileHeader));

    if(IS_NULL(dst->list))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    for(unsigned i=0; i< dst->numOfFiles; i++)
    {
        L_rsrcFileHeaderRead(src, &dst->list[i]);
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;

err_1:
    fclose(src);

    return funcSts;

}


void CEV_rsrcClear(CEV_RsrcFile* dst)
{//clears structure content _v2

    if(IS_NULL(dst))
        return;

    free(dst->list);
    fclose(dst->fSrc);

    *dst = (CEV_RsrcFile){0};
}



/*----Type Any----*/


void* CEV_anyFetchById(uint32_t id, CEV_RsrcFile* src)
{//returns finished load of any file type _v2

    CEV_Capsule lCaps   = {0};//raw data informations
    void* result        = NULL; //result

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (CEV_capsuleFetchById(id, src, &lCaps) != FUNC_OK)//loads raw data into lCaps
    {
        fprintf(stderr, "Err at %s / %d : Failed fetching capsule.\n", __FUNCTION__, __LINE__ );
        goto exit;
    }

    result = CEV_capsuleExtract(&lCaps, true); //will free caps.data only if possible.

    if(IS_NULL(result))
    {
        fprintf(stderr, "Warn at %s / %d : Capsule extraction failed.\n", __FUNCTION__, __LINE__ );
    }

exit:
    return result;
}


void* CEV_anyFetchByIndex(uint32_t index, CEV_RsrcFile* src)
{//returns finished load of any file type _v2

    CEV_Capsule lCaps   = {0};//raw data informations
    void* result        = NULL; //result

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (CEV_capsuleFetchByIndex(index, src, &lCaps) != FUNC_OK)//loads raw data into lCaps
    {
        fprintf(stderr, "Err at %s / %d : Failed fetching capsule.\n", __FUNCTION__, __LINE__ );
        goto exit;
    }

    result = CEV_capsuleExtract(&lCaps, true); //will free caps.data only if possible.

    if(IS_NULL(result))
    {
        fprintf(stderr, "Warn at %s / %d : Capsule extraction failed.\n", __FUNCTION__, __LINE__ );
    }

exit:
    return result;
}


void* CEV_anyFetchByIdFromFile(uint32_t id, const char* fileName)
{//returns finished load of any file from file

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_RsrcFile rsrc = {0}; //file handle
    void *result = NULL;

    if(!CEV_rsrcLoad(fileName, &rsrc)) //loading file
        result = CEV_anyFetchById(id, &rsrc);//fetching requested resource

    if(IS_NULL(result))
    {//on error
        fprintf(stderr, "Err at %s / %d : Unable to load %08X from %s.\n", __FUNCTION__, __LINE__, id, fileName);
    }

    CEV_rsrcClear(&rsrc);//clearing

    return result;
}


void* CEV_anyFetchByIndexFromFile(uint32_t index, const char* fileName)
{//returns finished load of any file from file

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_RsrcFile rsrc = {0}; //file handle
    void *result = NULL;

    if(!CEV_rsrcLoad(fileName, &rsrc)) //loading file
        result = CEV_anyFetchByIndex(index, &rsrc);//fetching requested resource

    if(IS_NULL(result))
    {//on error
        fprintf(stderr, "Err at %s / %d : Unable to load %08X from %s.\n", __FUNCTION__, __LINE__, index, fileName);
    }

    CEV_rsrcClear(&rsrc);//clearing

    return result;
}



/*// TODO (drx#8#): à tester...
void* CEV_anyFetchByIdByIndex(uint32_t index, char* fileName, int* type)
{
    void* result = NULL;

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    FILE* src = fopen(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    uint32_t numOfFile = read_u32le(src);

    if(index >= numOfFile)
    {
        fprintf(stderr, "Err at %s / %d : Index provided is higher than resources available in file.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    //going to indexth header
    fseek(src, index * 64, SEEK_CUR);

    CEV_RsrcFileHeader header;
    L_rsrcFileHeaderRead(src, &header);

    fseek(src, header.offset, SEEK_SET);

    CEV_Capsule caps = {0};

    CEV_capsuleTypeRead(src, &caps);

    if(NOT_NULL(type))
    {
        *type = caps->type;
    }

    result = CEV_capsuleExtract(&caps, true);


end:
    fclose(src);

    return result;
}
*/


int CEV_capsuleFetchById(uint32_t id, CEV_RsrcFile* src, CEV_Capsule* dst)
{//gets capsule from file holder base on id

    if((src == NULL) || (dst == NULL))
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    size_t offset = L_rsrcIdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id provided does not exist.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, dst);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_capsuleFetchByIndex(uint32_t index, CEV_RsrcFile* src, CEV_Capsule* dst)
{//gets capsule from file holder _based on index

    if((src == NULL) || (dst == NULL))
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    size_t offset = L_rsrcIndexToOffset(index, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id provided does not exist.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, dst);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_capsuleFetchByIdFromFile(uint32_t id, const char* fileName, CEV_Capsule* dst)
{//gets capsule from file base on id

    CEV_RsrcFile rsrc = {0};

    if((fileName == NULL) || (dst == NULL))
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc)) //loading file
        CEV_capsuleFetchById(id, &rsrc, dst);

    CEV_rsrcClear(&rsrc);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_capsuleFetchByIndexFromFile(uint32_t index, const char* fileName, CEV_Capsule* dst)
{//gets capsule from file base on id

    CEV_RsrcFile rsrc = {0};

    if((fileName == NULL) || (dst == NULL))
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc)) //loading file
        CEV_capsuleFetchByIndex(index, &rsrc, dst);

    CEV_rsrcClear(&rsrc);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


//int CEV_capsuleFromFile(CEV_Capsule* caps, const char* fileName)
//{//fills Capsule from file _v2
//
//    FILE* file  = NULL;
//    readWriteErr = 0;
//
//    if(IS_NULL(caps) || IS_NULL(fileName))
//    {
//        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
//        return ARG_ERR;
//    }
//
//    file = fopen(fileName, "rb");
//
//    if (IS_NULL(file))
//    {
//        printf("Err at %s / %d : Unable to open file %s : %s\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
//        return FUNC_ERR;
//    }
//
//    caps->type = CEV_fTypeFromName(fileName);
//    caps->size = CEV_fileSize(file);
//    rewind(file);
//
//    L_capsuleDataTypeRead(file, caps);//allocs & fills data field
//
//    fclose(file);
//
//    return (readWriteErr)? FUNC_ERR : FUNC_OK;
//}


void* CEV_capsuleExtract(CEV_Capsule* caps, bool freeData)
{//extracts result of capsule content _v2

    void* returnVal = NULL;

    if(IS_NULL(caps))
    {
        fprintf(stderr, "Err at %s / %d : NULL pointer provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    if(IS_NULL(caps->data))
    {
        fprintf(stderr, "Err at %s / %d : Capsule contains no data.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    switch (caps->type)
    {
        case IS_DEFAULT:
        case IS_DAT:

            returnVal = malloc(caps->size);

            if (returnVal == NULL)
            {
                fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));
                goto exit;
            }
            CEV_memCopy(caps->data, returnVal, caps->size);

        break;

        case IS_DTX : //returns CEV_Text ptr
            returnVal = L_capsuleToTxt(caps);
        break;

        case IS_BMP :
        case IS_PNG :
        case IS_JPG :
            returnVal = L_capsuleToPic(caps); //returns SDL_Texture ptr
        break;

        case IS_GIF :
            returnVal = L_capsuleToGif(caps); //returns SDL_GIFAnim ptr
        break;

        case IS_WAV :
            returnVal = L_capsuleToWav(caps); //returns CEV_chunk ptr

            if(returnVal)
                freeData = false; //DO NOT free data, raw file is used for read purposes.
        break;

        case IS_FONT :
            returnVal = L_capsuleToFont(caps); //returns CEV_Font ptr

            if(returnVal)
                freeData = false; //DO NOT free data, raw file is used for read purposes.
        break;

        case IS_SPS :
            returnVal = L_capsuleToAnim(caps); //returns SP_Anim ptr
        break;

        case IS_MENU :
            returnVal = L_capsuleToMenu(caps);//returns menu ptr
        break;

        case IS_SCROLL :
            returnVal = L_capsuleToScroll(caps);//extraction reads file directly
        break;

        case IS_MAP :
            returnVal = L_capsuleToMap(caps);
        break;

        case IS_MUSIC :
            returnVal = L_capsuleToMusic(caps);

            if(returnVal)
                freeData = false; //DO NOT free data, raw file is used for read purposes.
        break;

        case IS_PRLX :
            returnVal = L_capsuleToParallax(caps);
        break;

        case IS_WTHR :
            returnVal = L_capsuleToWeather(caps);
        break;

        case IS_ANI :
            returnVal = L_capsuleToAniMini(caps);
        break;

        case IS_OBJ :
            returnVal = L_capsuleToObject(caps);
        break;

        default :
            fprintf(stderr,"Warn at %s / %d : Unrecognized file format.\n", __FUNCTION__, __LINE__);//should not occur
            return NULL;
        break;
    }

    if(freeData)
        CEV_capsuleClear(caps);

exit:
    return returnVal;
}


/*----SDL_Surfaces----*/

SDL_Surface* CEV_surfaceLoad(const char* fileName)
{//direct load from any file _v2

    SDL_Surface *newSurface     = NULL,
                *tempSurface    = NULL;

    if(fileName == NULL)
        return NULL;

    tempSurface = IMG_Load(fileName);

    if(tempSurface == NULL)
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());

    newSurface = SDL_ConvertSurfaceFormat(tempSurface, CEV_PIXELFORMAT, 0);
    SDL_SetSurfaceBlendMode(newSurface, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(tempSurface);

    /*---POST---*/
    return newSurface;
}


SDL_Surface* CEV_surfaceFetchById(uint32_t id, CEV_RsrcFile* src)
{//fetch data index as SDL_Surface from resources based on id

    SDL_Surface *surface    = NULL,
                *temp       = NULL;
    CEV_Capsule  lCaps      = {0};


    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr,"Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    size_t offset = L_rsrcIdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %08X provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (!IS_PIC(lCaps.type))
    {
        fprintf(stderr,"Err at %s / %d : id  %08X provided is not a picture.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    temp = IMG_Load_RW(SDL_RWFromMem(lCaps.data, lCaps.size), 1);

    if(IS_NULL(temp))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto err_1;
    }

    surface = SDL_ConvertSurfaceFormat(temp, CEV_PIXELFORMAT, 0);

    if(IS_NULL(surface))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto err_2;
    }

    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);


err_2:
    SDL_FreeSurface(temp);

err_1:
    CEV_capsuleClear(&lCaps);

end:
    return(surface);
}


SDL_Surface* CEV_surfaceFetchByIndex(uint32_t index, CEV_RsrcFile* src)
{//fetch data index as SDL_Surface from resources based on index

    SDL_Surface *surface    = NULL,
                *temp       = NULL;
    CEV_Capsule  lCaps      = {0};


    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr,"Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    size_t offset = L_rsrcIndexToOffset(index, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : index %u provided does not exist.\n", __FUNCTION__, __LINE__, index);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (!IS_PIC(lCaps.type))
    {
        fprintf(stderr,"Err at %s / %d : index  %u provided is not a picture.\n", __FUNCTION__, __LINE__, index);
        goto err_1;
    }

    temp = IMG_Load_RW(SDL_RWFromMem(lCaps.data, lCaps.size), 1);

    if(IS_NULL(temp))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto err_1;
    }

    surface = SDL_ConvertSurfaceFormat(temp, CEV_PIXELFORMAT, 0);

    if(IS_NULL(surface))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto err_2;
    }

    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);


err_2:
    SDL_FreeSurface(temp);

err_1:
    CEV_capsuleClear(&lCaps);

end:
    return(surface);
}


SDL_Surface* CEV_surfaceFetchByIdFromFile(uint32_t id, const char* fileName)
{//fetch data id as SDL_Surface from compiled file _v2

    SDL_Surface *surface    = NULL;
    CEV_RsrcFile rsrc = {0};


    if(IS_NULL(fileName))
    {
        fprintf(stderr,"Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        surface = CEV_surfaceFetchById(id, &rsrc);

    if(IS_NULL(surface))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load surface.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

end:
    return(surface);
}


SDL_Surface* CEV_surfaceFetchByIndexFromFile(uint32_t index, const char* fileName)
{//fetch data index as SDL_Surface from compiled file _v2

    SDL_Surface *surface    = NULL;
    CEV_RsrcFile rsrc = {0};


    if(IS_NULL(fileName))
    {
        fprintf(stderr,"Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        surface = CEV_surfaceFetchByIndex(index, &rsrc);

    if(IS_NULL(surface))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load surface.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

end:
    return(surface);
}


SDL_Surface* CEV_surfaceFromCapsule(CEV_Capsule* src, bool freeSrc)
{//extracts surface from CEV_Capsule

    SDL_Surface *result = NULL;

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    if(!IS_PIC(src->type))
    {
        fprintf(stderr, "Err at %s / %d : CEV_Capsule is not picture.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    SDL_RWops *ops      = SDL_RWFromConstMem(src->data, src->size);
    SDL_Surface *loaded = IMG_Load_RW(ops, true);
    result              = SDL_ConvertSurfaceFormat(loaded, CEV_PIXELFORMAT, 0);
    SDL_FreeSurface(loaded);

end:

    if(freeSrc)
        CEV_capsuleClear(src);

    return result;
}


/*----SDL_Textures----*/

SDL_Texture* CEV_textureLoad(const char* fileName)
{//direct load from file as texture _v2

    SDL_Texture *texture    = NULL;;
    SDL_Renderer *render    = CEV_videoSystemGet()->render;

    texture = IMG_LoadTexture(render, fileName);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load %s, %s\n",__FUNCTION__, __LINE__, fileName, IMG_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}


SDL_Texture* CEV_textureFetchById(uint32_t id, CEV_RsrcFile* src)
{//fetchs data id as SDL_Texture in resources _v2

    SDL_Texture* texture    = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if(!IS_PIC(lCaps.type))
    {//checking we have pic
        fprintf(stderr,"Err at %s / %d : id %08X provided is not a picture.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    texture = L_capsuleToPic(&lCaps);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load texture.\n", __FUNCTION__, __LINE__);
        goto err_1;
    }

    //SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

err_1:
    CEV_capsuleClear(&lCaps);

    return(texture);
}


SDL_Texture* CEV_textureFetchByIndex(uint32_t index, CEV_RsrcFile* src)
{//fetchs data index as SDL_Texture in resources _v2

    SDL_Texture* texture    = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if(!IS_PIC(lCaps.type))
    {//checking we have pic
        fprintf(stderr,"Err at %s / %d : index %u provided is not a picture.\n", __FUNCTION__, __LINE__, index);
        goto err_1;
    }

    texture = L_capsuleToPic(&lCaps);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load texture.\n", __FUNCTION__, __LINE__);
        goto err_1;
    }

    //SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

err_1:
    CEV_capsuleClear(&lCaps);

    return(texture);
}


SDL_Texture* CEV_textureFetchByIdFromFile(uint32_t id, const char* fileName)
{//fetchs data id as SDL_Texture in file

    SDL_Texture* texture    = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(CEV_rsrcLoad(fileName, &rsrc)) //loading file
        texture = CEV_textureFetchById(id, &rsrc);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load texture.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

    return(texture);
}


SDL_Texture* CEV_textureFetchByIndexFromFile(uint32_t index, const char* fileName)
{//fetchs data index as SDL_Texture in file

    SDL_Texture* texture    = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(CEV_rsrcLoad(fileName, &rsrc)) //loading file
        texture = CEV_textureFetchByIndex(index, &rsrc);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load texture.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

    return(texture);
}


/*-----Texts from compiled file-----*/

CEV_Text* CEV_textFetchById(uint32_t id, CEV_RsrcFile* src)
{//fetches CEV_Text from ressouces id based

    CEV_Text* result    = NULL;
    CEV_Capsule lCaps   ={0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if(lCaps.type != IS_DTX)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not text.\n", __FUNCTION__, __LINE__, id);
        goto err_1;

    }

    result = L_capsuleToTxt(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load result.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Text* CEV_textFetchByIndex(uint32_t index, CEV_RsrcFile* src)
{//fetches CEV_Text from ressouces index based

    CEV_Text* result    = NULL;
    CEV_Capsule lCaps   ={0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if(lCaps.type != IS_DTX)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not text.\n", __FUNCTION__, __LINE__, index);
        goto err_1;

    }

    result = L_capsuleToTxt(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load result.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Text* CEV_textFetchByIdFromFile(uint32_t id, const char* fileName)
{//fetches CEV_Text from file id based

    CEV_Text* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(CEV_rsrcLoad(fileName, &rsrc)) //loading file
        result = CEV_textFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load result.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

    return(result);
}


CEV_Text* CEV_textFetchByIndexFromFile(uint32_t index, const char* fileName)
{//fetches CEV_Text from file index based

    CEV_Text* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(CEV_rsrcLoad(fileName, &rsrc)) //loading file
        result = CEV_textFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load result.\n", __FUNCTION__, __LINE__);
    }

    CEV_rsrcClear(&rsrc);

    return(result);
}



/*----TTF_Font from compiled file----*/

CEV_Font* CEV_fontFetchById(int32_t id, CEV_RsrcFile* src)
{//fetch font from resources id based

    CEV_Font* result    = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if(lCaps.type != IS_FONT)
    {
        fprintf(stderr, "Err at %s / %d : id %08X provided is not font.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    result = L_capsuleToFont(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Font* CEV_fontFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetch font from resources index based

    CEV_Font* result    = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if(lCaps.type != IS_FONT)
    {
        fprintf(stderr, "Err at %s / %d : index %u provided is not font.\n", __FUNCTION__, __LINE__, index);
        goto err_1;
    }

    result = L_capsuleToFont(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Font* CEV_fontFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch font from file id based

    CEV_Font* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_fontFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Font* CEV_fontFetchByIndexFromFile(int32_t index,  const char* fileName)
{//fetch font from file index based

    CEV_Font* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_fontFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}



/*----WAV from compiled file----*/

CEV_Chunk* CEV_waveFetchById(int32_t id, CEV_RsrcFile* src)
{//fetches wave in resources _v2

    CEV_Chunk * result  = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_WAV)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not wav.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToWav(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load wav.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Chunk* CEV_waveFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetches wave in resources _v2

    CEV_Chunk * result  = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_WAV)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not wav.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToWav(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load wav.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Chunk* CEV_waveFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch wave from file id based

    CEV_Chunk* result       = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_waveFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load wave.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Chunk* CEV_waveFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch wave from file index based

    CEV_Chunk* result       = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_waveFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load wave.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


/*---MUSIC from compiled file---*/

CEV_Music* CEV_musicFetchById(int32_t id, CEV_RsrcFile* src)
{//fetches CEV_Music in resources id based

    CEV_Music* result   = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_MUSIC)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not music.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMusic(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load music.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Music* CEV_musicFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetches CEV_Music in resources index based

    CEV_Music* result   = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_MUSIC)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not music.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToMusic(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load music.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Music* CEV_musicFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch music from file id based

    CEV_Music* result       = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_musicFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load music.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Music* CEV_musicFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch music from file index based

    CEV_Music* result       = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_musicFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load music.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}



/*----animations (sprite resource) from compiled file----*/

SP_Anim* CEV_animFetchById(int32_t id, CEV_RsrcFile* src)
{//fetch sprite in resources id based _v2

    SP_Anim* result     = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_SPS)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not sprite.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToAnim(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load sprite.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


SP_Anim* CEV_animFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetch sprite in resources index based _v2

    SP_Anim* result     = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_SPS)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not sprite.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToAnim(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load sprite.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


SP_Anim* CEV_animFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch SP_Anim from file id based

    SP_Anim* result         = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_animFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load sprite.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


SP_Anim* CEV_animFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch SP_Anim from file index based

    SP_Anim* result         = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_animFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load sprite.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}



/*----gif animation from compiled file----*/

CEV_GifAnim* CEV_gifFetchById(int32_t id, CEV_RsrcFile* src)
{//fetch gif file in resources by id _v2

    CEV_GifAnim* result = NULL;
    CEV_Capsule  lCaps  = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_GIF)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not gif.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToGif(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load gif.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_GifAnim* CEV_gifFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetch gif file in resources by index _v2

    CEV_GifAnim* result = NULL;
    CEV_Capsule  lCaps  = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_GIF)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not gif.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToGif(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load gif.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_GifAnim* CEV_gifFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_GifAnim from file id based

    CEV_GifAnim* result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_gifFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load gif.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_GifAnim* CEV_gifFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_GifAnim from file index based

    CEV_GifAnim* result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_gifFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load gif.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}



/*----Scroll fetch----*/

CEV_ScrollText* CEV_scrollFetchById(int32_t id, CEV_RsrcFile* src)
{//fetches scroll text in compiled data file _v2

    CEV_ScrollText* result  = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not scroller.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToScroll(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_ScrollText* CEV_scrollFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetches scroll text in compiled data file _v2

    CEV_ScrollText* result  = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not scroller.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToScroll(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_ScrollText* CEV_scrollFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_ScrollText from file id based

    CEV_ScrollText* result  = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_scrollFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_ScrollText* CEV_scrollFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_ScrollText from file index based

    CEV_ScrollText* result  = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_scrollFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


/*-----menu fetch-----*/

CEV_Menu* CEV_menuFetchById(int32_t id, CEV_RsrcFile* src)
{//fetch menu in compiled data file by id

    CEV_Menu* result    = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not scroller.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMenu(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Menu* CEV_menuFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetch menu in compiled data file by index

    CEV_Menu* result    = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not scroller.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToMenu(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Menu* CEV_menuFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_Menu from file id based

    CEV_Menu* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_menuFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Menu* CEV_menuFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_Menu from file index based

    CEV_Menu* result        = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_menuFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load font.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}



/*---- map fetch ----*/

CEV_TileMap* CEV_mapFetchById(int32_t id, CEV_RsrcFile* src)
{//fetches tile CEV_TileMap resources by id _v2

    CEV_TileMap *result = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_MAP)
    {
        fprintf(stderr,"Warn at %s / %d : id %08X provided is not map.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMap(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load map.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_TileMap* CEV_mapFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetches tile CEV_TileMap resources by index _v2

    CEV_TileMap *result = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_MAP)
    {
        fprintf(stderr,"Warn at %s / %d : index %u provided is not map.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToMap(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load map.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_TileMap* CEV_mapFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_TileMap from file id based

    CEV_TileMap* result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_mapFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load map.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_TileMap* CEV_mapFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_TileMap from file index based

    CEV_TileMap* result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_mapFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load map.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


/*----background parallax fetch ----*/

CEV_Parallax* CEV_parallaxFetchById(int32_t id, CEV_RsrcFile* src)
{//fetch CEV_Parallax from resources by id _v2

    CEV_Parallax * result   = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_PRLX)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not parallax.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToParallax(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load parallax.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Parallax* CEV_parallaxFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetch CEV_Parallax from resources by index _v2

    CEV_Parallax* result    = NULL;
    CEV_Capsule lCaps       = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_PRLX)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not parallax.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToParallax(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load parallax.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Parallax* CEV_parallaxFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_Parallax from file id based

    CEV_Parallax* result    = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_parallaxFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load Parallax.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Parallax* CEV_parallaxFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_Parallax from file index based

    CEV_Parallax* result    = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_parallaxFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load Parallax.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


/*----weather fetch----*/

CEV_Weather* CEV_weatherFetchById(int32_t id, CEV_RsrcFile* src)
{//fetches weather from resources by id

    CEV_Weather *result = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchById(id, src, &lCaps);

    if (lCaps.type != IS_WTHR)
    {
        fprintf(stderr,"Err at %s / %d : id %08X provided is not weather.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToWeather(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load weather.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Weather* CEV_weatherFetchByIndex(int32_t index, CEV_RsrcFile* src)
{//fetches weather from resources by index

    CEV_Weather *result = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    CEV_capsuleFetchByIndex(index, src, &lCaps);

    if (lCaps.type != IS_WTHR)
    {
        fprintf(stderr,"Err at %s / %d : index %u provided is not weather.\n", __FUNCTION__, __LINE__, index);
        goto end;
    }

    result = L_capsuleToWeather(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load weather.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_Weather* CEV_weatherFetchByIdFromFile(int32_t id, const char* fileName)
{//fetch CEV_Weather from file id based

    CEV_Weather *result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_weatherFetchById(id, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load weather.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}


CEV_Weather* CEV_weatherFetchByIndexFromFile(int32_t index, const char* fileName)
{//fetch CEV_Weather from file index based

    CEV_Weather* result     = NULL;
    CEV_RsrcFile rsrc = {0};

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if(!CEV_rsrcLoad(fileName, &rsrc))
        result = CEV_weatherFetchByIndex(index, &rsrc);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load weather.\n", __FUNCTION__, __LINE__ );
    }

    CEV_rsrcClear(&rsrc);

    return result;
}

/*----- Encapsulation -----


void CEV_capsuleTypeWrite(CEV_Capsule* src, FILE* dst)
{//writes capsule into file where it is

    write_u32le(src->type, dst);
    write_u32le(src->size, dst);

    if(fwrite(src->data, sizeof(char), src->size, dst) != src->size)
        readWriteErr++;
}


void CEV_capsuleTypeRead(FILE* src, CEV_Capsule* dst)
{//reads capsule from file where it is

    dst->type   = read_u32le(src);
    dst->size   = read_u32le(src);

    if(!readWriteErr)
        L_capsuleDataTypeRead(src, dst);//allocs & fills data field
}


void CEV_capsuleTypeWrite_RW(CEV_Capsule* src, SDL_RWops* dst)
{//writes capsule into RWops

    readWriteErr += SDL_WriteLE32(dst, src->type);
    readWriteErr += SDL_WriteLE32(dst, src->size);

    if(SDL_RWwrite(dst, src->data, 1, src->size) != src->size)
        readWriteErr++;
}


void CEV_capsuleTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst)
{//reads capsule from RWops

    dst->type   = SDL_ReadLE32(src);
    dst->size   = SDL_ReadLE32(src);

    L_capsuleDataTypeRead_RW(src, dst);//allocs & fills data field
}


void CEV_capsuleClear(CEV_Capsule* caps)
{//clears fileinfo content

    if(IS_NULL(caps))
        return;

    free(caps->data);

    *caps = (CEV_Capsule){NULL};
}


void CEV_capsuleDestroy(CEV_Capsule* caps)
{//frees fileinfo and content

    free(caps->data);
    free(caps);
}
--------------*/

int CEV_idTofType(uint32_t id)
{//id to file type
    return id>>24;
}


char* CEV_idToExt(uint32_t id)
{//id to file extension

    return CEV_fTypeToExt(CEV_idTofType(id));
}


uint32_t CEV_ftypeToId(int type)
{//file type to id

    return type<<24;
}


int CEV_fTypeFromName(const char* fileName)
{//turns file extention into file type enum

    char *extList[FILE_TYPE_NUM] = FILE_TYPE_LIST;  //list of known ext
    int i;
    size_t length   = 0;
    char *point     = fileName; //ptr to last '.' found

    length = strlen(fileName);

    for (i = length; (i >= 0) && (*point != '.'); i--)
        point = fileName+i;

    point++;

    for (i = 0; i < FILE_TYPE_NUM; i++)
    {
        if (!strcmp(point, extList[i]))
            return i;
    }

    return(IS_DEFAULT);
}


char* CEV_fTypeToExt(int type)
{//attributes extension

    char *extList[FILE_TYPE_NUM] = FILE_TYPE_LIST;

    if(type>0 && type<FILE_TYPE_NUM)
        return extList[type];

    return extList[0];
}

/*---Local Sub-functions not to be directly called---*/


static CEV_Text* L_capsuleToTxt(CEV_Capsule* caps)
{//extracts CEV_Text

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromConstMem(caps->data, caps->size);

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Text* result = CEV_textLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create texte from data.\n", __FUNCTION__, __LINE__);

    return(result);
}


static SDL_Texture* L_capsuleToPic(CEV_Capsule* caps)
{//extracts SDL_Texture

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromConstMem(caps->data, caps->size);

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    SDL_Renderer* render    = CEV_videoSystemGet()->render;
    SDL_Texture* pic        = IMG_LoadTexture_RW(render, vFile, 1);//closes rwops

    if (IS_NULL(pic))
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());
    else
        SDL_SetTextureBlendMode(pic, SDL_BLENDMODE_BLEND);

    return pic;
}


static CEV_GifAnim *L_capsuleToGif(CEV_Capsule* caps)
{//converts to a gif animation

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromConstMem(caps->data, caps->size);

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    SDL_Renderer* render    = CEV_videoSystemGet()->render;
    CEV_GifAnim* result     = CEV_gifAnimLoadRW(vFile, render, 1); //closes rwops

    if(result == NULL)
        fprintf(stderr, "Err at %s / %d : Unable to create animation from data.\n", __FUNCTION__, __LINE__);

    return result;
}


static CEV_Chunk* L_capsuleToWav(CEV_Capsule* caps)
{//convert to a wav sample

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))//MIX does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    Mix_Chunk* chunk = Mix_LoadWAV_RW(vFile, 1); //asks to close rwops but does not free datas

    if (chunk == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        return NULL;
    }

    CEV_Chunk* result = malloc(sizeof(*result));

    if (IS_NULL(result))
    {
         fprintf(stderr, "Err at %s / %d : Unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err;
    }

    result->sound       = chunk;
    result->virtualFile = caps->data;

    return result;

err:
    Mix_FreeChunk(chunk);
    return NULL;

}


static CEV_Music* L_capsuleToMusic(CEV_Capsule* caps)
{//convert to a music

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))//MIX does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    Mix_Music* music = Mix_LoadMUS_RW(vFile, 1); //ask to close rwops

    if (music == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        return NULL;
    }

    CEV_Music* result = malloc(sizeof(*result));

    if (result == NULL)
    {
         fprintf(stderr, "Err at %s / %d : Unable to allocate result : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err;
    }

    result->music       = music;
    result->virtualFile = caps->data;

    return result;

err:
    Mix_FreeMusic(music);
    return NULL;
}


static CEV_Font* L_capsuleToFont(CEV_Capsule* caps)
{//converts to CEV_Font

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    TTF_Font* font = TTF_OpenFontRW(vFile, 1, FONT_MAX);//asks to close rwops but doesn't free data

    if (IS_NULL(font))
    {
         fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
         return NULL;
    }

    CEV_Font* result = malloc(sizeof(*result));

    if (IS_NULL(result))
    {
         fprintf(stderr, "Err at %s / %d : Unable to allocate result: %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err;
    }

    result->font        = font;
    result->virtualFile = caps->data;

    return result;

err:
    TTF_CloseFont(font);
    return NULL;
}


static SP_Anim* L_capsuleToAnim(CEV_Capsule* caps)
{//converts to animSet

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    SP_Anim *result = SP_animLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create animSet.\n", __FUNCTION__, __LINE__);


    return result;
}


static CEV_TileMap* L_capsuleToMap(CEV_Capsule* caps)
{//convert to tile map

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_TileMap *result = CEV_mapLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create map.\n", __FUNCTION__, __LINE__);

    return result;
}


static CEV_Menu* L_capsuleToMenu(CEV_Capsule* caps)
{//converts to CEV_Menu

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Menu* result = CEV_menuLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create text scroll.\n", __FUNCTION__, __LINE__ );

    return result;


}


static CEV_ScrollText* L_capsuleToScroll(CEV_Capsule* caps)
{//converts to scroller

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_ScrollText* result = CEV_scrollLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create text scroll.\n", __FUNCTION__, __LINE__ );

    return result;
}


static CEV_Parallax* L_capsuleToParallax(CEV_Capsule* caps)
{//converts to parallax

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Parallax *result = CEV_parallaxLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create parallax.\n", __FUNCTION__, __LINE__);

    return  result;
}


static CEV_Weather* L_capsuleToWeather(CEV_Capsule* caps)
{//converts to weather

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Weather* result = CEV_weatherLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create weather.\n", __FUNCTION__, __LINE__);

    return result;
}



static CEV_AniMini* L_capsuleToAniMini(CEV_Capsule* caps)
{//converts to Animini

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_AniMini* result = CEV_aniMiniLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : Unable to create weather.\n", __FUNCTION__, __LINE__);

    return result;

}


static void* L_capsuleToObject(CEV_Capsule *caps)
{//converts to object

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

   return (void*)CEV_objectLoad_RW(vFile, true);
}


//static void L_capsuleDataTypeRead(FILE* src, CEV_Capsule* dst)
//{//fills and allocate caps->data 1.0
//
//    if(dst==NULL || src==NULL)
//        readWriteErr++;
//
//    dst->data = malloc(dst->size);
//
//    if (dst->data == NULL)
//    {
//        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
//        readWriteErr++;
//    }
//    else if (fread(dst->data, 1, dst->size, src) != dst->size)
//    {
//        fprintf(stderr, "Err at %s / %d : Unable to read data in src : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
//        CEV_capsuleClear(dst);
//        readWriteErr++;
//    }
//}
//
//
//static void L_capsuleDataTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst)
//{//fills and allocate dst->data 1.0
//
//    if(IS_NULL(dst) || IS_NULL(dst))
//    {
//        fprintf(stderr, "Err at %s / %d :  NULL arg provided.\n", __FUNCTION__, __LINE__ );
//        readWriteErr++;
//        return;
//    }
//
//    dst->data = malloc(dst->size);
//
//    if (IS_NULL(dst->data))
//    {
//        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
//        readWriteErr++;
//    }
//    else if (SDL_RWread(src, dst->data, 1, dst->size) != dst->size)
//    {
//        fprintf(stderr, "Err at %s / %d : Unable to read data in src : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
//        CEV_capsuleClear(dst);
//        readWriteErr++;
//    }
//}



static size_t L_rsrcIdToOffset(uint32_t id, CEV_RsrcFile* src)
{//finds offset of an id

    if(IS_NULL(src) || IS_NULL(src->list))
        return 0;

    for(unsigned i=0; i<src->numOfFiles; i++)
    {
        if(src->list[i].id == id)
            return src->list[i].offset;

    }

    return 0;
}


static size_t L_rsrcIndexToOffset(uint32_t index, CEV_RsrcFile* src)
{//finds offset of an id

    if(IS_NULL(src) || IS_NULL(src->list))
        return 0;

    if(index < src->numOfFiles)
        return src->list[index].offset;

    return 0;
}


static void L_rsrcFileHeaderRead(FILE* src, CEV_RsrcFileHeader* dst)
{//reads resource file header

    dst->id     = read_u32le(src);
    dst->offset = (size_t)read_u32le(src);

}
