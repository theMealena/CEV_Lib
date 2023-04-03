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
//- CEV 2023 03 19- V2.0, now works with capsules IDs instead of index.

/** \file   CEV_dataFile.c
 * \author  CEV
 * \version 2.0.0
 * \date    March 2023
 * \brief   Multi file in one ressources.
 *
 * \details
 */


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
static SP_AnimList* L_capsuleToAnimSet(CEV_Capsule* caps);

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


/*file manipulation*/
static size_t L_gotoDataIndex(unsigned int index, FILE* file);  //unused in v2 ?
static void L_gotoAndDiscard(unsigned int index, FILE *file);   //unused in v2 ?
static void L_capsuleDataTypeRead(FILE* src, CEV_Capsule* dst);
static void L_capsuleDataTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst);

//return offset pos in file from id
static size_t L_IdToOffset(uint32_t id, CEV_RsrcFileHolder* src);

//return offset pos in file from index
static size_t L_IndexToOffset(uint32_t index, CEV_RsrcFileHolder* src);

static void L_rsrcFileHolderHeaderRead(FILE* src, CEV_RsrcFileHolderHeader* dst);


//Ressource file handling

int CEV_rsrcLoad(const char* fileName, CEV_RsrcFileHolder* dst)
{//loads file & fills ressource holder _v2

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

    dst->list = calloc(dst->numOfFiles, sizeof(CEV_RsrcFileHolderHeader));

    if(IS_NULL(dst->list))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    for(int i=0; i< dst->numOfFiles; i++)
    {
        L_rsrcFileHolderHeaderRead(src, &dst->list[i]);
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;

err_1:
    fclose(src);

    return funcSts;

}


void CEV_rsrcClear(CEV_RsrcFileHolder* dst)
{//clears structure content _v2

    dst->numOfFiles = 0;

    fclose(dst->fSrc);
    free(dst->list);

    dst->fSrc = NULL;
    dst->list = NULL;
}



/*----Type Any----*/


void* CEV_anyFetch(uint32_t id, CEV_RsrcFileHolder* src)
{//returns finished load of any file type _v2

    CEV_Capsule lCaps   = {NULL};//raw data informations

    void* result        = NULL; //result


    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (CEV_capsuleFetch(id, src, &lCaps) != FUNC_OK)//loads raw data into lCaps
       goto exit;

    result = CEV_capsuleExtract(&lCaps, true); //will free caps.data only if possible.

exit:
    return result;
}


/*// TODO (drx#1#): à tester...
void* CEV_anyFetchByIndex(uint32_t index, char* fileName, int* type)
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
        fprintf(stderr, "Err at %s / %d : Index provided is higher than ressources available in file.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    //going to indexth header
    fseek(src, index * 64, SEEK_CUR);

    CEV_RsrcFileHolderHeader header;
    L_rsrcFileHolderHeaderRead(src, &header);

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


int CEV_capsuleFetch(uint32_t id, CEV_RsrcFileHolder* src, CEV_Capsule* dst)
{//gets capsule from file holder _v2


    if((src == NULL) || (dst == NULL))
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id provided does not exist.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, dst);

    return readWriteErr? FUNC_OK : FUNC_ERR;
}


int CEV_capsuleFromFile(CEV_Capsule* caps, const char* fileName)
{//fills Capsule from file _v2

    FILE* file  = NULL;
    readWriteErr = 0;

    if(IS_NULL(caps) || IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    file = fopen(fileName, "rb");

    if (IS_NULL(file))
    {
        printf("Err at %s / %d : unable to open file %s : %s\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return FUNC_ERR;
    }

    caps->type = CEV_fTypeFromName(fileName);
    caps->size = CEV_fileSize(file);
    rewind(file);

    L_capsuleDataTypeRead(file, caps);//allocs & fills data field

    fclose(file);

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
}


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
                fprintf(stderr, "Err at %s / %d : unable to allocate returnVal :%s\n", __FUNCTION__, __LINE__, strerror(errno));
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
            returnVal = L_capsuleToAnimSet(caps); //returns SP_AnimList ptr
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

        case IS_PLX :
            returnVal = L_capsuleToParallax(caps);
        break;

        case IS_WTHR :
            returnVal = L_capsuleToWeather(caps);
        break;

        default :
            fprintf(stderr,"Warn at %s / %d : unrecognized file format.\n", __FUNCTION__, __LINE__);//should not occur
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

    SDL_Surface *newSurface = NULL,
                *tempSurface = NULL;

    if(fileName == NULL)
        return NULL;

    tempSurface = IMG_Load(fileName);

    if(tempSurface == NULL)
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());

    newSurface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_SetSurfaceBlendMode(newSurface, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(tempSurface);

    /*---POST---*/
    return newSurface;
}


SDL_Surface* CEV_surfaceFetch(uint32_t id, CEV_RsrcFileHolder* src)
{//fetch data index as SDL_Surface in compiled file _v2

    SDL_Surface *surface    = NULL,
                *temp       = NULL;
    CEV_Capsule  lCaps      = {NULL};


    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr,"Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (!IS_PIC(lCaps.type))
    {
        fprintf(stderr,"Err at %s / %d : id  %u provided is not a picture.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    temp = IMG_Load_RW(SDL_RWFromMem(lCaps.data, lCaps.size), 1);

    if(IS_NULL(temp))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto err_1;
    }

    surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);

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



/*----SDL_Textures----*/

SDL_Texture* CEV_textureLoad(const char* fileName)
{//direct load from file as texture _v2

    SDL_Texture     *texture = NULL;;
    SDL_Renderer    *render  = CEV_videoSystemGet()->render;

    texture = IMG_LoadTexture(render, fileName);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n",__FUNCTION__, __LINE__, fileName, IMG_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}


SDL_Texture* CEV_textureFetch(uint32_t id, CEV_RsrcFileHolder* src)
{//fetchs data id as SDL_Texture in ressources _v2

    SDL_Texture* texture    = NULL;
    CEV_Capsule lCaps       = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if(!IS_PIC(lCaps.type))
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not a picture.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    texture = L_capsuleToPic(&lCaps);

    if(IS_NULL(texture))
    {
        fprintf(stderr, "Err at %s / %d : unable to load texture.\n", __FUNCTION__, __LINE__);
        goto err_1;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

err_1:
    CEV_capsuleClear(&lCaps);

    return(texture);
}


/*-----Texts from compiled file-----*/

CEV_Text* CEV_textFetch(uint32_t id, CEV_RsrcFileHolder* src)
{//fetches CEV_Text in ressouces _v2

    CEV_Text* result    = NULL;
    CEV_Capsule lCaps   ={NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if(lCaps.type != IS_DTX)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not text.\n", __FUNCTION__, __LINE__);
        goto err_1;

    }

    result = L_capsuleToTxt(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load result.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----TTF_Font from compiled file----*/

CEV_Font* CEV_fontFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetch font in compiled data file and fills lCaps _v2

    CEV_Font* result    = NULL;
    CEV_Capsule lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if(lCaps.type != IS_FONT)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided is not font.\n", __FUNCTION__, __LINE__, id);
        goto err_1;
    }

    result = L_capsuleToFont(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load font.\n", __FUNCTION__, __LINE__ );
    }

err_1 :
    CEV_capsuleClear(&lCaps);

    return result;
}



/*----WAV from compiled file----*/

CEV_Chunk* CEV_waveFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetches wave in ressources _v2

    CEV_Chunk * result  = NULL;
    CEV_Capsule lCaps   = {0};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_WAV)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not wav.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToWav(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load wav.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*---MUSIC from compiled file---*/

CEV_Music* CEV_musicFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetches CEV_Music in ressources _v2

    CEV_Music*  result  = NULL;
    CEV_Capsule lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_MUSIC)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not music.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMusic(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load music.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----animations from compiled file----*/

SP_AnimList* CEV_animListFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetch sprite in ressources _v2

    SP_AnimList* result = NULL;
    CEV_Capsule lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_SPS)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not sprite.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToAnimSet(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load sprite.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


CEV_GifAnim* CEV_gifFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetch gif file in ressources _v2

    CEV_GifAnim* result = NULL;
    CEV_Capsule  lCaps  = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_GIF)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not gif.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToGif(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load gif.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----Scroll fetch----*/

CEV_ScrollText* CEV_scrollFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetches scroll text in compiled data file _v2

    CEV_ScrollText* result = NULL;
    CEV_Capsule     lCaps  = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not scroller.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToScroll(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*-----menu fetch-----*/

CEV_Menu* CEV_menuFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetch menu in compiled data file

    CEV_Menu* result = NULL;
    CEV_Capsule     lCaps  = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_SCROLL)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not scroller.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMenu(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load scroller.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*---- map fetch ----*/

CEV_TileMap* CEV_mapFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetches tile map from ressources _v2

    CEV_TileMap *result = NULL;
    CEV_Capsule lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_MAP)
    {
        fprintf(stderr,"Warn at %s / %d : id %u provided is not map.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToMap(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load map.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----background parallax fetch ----*/

CEV_Parallax* CEV_parallaxFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetch parallax background from ressources _v2

    CEV_Parallax    *result = NULL;
    CEV_Capsule     lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_PLX)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not parallax.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToParallax(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load parallax.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----weather fetch----*/

CEV_Weather* CEV_weatherFetch(int32_t id, CEV_RsrcFileHolder* src)
{//fetches weather from ressources

    CEV_Weather     *result = NULL;
    CEV_Capsule     lCaps   = {NULL};

    if(IS_NULL(src) || IS_NULL(src->fSrc))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    size_t offset = L_IdToOffset(id, src);

    if(!offset)
    {
        fprintf(stderr, "Err at %s / %d : id %u provided does not exist.\n", __FUNCTION__, __LINE__, id);
        return NULL;
    }

    fseek(src->fSrc, offset, SEEK_SET);

    CEV_capsuleTypeRead(src->fSrc, &lCaps);

    if (lCaps.type != IS_WTHR)
    {
        fprintf(stderr,"Err at %s / %d : id %u provided is not weather.\n", __FUNCTION__, __LINE__, id);
        goto end;
    }

    result = L_capsuleToWeather(&lCaps);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to load weather.\n", __FUNCTION__, __LINE__ );
    }

end:
    CEV_capsuleClear(&lCaps);

    return result;
}


/*----- Encapsulation -----*/


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
    caps->data = NULL;
    caps->size = 0;
    caps->type = 0;
}


void CEV_capsuleDestroy(CEV_Capsule* caps)
{//frees fileinfo and content

    free(caps->data);
    free(caps);
}


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


int CEV_fTypeFromName(char* fileName)
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
        fprintf(stderr, "Err at %s / %d : unable to create texte from data.\n", __FUNCTION__, __LINE__);

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
        fprintf(stderr, "Err at %s / %d : unable to create animation from data.\n", __FUNCTION__, __LINE__);

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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
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
         fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
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
         fprintf(stderr, "Err at %s / %d : unable to allocate result : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
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
         fprintf(stderr, "Err at %s / %d : unable to allocate result: %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err;
    }

    result->font        = font;
    result->virtualFile = caps->data;

    return result;

err:
    TTF_CloseFont(font);
    return NULL;
}


static SP_AnimList* L_capsuleToAnimSet(CEV_Capsule* caps)
{//converts to animSet

    if (IS_NULL(caps) || (!caps->size) || IS_NULL(caps->data))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromMem(caps->data, caps->size);

    if (IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    SP_AnimList *result = SP_animListLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create animSet.\n", __FUNCTION__, __LINE__);


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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_TileMap *result = CEV_mapLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create map.\n", __FUNCTION__, __LINE__);

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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Menu* result = CEV_menuLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create text scroll.\n", __FUNCTION__, __LINE__ );

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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_ScrollText* result = CEV_scrollLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create text scroll.\n", __FUNCTION__, __LINE__ );

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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Parallax *result = CEV_parallaxLoad_RW(vFile, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create parallax.\n", __FUNCTION__, __LINE__);

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
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Weather* result = CEV_weatherLoad_RW(vFile, 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create weather.\n", __FUNCTION__, __LINE__);

    return result;
}


static void L_capsuleDataTypeRead(FILE* src, CEV_Capsule* dst)
{//fills and allocate caps->data 1.0

    if(dst==NULL || src==NULL)
        readWriteErr++;

    dst->data = malloc(dst->size);

    if (dst->data == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        readWriteErr++;
    }
    else if (fread(dst->data, 1, dst->size, src) != dst->size)
    {
        fprintf(stderr, "Err at %s / %d : unable to read data in src : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        CEV_capsuleClear(dst);
        readWriteErr++;
    }
}


static void L_capsuleDataTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst)
{//fills and allocate dst->data 1.0

    if(IS_NULL(dst) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d :  NULL arg provided.\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    dst->data = malloc(dst->size);

    if (IS_NULL(dst->data))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        readWriteErr++;
    }
    else if (SDL_RWread(src, dst->data, 1, dst->size) != dst->size)
    {
        fprintf(stderr, "Err at %s / %d : unable to read data in src : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        CEV_capsuleClear(dst);
        readWriteErr++;
    }
}


static size_t L_IdToOffset(uint32_t id, CEV_RsrcFileHolder* src)
{//finds offset of an id

    if(IS_NULL(src) || IS_NULL(src->list))
        return 0;

    for(int i=0; i<src->numOfFiles; i++)
    {
        if(src->list[i].id == id)
            return src->list[i].offset;

    }

    return 0;
}


static size_t L_IndexToOffset(uint32_t index, CEV_RsrcFileHolder* src)
{//finds offset of an id

    if(IS_NULL(src) || IS_NULL(src->list))
        return 0;

    if(index < src->numOfFiles)
        return src->list[index].offset;

    return 0;
}


static void L_rsrcFileHolderHeaderRead(FILE* src, CEV_RsrcFileHolderHeader* dst)
{
    dst->id     = read_u32le(src);
    dst->offset = (size_t)read_u32le(src);

}
