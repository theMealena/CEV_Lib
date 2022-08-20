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
//**********************************************************/
//- CEV 2021 05 20- removed capsule data free from L_capsuleToXxx functions -> capsule cleared in calling functions.
//- CEV 2022 07 24- Added Capsule functions set


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CEV_gif.h"
#include "project_def.h"
#include "CEV_mixSystem.h"
#include "rwtypes.h"
#include "CEV_dataFile.h"
#include "CEV_animator.h"
#include "CEV_scroll.h"
#include "CEV_texts.h"
#include "CEV_selection.h"
#include "CEV_maps.h"
#include "CEV_parallax.h"
#include "CEV_weather.h"

/***** Local functions****/

/*ram mem into CEV_Text*/
static CEV_Text* L_capsuleToTxt(CEV_Capsule* caps);

/*ram mem into texture*/
static SDL_Texture* L_capsuleToPic(CEV_Capsule* caps);

/*ram mem into gif animation*/
static CEV_GifAnim* L_capsuleToGif(CEV_Capsule *caps);

/*ram mem into wav*/
static CEV_Chunk* L_capsuleToWav(CEV_Capsule *caps);

/*ram mem int music*/
static CEV_Music* L_capsuleToMusic(CEV_Capsule *caps);

/*ram mem into font*/
static CEV_Font* L_capsuleToFont(CEV_Capsule* caps);

/*ram mem into animations set*/
static SP_AnimList* L_capsuleToAnimSet(CEV_Capsule* caps);

/*ram to tile map*/
static CEV_TileMap* L_capsuleToMap(CEV_Capsule* caps);

/*ram to text scroller*/
static CEV_ScrollText* L_capsuleToScroll(CEV_Capsule* caps);

/*ram to parallax background*/
static CEV_Parallax* L_capsuleToParallax(CEV_Capsule* caps);

/*ram to weather*/
static CEV_Weather* L_capsuleToWeather(CEV_Capsule* caps);


/*file manipulation*/
static int L_capsuleReadIndex(unsigned int num, CEV_Capsule* caps, FILE* file);
static size_t L_gotoDataIndex(unsigned int index, FILE* file);
static void L_gotoAndDiscard(unsigned int index, FILE *file);
static void L_capsuleDataRead(FILE* src, CEV_Capsule* dst);
static void L_capsuleDataRead_RW(SDL_RWops* src, CEV_Capsule* dst);
static size_t L_fileSize(FILE* file);




/*----Type Any----*/


void* CEV_anyFetch(unsigned int index, FILE* src)
{/*returns finished load of any file type */


    /*---DECLARATIONS---*/

    CEV_Capsule lCaps       = {.type=0, .size=0, .data=NULL};//raw data informations
    uint32_t indexTabSize   = 0;//size of header

    void* returnVal = NULL; //result

    /*---PRL---*/

    if(src == NULL)
        return NULL;

    rewind(src);

    indexTabSize = read_u32le(src);//gets number of raw data in file

    if(index >= indexTabSize/sizeof(uint32_t))
    {//checking rawData index is available in file
        fprintf(stderr, "Err at %s / %d : Index provided is higher than available in file.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, src) != FUNC_OK)//loads raw data into lCaps
       goto exit;

    switch (lCaps.type)
    {
        case IS_DEFAULT:
        case IS_DAT:

            returnVal = malloc(sizeof(lCaps));

            if (returnVal == NULL)
            {
                fprintf(stderr, "Err at %s / %d : unable to allocate returnVal :%s\n", __FUNCTION__, __LINE__, strerror(errno));
                goto exit;
            }
            *((CEV_Capsule*)returnVal) = lCaps;

        break;

        case IS_DTX : //returns CEV_Text ptr
            returnVal = L_capsuleToTxt(&lCaps);
            CEV_capsuleClear(&lCaps);
        break;

        case IS_BMP :
        case IS_PNG :
        case IS_JPG :
            returnVal = L_capsuleToPic(&lCaps); //returns SDL_Texture ptr
            CEV_capsuleClear(&lCaps);
        break;

        case IS_GIF :
            returnVal = L_capsuleToGif(&lCaps); //returns SDL_GIFAnim ptr
            CEV_capsuleClear(&lCaps);
        break;

        case IS_WAV :
            returnVal = L_capsuleToWav(&lCaps); //returns CEV_chunk ptr
        break;

        case IS_FONT :
            returnVal = L_capsuleToFont(&lCaps); //returns CEV_Font ptr
        break;

        case IS_SPS :
            returnVal = L_capsuleToAnimSet(&lCaps); //return SP_AnimList ptr
            CEV_capsuleClear(&lCaps);
        break;

        case IS_MENU :

            CEV_capsuleClear(&lCaps);//unused to extract scroll
            L_gotoAndDiscard(index, src);
            returnVal = CEV_menuLoadf(src);//returns menu ptr
        break;

        case IS_SCROLL :

            CEV_capsuleClear(&lCaps);//unused to extract scroll
            L_gotoAndDiscard(index, src);
            returnVal = CEV_scrollTypeRead(src);//extraction reads file directly

        break;

        case IS_MAP :

            returnVal = L_capsuleToMap(&lCaps);
            CEV_capsuleClear(&lCaps);
        break;

        case IS_MUSIC :

            returnVal = L_capsuleToMusic(&lCaps);
        break;

        case IS_PLX :

            returnVal = L_capsuleToParallax(&lCaps);
            CEV_capsuleClear(&lCaps);
        break;

        case IS_WTHR :

            returnVal = L_capsuleToWeather(&lCaps);
            CEV_capsuleClear(&lCaps);

        default :
            CEV_capsuleClear(&lCaps);
            fprintf(stderr,"Err at %s / %d : unrecognized file format.\n", __FUNCTION__, __LINE__);//should not occur
        break;
    }

    /*---POST---*/

exit:
    return returnVal;
}


int CEV_capsuleFetch(unsigned int index, FILE* src, CEV_Capsule* dst)
{/*gets raw data in opened compiled file */

    /*---DECLARATIONS---*/

    int funcSts = FUNC_OK;

    /*---PRL---*/

    if((src == NULL) || (dst == NULL))
        return ARG_ERR;

    rewind(src);

    if(index >= read_u32le(src) / sizeof(uint32_t))
    {//checking if rawData index is available in file
        fprintf(stderr, "Err at %s / %d : Index provided is higher than available in file.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    /*---EXECUTION---*/

    funcSts = L_capsuleReadIndex(index, dst, src);

    /*---POST---*/

    return funcSts;
}


int CEV_capsuleLoad(CEV_Capsule* caps, const char* fileName)
{//fills Capsule from file

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

    caps->type = CEV_fileToType(fileName);
    caps->size = L_fileSize(file);
    rewind(file);

    L_capsuleDataRead(file, caps);//allocs & fills data field

    fclose(file);

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
}


void* CEV_capsuleExtract(CEV_Capsule* caps, bool freeData)
{/*extract result of capsule content*/

    void* returnVal = NULL;

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
        break;

        case IS_FONT :
            returnVal = L_capsuleToFont(caps); //returns CEV_Font ptr
        break;

        case IS_SPS :
            returnVal = L_capsuleToAnimSet(caps); //returns SP_AnimList ptr
        break;
/*
        case IS_MENU :
            L_gotoAndDiscard(index, file);
            returnVal = CEV_menuLoadf(file);//returns menu ptr
        break;
*/
        case IS_SCROLL :
            returnVal = L_capsuleToScroll(caps);//extraction reads file directly
        break;

        case IS_MAP :
            returnVal = L_capsuleToMap(caps);
        break;

        case IS_MUSIC :
            returnVal = L_capsuleToMusic(caps);
        break;

        case IS_PLX :
            returnVal = L_capsuleToParallax(caps);
        break;

        case IS_WTHR :
            returnVal = L_capsuleToWeather(caps);

        default :
            fprintf(stderr,"Err at %s / %d : unrecognized file format.\n", __FUNCTION__, __LINE__);//should not occur
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
{/*direct load from any file type T**/

    /*---DECLARATIONS---*/

    SDL_Surface *newSurface = NULL,
                *tempSurface = NULL;

    /*---PRL---*/

    if(fileName == NULL)
        return NULL;

    /*---EXECUTION---*/

    tempSurface = IMG_Load(fileName);

    if(tempSurface == NULL)
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());

    newSurface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_SetSurfaceBlendMode(newSurface, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(tempSurface);

    /*---POST---*/
    return newSurface;
}


SDL_Surface* CEV_surfaceFetch(unsigned int index, const char* fileName)
{/*fetch data index as SDL_Surface in compiled file T**/

    /*---DECLARATIONS---*/

    SDL_Surface *surface  = NULL,
                *temp     = NULL;
    CEV_Capsule  lCaps = {.type = 0, .size = 0, .data = NULL};
    FILE* file            = NULL;

    /*---PRELIMINAIRE---*/

    lCaps.data    = NULL;

    if(fileName == NULL)
    {
        fprintf(stderr,"Err at %s / %d : arg error received %d, %s\n", __FUNCTION__, __LINE__, index, fileName);
        goto end;
    }

    file = fopen(fileName,"rb");

    if (file == NULL)
    {//on error
        fprintf(stderr,"Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
       goto err_1;

    if (IS_PIC(lCaps.type))
        temp = IMG_Load_RW(SDL_RWFromMem(lCaps.data, lCaps.size), 1);

    else
        fprintf(stderr,"Err at %s / %d : Index provided is not a picture.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);


    SDL_FreeSurface(temp);

    CEV_capsuleClear(&lCaps);

err_1:
    fclose(file);

end:
    return(surface);
}



/*----SDL_Textures----*/

SDL_Texture* CEV_textureLoad(const char* fileName)
{/*direct load from file as texture T*/

        /*---DECLARATIONS---*/

    SDL_Texture     *texture = NULL;;
    SDL_Renderer    *render  = CEV_videoSystemGet()->render;

        /*---EXECUTION---*/

    texture = IMG_LoadTexture(render, fileName);

    if(texture != NULL)
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    else
        fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n",__FUNCTION__, __LINE__, fileName, IMG_GetError());

        /*---POST---*/

    return texture;
}


SDL_Texture* CEV_textureFetch(unsigned int index, const char* fileName)
{/*fetch data index as CEV_Texture in compiled file T*/

     /*---DECLARATIONS---*/

    SDL_Texture     *texture = NULL;
    CEV_Capsule    lCaps  = {.type = 0, .size = 0, .data = NULL};
    FILE            *file    = NULL;

    /*---PRELIMINAIRE---*/

    if(fileName == NULL)
    {
        fprintf(stderr, "Err at %s / %d : arg error received %d, %s\n", __FUNCTION__, __LINE__, index, fileName);
        goto end;
    }

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr,"Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
       goto err_1;

    if(IS_PIC(lCaps.type))
    {
        texture = L_capsuleToPic(&lCaps);

        if(texture != NULL)
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        else
            fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n", __FUNCTION__, __LINE__, fileName, IMG_GetError());
    }
    else
        fprintf(stderr,"Err at %s / %d : Index %d provided is not a picture.\n", __FUNCTION__, __LINE__, index);

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

err_1:
    fclose(file);

end:
    return(texture);
}


/*-----Texts from compiled file-----*/

CEV_Text* CEV_textFetch(unsigned int index, const char* fileName)
{/*fetch text in compiled data file and fills caps**/

    /*---DECLARATIONS---*/

    FILE *file          = NULL;
    CEV_Text* result    = NULL;

    CEV_Capsule lCaps;

    /*---PRL---**/

    if(fileName == NULL)
    {
        fprintf(stderr,"Err at %s / %d : arg error received %d, %s\n", __FUNCTION__, __LINE__, index, fileName);
        return NULL;
    }

    file = fopen(fileName,"rb");

    if (file == NULL)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto err_1;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
       goto err_2;

    if(lCaps.type == IS_DTX)
        result = L_capsuleToTxt(&lCaps);

    else
        fprintf(stderr,"Err at %s / %d : index provided is not text.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

err_2 :

    fclose(file);
err_1 :

    return result;
}


/*----TTF_Font from compiled file----*/

CEV_Font* CEV_fontFetch(unsigned int index, const char* fileName)
{//fetch font in compiled data file and fills lCaps

    CEV_Font* font       = NULL;
    FILE* file           = NULL;
    CEV_Capsule lCaps = {.type = 0, .size = 0, .data = NULL};

    if (fileName == NULL)
        return NULL;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
        goto err_1;

    if(lCaps.type == IS_FONT)
        font = L_capsuleToFont(&lCaps);
    else
        fprintf(stderr, "Err at %s / %d : index provided is not font.\n", __FUNCTION__, __LINE__);

    CEV_capsuleClear(&lCaps);

err_1 :
    fclose(file);

end :
    return font;
}



/*----WAV from compiled file----*/

CEV_Chunk* CEV_waveFetch(unsigned int index, const char* fileName)
{/*fetch Mix_chunk in compiled data file and fills lCaps**/

    /*---DECLARATIONS---*/

    CEV_Chunk       *result = NULL;
    FILE            *file   = NULL;
    CEV_Capsule    lCaps = {.type = 0, .size = 0, .data = NULL};

        /*---PRL---*/

    if (fileName == NULL)
        return NULL;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
        goto err_1;

    if (lCaps.type == IS_WAV)
        result = L_capsuleToWav(&lCaps);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not wav.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

err_1 :
    fclose(file);

end :
    return result;
}


/*---MUSIC from compiled file---*/

CEV_Music* CEV_musicFetch(unsigned int index, const char* fileName)
{/*fetch data index as Mix_Music in compiled file T*/

     /*---DECLARATIONS---*/

    CEV_Music   *music  = NULL;
    CEV_Capsule lCaps   = {.type = 0, .size = 0, .data = NULL};
    FILE        *file   = NULL;

    /*---PRELIMINAIRE---*/

    if(fileName == NULL)
    {
        fprintf(stderr, "Err at %s / %d : arg error received %d, %s\n", __FUNCTION__, __LINE__, index, fileName);
        goto end;
    }

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr,"Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
       goto err_1;

    if(lCaps.type == IS_MUSIC)
    {
        music = L_capsuleToMusic(&lCaps);

        if(IS_NULL(music))
            fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n", __FUNCTION__, __LINE__, fileName, Mix_GetError());
    }
    else
        fprintf(stderr,"Err at %s / %d : Index provided is not a music.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    CEV_soundSystemGet()->loadedMusic = music;

    CEV_capsuleClear(&lCaps);

err_1:
    fclose(file);

end:
    return(music);
}


/*----animations from compiled file----*/

SP_AnimList* CEV_animListFetch(unsigned int index, char* fileName)
{/*fetch SP_AnimList in compiled data file and fills caps**/

    /*---DECLARATIONS---*/

    FILE            *file       = NULL;
    SP_AnimList      *animSet    = NULL;
    CEV_Capsule    lCaps     = {.type = 0, .size = 0, .data = NULL};

        /*---PRL---*/

    if (fileName == NULL)
        return NULL;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
        goto err_1;

    if (lCaps.type == IS_SPS)
        animSet = L_capsuleToAnimSet(&lCaps);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not an animation set.\n", __FUNCTION__, __LINE__);

    /*---POST---*/
    CEV_capsuleClear(&lCaps);

err_1 :
    fclose(file);

end :
    return animSet;
}


CEV_GifAnim* CEV_gifFetch(unsigned int index, char* fileName)
{/*fetch gif file in compiled data file**/

    /*---DECLARATIONS---*/

    FILE            *file       = NULL;
    CEV_GifAnim     *animSet    = NULL;
    CEV_Capsule    lCaps     = {.type = 0, .size = 0, .data = NULL};

        /*---PRL---*/

    if ( (fileName == NULL) )
        return NULL;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
        goto end;
    }

    /*---EXECUTION---*/

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
        goto err_1;

    if (lCaps.type == IS_GIF)
        animSet = L_capsuleToGif(&lCaps);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not gif.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

err_1 :
    fclose(file);

end :
    return animSet;
}


/*----Scroll fetch----*/

CEV_ScrollText* CEV_scrollFetch(unsigned int index, char* fileName)
{//fetches scroll text in compiled data file

    FILE            *file   = NULL;
    CEV_ScrollText  *result = NULL;
    CEV_Capsule    lCaps = {.data = NULL};

    if(fileName == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : fileName arg is NULL.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    //openning file
    file = fopen(fileName, "rb");

    if(file == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto end;
    }

    if (L_capsuleReadIndex(index, &lCaps, file) != FUNC_OK)
        goto end;

    if(lCaps.type != IS_SCROLL)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not scroll.\n", __FUNCTION__, __LINE__);
        goto end;
    }

    result = L_capsuleToScroll(&lCaps);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
    }

end:

    CEV_capsuleClear(&lCaps);
    fclose(file);

    return result;
}


/*-----menu fetch-----*/

CEV_Menu* CEV_menuFetch(int index, char* fileName)
{/*fetch menu in compiled data file**/

    /*---DECLARATIONS---*/

    FILE *file     = NULL;
    CEV_Menu *menu = NULL;
    int type;

    if (fileName == NULL)
        return NULL;

    file = fopen(fileName, "rb");

    if (file == NULL)
    {/*gestion d'erreur*/
       fprintf(stderr, "Err at %s / %d : unable to open file %s\n", __FUNCTION__, __LINE__, fileName);
       goto end;
    }

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);

    type    = read_u32le(file);//read file type
    read_u32le(file);//skip file size

    if(type == IS_MENU)
        menu = CEV_menuLoadf(file);
    else
        fprintf(stderr, "Err at %s / %d : index provided is not menu.\n", __FUNCTION__, __LINE__);

    fclose(file);

end :
    return menu;
}


/*---- map fetch ----*/

CEV_TileMap* CEV_mapFetch(int index, char* fileName)
{/*fetch tile map in compressed data file*/

    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_TileMap     *result = NULL;
    CEV_Capsule    lCaps = {.data = NULL};

    /*---PRL---*/

    if(fileName == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : fileName arg is NULL.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /**openning file*/
    file = fopen(fileName, "rb");

    if(file == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto exit;
    }

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);

    lCaps.type = read_u32le(file);//just to check
    lCaps.size = read_u32le(file);//useless but has to be read

    if(lCaps.type != IS_MAP)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not map.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_capsuleDataRead(file, &lCaps);

    if(IS_NULL(lCaps.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }


    result = L_capsuleToMap(&lCaps);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create map.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

exit :

    if(file)
        fclose(file);

    return result;
}


/*----background parallax fetch ----*/

CEV_Parallax* CEV_parallaxFetch(int index, char* fileName)
{/*fetch parallax background in compressed data file*/
    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_Parallax    *result = NULL;
    CEV_Capsule    lCaps = {.data = NULL};

    /*---PRL---*/

    if(fileName == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : fileName arg is NULL.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /**openning file*/
    file = fopen(fileName, "rb");

    if(file == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto exit;
    }

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);

    lCaps.type = read_u32le(file);//just to check
    lCaps.size = read_u32le(file);//useless but has to be read

    if(lCaps.type != IS_PLX)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not parallax background.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_capsuleDataRead(file, &lCaps);

    if(IS_NULL(lCaps.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }

    result = L_capsuleToParallax(&lCaps);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create parallax background.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

    CEV_capsuleClear(&lCaps);

exit :

    if(file)
        fclose(file);

    return result;
}


/*----weather fetch----*/

CEV_Weather* CEV_weatherFetch(int index, char* fileName)
{/*fetch weather in compressed data file*/
    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_Weather     *result = NULL;
    CEV_Capsule    lCaps = {.data = NULL};

    /*---PRL---*/

    if(fileName == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : fileName arg is NULL.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /**openning file*/
    file = fopen(fileName, "rb");

    if(file == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto exit;
    }

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);

    lCaps.type = read_u32le(file);//just to check
    lCaps.size = read_u32le(file);//useless but has to be read

    if(lCaps.type != IS_WTHR)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not weather file.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_capsuleDataRead(file, &lCaps);

    if(IS_NULL(lCaps.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }

    result = L_capsuleToWeather(&lCaps);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create weather.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/
    CEV_capsuleClear(&lCaps);

exit :

    if(file)
        fclose(file);

    return result;
}


/*----- Encapsulation -----*/


void CEV_capsuleWrite(CEV_Capsule* caps, FILE* dst)
{/*writes capsule into file*/

    write_u32le(caps->type, dst);
    write_u32le(caps->size, dst);

    if(fwrite(caps->data, sizeof(char), caps->size, dst) != caps->size)
        readWriteErr++;
}


void CEV_capsuleRead(FILE* src, CEV_Capsule* dst)
{/*reads capsule from file*/

    dst->type   = read_u32le(src);
    dst->size   = read_u32le(src);

    if(!readWriteErr)
        L_capsuleDataRead(src, dst);//allocs & fills data field
}


void CEV_capsuleWrite_RW(CEV_Capsule* src, SDL_RWops* dst)
{/*writes capsule into RWops*/

    readWriteErr += SDL_WriteLE32(dst, src->type);
    readWriteErr += SDL_WriteLE32(dst, src->size);

    if(SDL_RWwrite(dst, src->data, 1, src->size) != src->size)
        readWriteErr++;
}


void CEV_capsuleRead_RW(SDL_RWops* src, CEV_Capsule* dst)
{/*reads capsule from RWops*/

    dst->type   = SDL_ReadLE32(src);
    dst->size   = SDL_ReadLE32(src);

    L_capsuleDataRead_RW(src, dst);
}


void CEV_capsuleClear(CEV_Capsule* caps)
{/*clears fileinfo content*/

    if(IS_NULL(caps))
        return;

    free(caps->data);
    caps->data = NULL;
    caps->size = 0;
    caps->type = 0;
}


void CEV_capsuleDestroy(CEV_Capsule* caps)
{/*frees fileinfo and content*/

    free(caps->data);
    free(caps);
}


int CEV_fileToType(char* fileName)
{/*turns file extention into file type enum*/

    /*---DECLARATIONS---*/
    char *extList[FILE_TYPE_NUM] = FILE_TYPE_LIST;
    int i;
    size_t length=0;
    char *point = fileName;

    /*---EXECUTION---*/
    length = strlen(fileName);

    for (i = length; i>=0 && *point!='.'; i--)
        point = fileName+i;

    point++;

    for (i = 0; i < FILE_TYPE_NUM; i++)
    {
        if (!strcmp(point, extList[i]))
            return i;
    }

    /*---POST---*/

    return(IS_DEFAULT);
}

//attributes extension
char* CEV_fileTypeToExt(int type)
{
    char *extList[FILE_TYPE_NUM] = FILE_TYPE_LIST;

    if(type>0 && type<FILE_TYPE_NUM)
        return extList[type];

    return extList[0];
}

/*---Local Sub-functions not to be directly called---*/


static CEV_Text* L_capsuleToTxt(CEV_Capsule* caps)
{/**create table of pointers to lines*/

    CEV_Text* result = NULL;

    if (caps==NULL || caps->size<=0 || caps->data==NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops* vFile = SDL_RWFromConstMem(caps->data, caps->size);

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d :   .\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    result = CEV_textLoad_RW(vFile, 1);

    return(result);
}


static SDL_Texture *L_capsuleToPic(CEV_Capsule* caps)
{/*converts raw mem to SDL_Texture*/

    /*---DECLARATIONS---*/
    SDL_Renderer    *render     = CEV_videoSystemGet()->render;
    SDL_Texture     *pic        = NULL;

    /*---PRL---*/

    if ((caps == NULL) || (caps->size<=0))
        return NULL;

    /*---EXECUTION---*/

    pic = IMG_LoadTexture_RW(render, SDL_RWFromMem(caps->data, caps->size), 1);//closes rwops

    if (pic == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());
        return NULL;
    }

    /*---POST---*/

    return pic;
}


static CEV_GifAnim *L_capsuleToGif(CEV_Capsule* caps)
{/*converts to a gif animation*/

    /*---DECLARATIONS---*/

    CEV_GifAnim* animation  = NULL;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    /*---PRL---*/

    if ((caps == NULL) || (render == NULL))
        return NULL;

    /*---EXECUTION---*/

    animation = CEV_gifAnimLoadRW(SDL_RWFromMem(caps->data, caps->size), render, 1); //closes rwops

    if(animation == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create animation from data.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    //free(caps->data);
    //caps->data = NULL;

    return animation;
}


static CEV_Chunk* L_capsuleToWav(CEV_Capsule* caps)
{/*convert to a wav sample*/

    /*---DECLARATIONS---*/

    CEV_Chunk   *result  = NULL;
    Mix_Chunk   *chunk   = NULL;
    SDL_RWops   *memFile = SDL_RWFromMem(caps->data, caps->size);

    /*---PRL---*/

    if ((caps==NULL) || (caps->size<=0))//bad arg
        return NULL;

    /*---EXECUTION---*/

    if (memFile == NULL)//MIX does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_1;
    }

    chunk = Mix_LoadWAV_RW(memFile, 0); //ask to close rwops but does not free datas

    if (chunk == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        goto err_2;
    }

    result = malloc(sizeof(*result));

    if (result == NULL)
    {
         fprintf(stderr, "Err at %s / %d : unable to allocate result : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err_3;
    }

    /*---POST---*/

    result->sound       = chunk;
    result->virtualFile = caps->data;

    SDL_RWclose(memFile);

    return result;

err_3:
    Mix_FreeChunk(chunk);

err_2:
    SDL_RWclose(memFile);

err_1:
    //free(caps->data);
    //caps->data = NULL;

    return NULL;
}


static CEV_Music* L_capsuleToMusic(CEV_Capsule* caps)
{/*convert to a wav sample*/

    /*---DECLARATIONS---*/

    CEV_Music   *result  = NULL;
    Mix_Music   *music   = NULL;
    SDL_RWops   *memFile = NULL;

    /*---PRL---*/

    if ((caps==NULL) || (caps->size<=0))//bad arg
        return NULL;

    /*---EXECUTION---*/

    memFile = SDL_RWFromMem(caps->data, caps->size);

    if (memFile == NULL)//MIX does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_1;
    }

    music = Mix_LoadMUS_RW(memFile, 1); //ask to close rwops

    if (music == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        goto err_2;
    }

    result = malloc(sizeof(*result));

    if (result == NULL)
    {
         fprintf(stderr, "Err at %s / %d : unable to allocate result : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err_3;
    }

    /*---POST---*/

    result->music       = music;
    result->virtualFile = caps->data;

    //SDL_RWclose(memFile);

    return result;

err_3:
    Mix_FreeMusic(music);

err_2:
    SDL_RWclose(memFile);

err_1:
    //free(caps->data);
    //caps->data = NULL;

    return NULL;
}


static CEV_Font* L_capsuleToFont(CEV_Capsule* caps)
{/*converts to CEV_Font*/

    /*---DECLARATIONS---*/

    CEV_Font* result    = NULL;//overlay to keep track of caps->data until TTF_CloseFont
    TTF_Font* font      = NULL;
    SDL_RWops* memFile  = NULL;

    /*---PRL---*/

    if ((caps==NULL) || (caps->data == NULL) || (caps->size<=0))
        goto err_exit;

    /*---EXECUTION---*/

    memFile  = SDL_RWFromMem(caps->data, caps->size);

    if (memFile == NULL)//TTF does not check SDL_RWops* != NULL better do it myself*/
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_1;
    }

    font = TTF_OpenFontRW(memFile, 1, FONT_MAX);//asks to close rwops but doesn't free data

    if (font == NULL)
    {
         fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
         goto err_2;
    }

    result = malloc(sizeof(*result));

    if (result == NULL)
    {
         fprintf(stderr, "Err at %s / %d : unable to allocate result: %s.\n", __FUNCTION__, __LINE__, strerror(errno));
         goto err_3;
    }

    /*---POST---*/

    result->font        = font;
    result->virtualFile = caps->data;

    return result;

err_3:
    TTF_CloseFont(font);

err_2:
    SDL_RWclose(memFile);

err_1:
    //free(caps->data);
    //caps->data = NULL;

err_exit :
    return NULL;
}


static SP_AnimList* L_capsuleToAnimSet(CEV_Capsule* caps)
{/*converts to animSet */

    /*---DECLARATIONS---*/

    SP_AnimList *animSet = NULL;

    /*---EXECUTION---*/

    animSet = SP_animListLoad_RW(SDL_RWFromConstMem(caps->data, caps->size), 1);

    if (animSet == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create animSet.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    //free(caps->data);
    //caps->data = NULL;

    return animSet;
}


static CEV_TileMap* L_capsuleToMap(CEV_Capsule* caps)
{/*convert to tile map*/

    /*---DECLARATIONS---*/

    CEV_TileMap *map = NULL;

    /*---EXECUTION---*/

    map = CEV_mapLoad_RW(SDL_RWFromConstMem(caps->data, caps->size), 1);

    if (map == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create map.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    //free(caps->data);
    //caps->data = NULL;

    return map;
}


static CEV_ScrollText* L_capsuleToScroll(CEV_Capsule* caps)
{
    CEV_ScrollText* result = CEV_scrollLoad_RW(SDL_RWFromConstMem(caps->data, caps->size), 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create text scroll.\n", __FUNCTION__, __LINE__ );

    return result;
}


static CEV_Parallax* L_capsuleToParallax(CEV_Capsule* caps)
{//convert to parallax

    CEV_Parallax *bckgd = NULL;

    bckgd = CEV_parallaxLoad_RW(SDL_RWFromConstMem(caps->data, caps->size), 1);

    if (bckgd == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create parallax.\n", __FUNCTION__, __LINE__);

    return  bckgd;
}


static CEV_Weather* L_capsuleToWeather(CEV_Capsule* caps)
{//convert to weather

    CEV_Weather *result = NULL;

    result = CEV_weatherLoad_RW(SDL_RWFromConstMem(caps->data, caps->size), 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create weather.\n", __FUNCTION__, __LINE__);

    return  result;
}


static void L_gotoAndDiscard(unsigned int index, FILE* file)
{/*reach raw data discarding header*/

    L_gotoDataIndex(index, file);

    read_u32le(file);//discards type and size
    read_u32le(file);
}


static int L_capsuleReadIndex(unsigned int index, CEV_Capsule* caps, FILE* file)
{/*reaches index and fills caps**/

    /*---PRL---*/
    if (caps==NULL || file==NULL)
        return ARG_ERR;

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);   //goto data offset
    CEV_capsuleRead(file, caps);    //fills caps

    if (readWriteErr)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to read header informations.\n", __FUNCTION__, __LINE__);
        return (FUNC_ERR);
    }

    /*---POST---*/

    return FUNC_OK;
}


static size_t L_gotoDataIndex(unsigned int index, FILE* file)
{//goes to index in file

    fseek(file, index * sizeof(uint32_t), SEEK_SET);

    size_t offset = read_u32le(file);

    fseek(file, offset, SEEK_SET);

    return offset;
}


static void L_capsuleDataRead(FILE* src, CEV_Capsule* dst)
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


static void L_capsuleDataRead_RW(SDL_RWops* src, CEV_Capsule* dst)
{//fills and allocate dst->data 1.0

    if(dst==NULL || src==NULL)
        readWriteErr++;

    dst->data = malloc(dst->size);

    if (dst->data == NULL)
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


static size_t L_fileSize(FILE* file)
{/*size of a file*/

    //recording pos
    long pos = ftell(file),
         size = 0;

    //getting file size
    fseek(file, 0, SEEK_END);
    size = ftell(file);

    //back to recorded pos
    fseek(file, pos, SEEK_SET);

    return(size);
}



