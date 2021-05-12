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
#include "../parallax.h"
#include "CEV_weather.h"

/***** Local functions****/

/*ram mem into CEV_Text*/
static CEV_Text* L_rawToTxt(CEV_FileInfo* info);

/*ram mem into texture*/
static SDL_Texture* L_rawToPic(CEV_FileInfo* info);

/*ram mem into gif animation*/
static CEV_GifAnim* L_rawToGif(CEV_FileInfo *info);

/*ram mem into wav*/
static CEV_Chunk* L_rawToWav(CEV_FileInfo *info);

/*ram mem int music*/
static CEV_Music* L_rawToMusic(CEV_FileInfo *buffer);

/*ram mem into font*/
static CEV_Font* L_rawToFont(CEV_FileInfo* info);

/*ram mem into animations set*/
static SP_AnimList* L_rawToAnimSet(CEV_FileInfo* info);

/*ram to tile map*/
static CEV_TileMap* L_rawToMap(CEV_FileInfo* buffer);

/*ram to parallax background*/
static CEV_Parallax* L_rawToParallax(CEV_FileInfo* buffer);

/*ram to weather*/
static CEV_Weather* L_rawToWeather(CEV_FileInfo* buffer);


/*file manipulation*/
static int L_fileInfoReadIndex(unsigned int num, CEV_FileInfo* buffer, FILE* file);
static size_t L_gotoDataIndex(unsigned int index, FILE* file);
static void L_gotoAndDiscard(unsigned int index, FILE *file);
static void L_infoReadRaw(CEV_FileInfo* buffer, FILE* file);
static size_t L_fileSize(FILE* file);




/*----Type Any----*/


void* CEV_anyFetch(unsigned int index, FILE* file)
{/*returns finished load of any file type**/


    /*---DECLARATIONS---*/

    CEV_FileInfo lBuffer    = {.type=0, .size=0, .data=NULL};//raw data informations
    uint32_t indexTabSize   = 0;//size of header

    void* returnVal = NULL; //result


    /*---PRL---*/

    if(index<0 || file == NULL)
        return NULL;

    rewind(file);

    indexTabSize = read_u32le(file);//gets number of raw data in file

    if(index >= indexTabSize/sizeof(uint32_t))
    {//checking rawData index is available in file
        fprintf(stderr, "Err at %s / %d : Index provided is higher than available in file.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)//loads raw data into lBuffer
       goto exit;

    switch (lBuffer.type)
    {
        case IS_DEFAULT:
        case IS_DAT:

            returnVal = malloc(sizeof(lBuffer));

            if (returnVal == NULL)
            {
                fprintf(stderr, "Err at %s / %d : unable to allocate returnVal :%s\n", __FUNCTION__, __LINE__, strerror(errno));
                goto exit;
            }
            *((CEV_FileInfo*)returnVal) = lBuffer;

        break;

        case IS_TXT : //returns CEV_Text ptr
            returnVal = L_rawToTxt(&lBuffer);
        break;

        case IS_BMP :
        case IS_PNG :
        case IS_JPG :
            returnVal = L_rawToPic(&lBuffer); //returns SDL_Texture ptr
        break;

        case IS_GIF :
            returnVal = L_rawToGif(&lBuffer); //returns SDL_GIFAnim ptr
        break;

        case IS_WAV :
            returnVal = L_rawToWav(&lBuffer); //returns CEV_chunk ptr
        break;

        case IS_FONT :
            returnVal = L_rawToFont(&lBuffer); //returns CEV_Font ptr
        break;

        case IS_SPS :
            returnVal = L_rawToAnimSet(&lBuffer); //return SP_AnimList ptr
        break;

        case IS_MENU :

            CEV_fileInfoClear(&lBuffer);//unused to extract scroll
            L_gotoAndDiscard(index, file);
            returnVal = CEV_menuLoadf(file);//returns menu ptr
        break;

        case IS_SCROLL :

            CEV_fileInfoClear(&lBuffer);//unused to extract scroll
            L_gotoAndDiscard(index, file);
            returnVal = CEV_scrollLoadf(file);//extraction reads file directly

        break;

        case IS_MAP :

            returnVal = L_rawToMap(&lBuffer);
        break;

        case IS_MUSIC :

            returnVal = L_rawToMusic(&lBuffer);
        break;

        case IS_PRALX :

            returnVal = L_rawToParallax(&lBuffer);
        break;

        case IS_WTHR :

            returnVal = L_rawToWeather(&lBuffer);

        default :
            CEV_fileInfoClear(&lBuffer);
            fprintf(stderr,"Err at %s / %d : unrecognized file format.\n", __FUNCTION__, __LINE__);//should not occur
        break;
    }

    /*---POST---*/

exit:

    return returnVal;
}


int CEV_rawFetch(unsigned int index, CEV_FileInfo* infos, FILE* file)
{/*gets raw data in opened compiled file */

    /*---DECLARATIONS---*/

    int funcSts = FUNC_OK;

    /*---PRL---*/

    if((file == NULL) || (infos == NULL))
        return ARG_ERR;

    rewind(file);

    if(index >= read_u32le(file) / sizeof(uint32_t))
    {//checking if rawData index is available in file
        fprintf(stderr, "Err at %s / %d : Index provided is higher than available in file.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    /*---EXECUTION---*/

    funcSts = L_fileInfoReadIndex(index, infos, file);

    /*---POST---*/

    return funcSts;
}


int CEV_rawLoad(CEV_FileInfo* infos, char* fileName)
{/*gets raw from natural file*/

    /*--DECLARATIONS--*/

    FILE* file  = NULL;

    /*--PRELIMINAIRES--*/

    readWriteErr = 0;

    if(infos == NULL || fileName == NULL)
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    file = fopen(fileName, "rb");

    if (file == NULL)
    {
        printf("Err at %s / %d : unable to open file %s : %s\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return FUNC_ERR;
    }

    infos->type = CEV_fileToType(fileName);
    infos->size = L_fileSize(file);
    rewind(file);

    L_infoReadRaw(infos, file);

    /*--POST--*/

    fclose(file);

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
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
    CEV_FileInfo  lBuffer = {.type = 0, .size = 0, .data = NULL};
    FILE* file            = NULL;

    /*---PRELIMINAIRE---*/

    lBuffer.data    = NULL;

    if(index<0 || fileName == NULL)
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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
       goto err_1;

    if (IS_PIC(lBuffer.type))
        temp = IMG_Load_RW(SDL_RWFromMem(lBuffer.data, lBuffer.size), 1);

    else
        fprintf(stderr,"Err at %s / %d : Index provided is not a picture.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    surface = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);


    SDL_FreeSurface(temp);

    CEV_fileInfoClear(&lBuffer);

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
    CEV_FileInfo    lBuffer  = {.type = 0, .size = 0, .data = NULL};
    FILE            *file    = NULL;

    /*---PRELIMINAIRE---*/

    if(index<0 || fileName == NULL)
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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
       goto err_1;

    if(IS_PIC(lBuffer.type))
    {
        texture = L_rawToPic(&lBuffer);//frees data field

        if(texture != NULL)
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        else
            fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n", __FUNCTION__, __LINE__, fileName, IMG_GetError());
    }
    else
        fprintf(stderr,"Err at %s / %d : Index %d provided is not a picture.\n", __FUNCTION__, __LINE__, index);

    /*---POST---*/

err_1:
    fclose(file);

end:
    return(texture);
}


/*-----Texts from compiled file-----*/

CEV_Text* CEV_textFetch(unsigned int index, const char* fileName)
{/*fetch text in compiled data file and fills buffer**/

    /*---DECLARATIONS---*/

    FILE *file          = NULL;
    CEV_Text* result    = NULL;

    CEV_FileInfo lbuffer;

    /*---PRL---**/

    if(index<0 || fileName == NULL)
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

    if (L_fileInfoReadIndex(index, &lbuffer, file) != FUNC_OK)
       goto err_2;

    if(lbuffer.type == IS_TXT)
        result = L_rawToTxt(&lbuffer);

    else
        fprintf(stderr,"Err at %s / %d : index provided is not text.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

err_2 :

    fclose(file);
err_1 :

    return result;
}


/*----TTF_Font from compiled file----*/

CEV_Font* CEV_fontFetch(unsigned int index, const char* fileName)
{/*fetch font in compiled data file and fills lBuffer*/

    /*---DECLARATIONS---*/

    CEV_Font* font       = NULL;
    FILE* file           = NULL;
    CEV_FileInfo lBuffer = {.type = 0, .size = 0, .data = NULL};

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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
        goto err_1;

    if(lBuffer.type == IS_FONT)
        font = L_rawToFont(&lBuffer);
    else
        fprintf(stderr, "Err at %s / %d : index provided is not font.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

err_1 :
    fclose(file);

end :
    return font;
}



/*----WAV from compiled file----*/

CEV_Chunk* CEV_waveFetch(unsigned int index, const char* fileName)
{/*fetch Mix_chunk in compiled data file and fills lBuffer**/

    /*---DECLARATIONS---*/

    CEV_Chunk       *result = NULL;
    FILE            *file   = NULL;
    CEV_FileInfo    lBuffer = {.type = 0, .size = 0, .data = NULL};

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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
        goto err_1;

    if (lBuffer.type == IS_WAV)
        result = L_rawToWav(&lBuffer);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not wav.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

err_1 :
    fclose(file);

end :
    return result;
}


/*---MUSIC from compiled file---*/

CEV_Music *CEV_musicFetch(unsigned int index, const char* fileName)
{/*fetch data index as Mix_Music in compiled file T*/

     /*---DECLARATIONS---*/

    CEV_Music       *music  = NULL;
    CEV_FileInfo    lBuffer = {.type = 0, .size = 0, .data = NULL};
    FILE            *file   = NULL;

    /*---PRELIMINAIRE---*/

    if(index<0 || fileName == NULL)
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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
       goto err_1;

    if(lBuffer.type == IS_MUSIC)
    {
        music = L_rawToMusic(&lBuffer);//frees data field

        if(IS_NULL(music))
            fprintf(stderr, "Err at %s / %d : unable to load %s, %s\n", __FUNCTION__, __LINE__, fileName, Mix_GetError());
    }
    else
        fprintf(stderr,"Err at %s / %d : Index provided is not a music.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    CEV_soundSystemGet()->loadedMusic = music;

err_1:
    fclose(file);

end:
    return(music);
}


/*----animations from compiled file----*/

SP_AnimList* CEV_animListFetch(unsigned int index, char* fileName)
{/*fetch SP_AnimList in compiled data file and fills buffer**/

    /*---DECLARATIONS---*/

    FILE            *file       = NULL;
    SP_AnimList      *animSet    = NULL;
    CEV_FileInfo    lBuffer     = {.type = 0, .size = 0, .data = NULL};

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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
        goto err_1;

    if (lBuffer.type == IS_SPS)
        animSet = L_rawToAnimSet(&lBuffer);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not an animation set.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

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
    CEV_FileInfo    lBuffer     = {.type = 0, .size = 0, .data = NULL};

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

    if (L_fileInfoReadIndex(index, &lBuffer, file) != FUNC_OK)
        goto err_1;

    if (lBuffer.type == IS_GIF)
        animSet = L_rawToGif(&lBuffer);
    else
        fprintf(stderr,"Err at %s / %d : index provided is not gif.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

err_1 :
    fclose(file);

end :
    return animSet;
}


/*----Scroll fetch----*/

CEV_ScrollText *CEV_scrollFetch(unsigned int index, char* fileName)
{/*fetch scroll text in compiled data file**/

    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_ScrollText  *result = NULL;
    CEV_FileInfo    lBuffer = {.data = NULL};

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

    lBuffer.type = read_u32le(file);//just to check
    lBuffer.size = read_u32le(file);//useless but has to be read

    if(lBuffer.type != IS_SCROLL)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    result = CEV_scrollLoadf(file);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

exit :

    if(file)
        fclose(file);

    return result;
}


/*-----menu fetch-----*/

CEV_Menu *CEV_menuFetch(int index, char* fileName)
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

CEV_TileMap *CEV_mapFetch(int index, char* fileName)
{/*fetch tile map in compressed data file*/

    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_TileMap     *result = NULL;
    CEV_FileInfo    lBuffer = {.data = NULL};

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

    lBuffer.type = read_u32le(file);//just to check
    lBuffer.size = read_u32le(file);//useless but has to be read

    if(lBuffer.type != IS_MAP)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not map.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_infoReadRaw(&lBuffer, file);

    if(IS_NULL(lBuffer.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }


    result = L_rawToMap(&lBuffer);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create map.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

exit :

    if(file)
        fclose(file);

    return result;
}


/*----background parallax fetch ----*/

CEV_Parallax *CEV_parallaxFetch(int index, char* fileName)
{/*fetch parallax background in compressed data file*/
    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_Parallax    *result = NULL;
    CEV_FileInfo    lBuffer = {.data = NULL};

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

    lBuffer.type = read_u32le(file);//just to check
    lBuffer.size = read_u32le(file);//useless but has to be read

    if(lBuffer.type != IS_PRALX)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not parallax background.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_infoReadRaw(&lBuffer, file);

    if(IS_NULL(lBuffer.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }


    result = L_rawToParallax(&lBuffer);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create parallax background.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

exit :

    if(file)
        fclose(file);

    return result;
}


CEV_Weather *CEV_weatherFetch(int index, char* fileName)
{/*fetch weather in compressed data file*/
    /*---DECLARATIONS---*/

    FILE            *file   = NULL;
    CEV_Weather     *result = NULL;
    CEV_FileInfo    lBuffer = {.data = NULL};

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

    lBuffer.type = read_u32le(file);//just to check
    lBuffer.size = read_u32le(file);//useless but has to be read

    if(lBuffer.type != IS_WTHR)
    {
        fprintf(stderr, "Err at %s / %d : index provided is not weather file.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else
        L_infoReadRaw(&lBuffer, file);

    if(IS_NULL(lBuffer.data))
    {
        fprintf(stderr, "Err at %s / %d : cannot read data %d.\n", __FUNCTION__, __LINE__, index);
        goto exit;
    }

    result = L_rawToWeather(&lBuffer);

    if(!result)
    {
        fprintf(stderr, "Err at %s / %d : unable to create weather.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

exit :

    if(file)
        fclose(file);

    return result;
}



void CEV_fileInfoTypeWrite(CEV_FileInfo *buffer, FILE* dst)
{/*writes file info into file*/
    write_u32le(buffer->type, dst);
    write_u32le(buffer->size, dst);

    if(fwrite(buffer->data, sizeof(char), buffer->size, dst) != buffer->size)
        readWriteErr++;
}


void CEV_fileInfoTypeRead(FILE* src, CEV_FileInfo *buffer)
{/*reads file info into file*/
    buffer->type       = read_u32le(src);
    buffer->size       = read_u32le(src);

    L_infoReadRaw(buffer, src);
}


void CEV_fileInfoClear(CEV_FileInfo *buffer)
{/*clears fileinfo content*/

    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->type = 0;
}


void CEV_fileInfoFree(CEV_FileInfo *buffer)
{/*frees fileinfo and content*/
    free(buffer->data);
    free(buffer);
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



/*---Local Sub-functions not to be directly called---*/


static CEV_Text* L_rawToTxt(CEV_FileInfo* buffer)
{/**create table of pointers to lines*/

    /*---DECLARATIONS---*/

    CEV_Text    *result = NULL;

    /*---PRL---*/

    if (buffer==NULL || buffer->size<=0 || buffer->data==NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    result = CEV_textLoad_RW(SDL_RWFromConstMem(buffer->data, buffer->size), 1);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return(result);
}


static SDL_Texture *L_rawToPic(CEV_FileInfo* buffer)
{/*converts raw mem to SDL_Texture*/

    /*---DECLARATIONS---*/
    SDL_Renderer    *render     = CEV_videoSystemGet()->render;
    SDL_Texture     *pic        = NULL;

    /*---PRL---*/

    if ((buffer == NULL) || (buffer->size<=0))
        return NULL;

    /*---EXECUTION---*/

    pic = IMG_LoadTexture_RW(render, SDL_RWFromMem(buffer->data, buffer->size), 1);//closes rwops

    if (pic == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, IMG_GetError());
        return NULL;
    }

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return pic;
}


static CEV_GifAnim *L_rawToGif(CEV_FileInfo *buffer)
{/*converts to a gif animation*/

    /*---DECLARATIONS---*/

    CEV_GifAnim* animation  = NULL;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    /*---PRL---*/

    if ((buffer == NULL) || (buffer->size<=0) || (render == NULL))
        return NULL;

    /*---EXECUTION---*/

    animation = CEV_gifAnimLoadRW(SDL_RWFromMem(buffer->data, buffer->size), render, 1); //closes rwops

    if(animation == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create animation from data.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return animation;
}


static CEV_Chunk* L_rawToWav(CEV_FileInfo *buffer)
{/*convert to a wav sample*/

    /*---DECLARATIONS---*/

    CEV_Chunk   *result  = NULL;
    Mix_Chunk   *chunk   = NULL;
    SDL_RWops   *memFile = SDL_RWFromMem(buffer->data, buffer->size);

    /*---PRL---*/

    if ((buffer==NULL) || (buffer->size<=0))//bad arg
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
    result->virtualFile = buffer->data;

    SDL_RWclose(memFile);

    return result;

err_3:
    Mix_FreeChunk(chunk);

err_2:
    SDL_RWclose(memFile);

err_1:
    free(buffer->data);
    buffer->data = NULL;

    return NULL;
}


static CEV_Music* L_rawToMusic(CEV_FileInfo *buffer)
{/*convert to a wav sample*/

    /*---DECLARATIONS---*/

    CEV_Music   *result  = NULL;
    Mix_Music   *music   = NULL;
    SDL_RWops   *memFile = NULL;

    /*---PRL---*/

    if ((buffer==NULL) || (buffer->size<=0))//bad arg
        return NULL;

    /*---EXECUTION---*/

    memFile = SDL_RWFromMem(buffer->data, buffer->size);

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
    result->virtualFile = buffer->data;

    //SDL_RWclose(memFile);

    return result;

err_3:
    Mix_FreeMusic(music);

err_2:
    SDL_RWclose(memFile);

err_1:
    free(buffer->data);
    buffer->data = NULL;

    return NULL;
}


static CEV_Font* L_rawToFont(CEV_FileInfo* buffer)
{/*converts to CEV_Font*/

    /*---DECLARATIONS---*/

    CEV_Font* result    = NULL;//overlay to keep track of buffer->data until TTF_CloseFont
    TTF_Font* font      = NULL;
    SDL_RWops* memFile  = NULL;

    /*---PRL---*/

    if ((buffer==NULL) || (buffer->data == NULL) || (buffer->size<=0))
        goto err_exit;

    /*---EXECUTION---*/

    memFile  = SDL_RWFromMem(buffer->data, buffer->size);

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
    result->virtualFile = buffer->data;

    return result;

err_3:
    TTF_CloseFont(font);

err_2:
    SDL_RWclose(memFile);

err_1:
    free(buffer->data);
    buffer->data = NULL;

err_exit :
    return NULL;
}


static SP_AnimList* L_rawToAnimSet(CEV_FileInfo* buffer)
{/*converts to animSet */

    /*---DECLARATIONS---*/

    SP_AnimList *animSet = NULL;

    /*---EXECUTION---*/

    animSet = SP_animListLoad_RW(SDL_RWFromConstMem(buffer->data, buffer->size), 1);

    if (animSet == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create animSet.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return animSet;
}


static CEV_TileMap* L_rawToMap(CEV_FileInfo* buffer)
{/*convert to tile map*/

    /*---DECLARATIONS---*/

    CEV_TileMap *map = NULL;

    /*---EXECUTION---*/

    map = CEV_mapLoad_RW(SDL_RWFromConstMem(buffer->data, buffer->size), 1);

    if (map == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create map.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return map;
}


static CEV_Parallax* L_rawToParallax(CEV_FileInfo* buffer)
{/*convert to parallax*/

    /*---DECLARATIONS---*/

    CEV_Parallax *bckgd = NULL;

    /*---EXECUTION---*/

    bckgd = CEV_parallaxLoad_RW(SDL_RWFromConstMem(buffer->data, buffer->size), 1);

    if ( bckgd == NULL)
        fprintf(stderr, "Err at %s / %d : unable to create parallax.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return  bckgd;
}


static CEV_Weather* L_rawToWeather(CEV_FileInfo* buffer)
{/*convert to weather*/
        /*---DECLARATIONS---*/
    CEV_Weather *result = NULL;

        /*---EXECUTION---*/
    result = CEV_weatherLoad_RW(SDL_RWFromConstMem(buffer->data, buffer->size), 1);

    if (IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : unable to create weather.\n", __FUNCTION__, __LINE__);

    /*---POST---*/

    free(buffer->data);
    buffer->data = NULL;

    return  result;
}



static void L_gotoAndDiscard(unsigned int index, FILE *file)
{/*reach raw data discarding header*/

    L_gotoDataIndex(index, file);

    read_u32le(file);//discards type and size
    read_u32le(file);
}


static int L_fileInfoReadIndex(unsigned int index, CEV_FileInfo* buffer, FILE* file)
{/*reaches index and fills buffer**/

    /*---PRL---*/
    if (index<0 || buffer==NULL || file==NULL)
        return ARG_ERR;

    /*---EXECUTION---*/

    L_gotoDataIndex(index, file);         //goto data offset
    CEV_fileInfoTypeRead(file, buffer); //fills buffer

    if (readWriteErr)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to read header informations.\n", __FUNCTION__, __LINE__);
        return (FUNC_ERR);
    }

    /*---POST---*/

    return FUNC_OK;
}


static size_t L_gotoDataIndex(unsigned int index, FILE *file)
{/*goes to index in file*/

    size_t offset=0;

    fseek(file, index*sizeof(uint32_t), SEEK_SET);

    offset = read_u32le(file);

    fseek(file, offset, SEEK_SET);

    return offset;
}


static void L_infoReadRaw(CEV_FileInfo* buffer, FILE* file)
{/*fills and allocate buffer->data*//**1.0**/

    /*---PRL---*/

    if(buffer==NULL || file==NULL)
        readWriteErr++;

    buffer->data = malloc(buffer->size);

    /*---EXECUTION---*/

    if (buffer->data == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        readWriteErr++;
    }
    else if (fread(buffer->data, 1, buffer->size, file) != buffer->size)
    {
        fprintf(stderr, "Err at %s / %d : unable to read data in file : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        CEV_fileInfoClear(buffer);
        readWriteErr++;
    }
}


static size_t L_fileSize(FILE* file)
{/*size of a file*/

    long pos = ftell(file),
         size = 0;

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    fseek(file, pos, SEEK_SET);

    return(size);
}



