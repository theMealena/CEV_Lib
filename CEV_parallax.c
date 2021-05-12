#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "CEV_mixSystem.h"
#include "CEV_texts.h"
#include "CEV_dataFile.h"
#include "parallax.h"
#include "project_def.h"


void L_parallaxTypeWrite(FILE *src, FILE *dst, char *folder);

void L_parallaxPictureTypeWrite(char* fileName, FILE* dst);

SDL_Texture *L_parallaxTextureGet_RW(SDL_RWops* ops, ParaLayer* layer);


CEV_Parallax* CEV_parallaxCreate(int numOfLayer, SDL_Rect *scroll)
{
    if(numOfLayer < 0)
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n ",__FUNCTION__,  __LINE__);
        return NULL;
    }


    CEV_Parallax *result = calloc(1, sizeof(CEV_Parallax));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate.\n ",__FUNCTION__,  __LINE__);
        return NULL;
    }

    result->layers = calloc(numOfLayer, sizeof(ParaLayer));

    if(IS_NULL(result->layers))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate.\n ",__FUNCTION__,  __LINE__);
        free(result);
        return NULL;
    }

    result->scrollPos = scroll;
    result->numOfLayer = numOfLayer;

    return result;
}


void CEV_parallaxDestroy(CEV_Parallax *in)
{
    for(int i=0; i < in->numOfLayer; i++)
    {
        if(in->layers[i].isGif)
            CEV_gifAnimFree(in->layers->anim);
        else
            SDL_DestroyTexture(in->layers[i].texture);
    }

    free(in->layers);
    free(in);
}


void CEV_parallaxUpdate(CEV_Parallax *in)
{

    //printf("in parralax : %d\n", SDL_GetTicks());
    int xPos = in->scrollPos->x;
    SDL_Rect blitPos;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    //printf("1 = %d\n", SDL_GetTicks());

    for (int i=0; i<in->numOfLayer; i++)
    {
        //printf("2-%d = %d\n",i, SDL_GetTicks());
        ParaLayer *thisLayer = &in->layers[i];
        blitPos = thisLayer->picSize;

        if(thisLayer->isGif)
        {
            CEV_gifAnimAuto(thisLayer->anim);
        }

        //printf("picsize = %d, %d, blitPos =%d, %d\n", thisLayer->picSize.w, thisLayer->picSize.h,blitPos.w,blitPos.h);
        //printf("scroll = %d, %d, %d, %d\n", in->scrollPos->x, in->scrollPos->y,in->scrollPos->w,in->scrollPos->h);

        thisLayer->x += thisLayer->velX;

		if ((thisLayer->velX > 0.0) && ((int)thisLayer->x >= 0))
        {
            thisLayer->x = -blitPos.w;//(float)in->scrollPos->w;
        }

        else if((thisLayer->velX < 0.0) && ((int)thisLayer->x < -blitPos.w+1 ))
        {
           thisLayer->x = 0.0;
        }

        //printf("at %d : ratio = %f, vel =%f\n",i, thisLayer->ratio, thisLayer->velX);

        int startBlitX = -(((int)(xPos*thisLayer->ratio) % thisLayer->picSize.w)) + thisLayer->x;
        blitPos.x = startBlitX;

        //printf("startBlitX = %d\n", startBlitX);

        int loop = 0;

        while(blitPos.x < (in->scrollPos->w+1) && loop<500)
        {
            //printf("3-%d = %d\n",i, SDL_GetTicks());
            //printf("blitpos.x = %d\n", blitPos.x);
            blitPos.y = in->scrollPos->h - in->layers[i].picSize.h;
            SDL_RenderCopy(render, thisLayer->texture, NULL, &blitPos);
            blitPos.x += blitPos.w;
            //printf("4-%d = %d\n",i, SDL_GetTicks());
            loop++;
        }
    }

    //printf("out parralax : %d\n", SDL_GetTicks());
}


/**** file related functions ****/

int CEV_convertParallaxCSVToData(const char* srcName, const char* dstName)
{
        /*---DECLARATIONS---*/
    uint32_t num = 0;

    FILE *src = NULL,
         *dst = NULL;


    char folderName[FILENAME_MAX] = "\0",
         hasFolder =  0;

        /*---PRL---*/

    readWriteErr = 0;
    hasFolder = CEV_fileFolder(srcName, folderName);


    src = fopen(srcName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : Unable to open file %s.\n ",__FUNCTION__,  __LINE__, srcName);
        goto err;
    }

    dst = fopen(dstName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create file %s.\n ",__FUNCTION__,  __LINE__, dstName);
        goto err_1;
    }

        /*---EXECUTION---*/

    fscanf(src, "%d", (int*)&num);

    write_u32le(num, dst);

    for(int i=0; i<num; i++)
    {
        L_parallaxTypeWrite(src, dst, hasFolder? folderName : NULL);
    }

    if (readWriteErr)
    {
        fprintf(stderr, "Err at %s / %d : read / write error.\n ",__FUNCTION__,  __LINE__);
        goto err_2;
    }

    fclose(src);
    fclose(dst);

    return FUNC_OK;

        /*---ERROR---*/
err_2:
    fclose(dst);

err_1:
    fclose(src);

err:
    return FUNC_ERR;
}


CEV_Parallax* CEV_parallaxLoad(const char* fileName)
{
     /*---DECLARATIONS---*/

    CEV_Parallax *result = NULL;
    SDL_RWops    *ops = NULL;

    /*---PRL---*/

    if IS_NULL(fileName)
    {
        fprintf(stderr,"Err at %s / %d : fileName is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    ops = SDL_RWFromFile(fileName, "rb");

    if IS_NULL(ops)
    {
        fprintf(stderr, "Err at %s / %d : unable to open file %s: %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
        return NULL;
    }

    /*---POST---*/

    result = CEV_parallaxLoad_RW(ops, 1);//frees ops

    return result;
}


CEV_Parallax* CEV_parallaxLoad_RW(SDL_RWops* ops, uint8_t freeSrc)
{//load animation set from RWops

    /*---DECLARATIONS---*/

    CEV_Parallax* result = NULL;
    uint32_t temp32;
    float* toFloat = (float*)&temp32;

    /*---PRL---*/

    result = malloc(sizeof(CEV_Parallax));

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate result : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    /*---EXECUTION---*/

    SDL_RWseek(ops, 0, RW_SEEK_SET);

    result->numOfLayer = SDL_ReadLE32(ops); //number of layer in file

    result->layers = calloc(result->numOfLayer , sizeof(ParaLayer));

    if IS_NULL(result->layers)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate layers : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_2;
    }

    for (int i =0; i<result->numOfLayer; i++)
    {
        result->layers[i].picSize = (SDL_Rect){0,0,0,0};
        result->layers[i].x = result->layers[i].y = 0;

        temp32  = SDL_ReadLE32(ops);//reading ratio

        result->layers[i].ratio = *toFloat;//to float

        temp32   = SDL_ReadLE32(ops);//reading X velocity

        result->layers[i].velX = *toFloat;//to float

        L_parallaxTextureGet_RW(ops, &result->layers[i]);

        SDL_QueryTexture(result->layers[i].texture,
                            NULL,
                            NULL,
                            &result->layers[i].picSize.w,
                            &result->layers[i].picSize.h);

    }

    /*---POST---*/

    if (freeSrc)
        SDL_RWclose(ops);

    return result;

    /*---ERROR---*/

err_2 :
    free(result);

err_1 :
    if (freeSrc)
        SDL_RWclose(ops);

    return NULL;
}

/*** Local functions ***/

void L_parallaxTypeWrite(FILE *src, FILE *dst, char *folder)
{
    float ratio, vel;
    char fileName[FILENAME_MAX] = "\0",
         lString[FILENAME_MAX];

    fscanf(src, "%f\t%f\t%s", &ratio, &vel, lString);//reading parameters and pic name

    write_f32le(&ratio, dst); //writting scroll ratio
    write_f32le(&vel, dst);   //writting velocity

    if(folder != NULL)
    {
        strcpy(fileName, folder);
        strcat(fileName, lString);
    }
    else
        strcpy(fileName, lString);

    L_parallaxPictureTypeWrite(fileName, dst);//inserting pic into file
}


void L_parallaxPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic

    printf("Opening picture %s.\n", fileName);
    CEV_FileInfo buffer;

    CEV_rawLoad(&buffer, fileName);

    CEV_fileInfoTypeWrite(&buffer, dst);

    free(buffer.data);
}


SDL_Texture *L_parallaxTextureGet_RW(SDL_RWops* ops, ParaLayer* layer)
{//extract SDL_Texture from ops file

    /*---DECLARATIONS---*/

    uint32_t        textureSize = 0,
                    type = 0;
    SDL_Renderer    *render     = CEV_videoSystemGet()->render;
    void            *rawData    = NULL;

    /*---PRL---*/

    if IS_NULL(ops)
        return NULL;

    /*---EXECUTION---*/

    type = SDL_ReadLE32(ops);

    textureSize = SDL_ReadLE32(ops);

    rawData = malloc(textureSize);

    if IS_NULL(rawData)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate rawData : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    //reading raw data
    SDL_RWread(ops, rawData, 1, textureSize);

    if (IS_PIC(type))
    {
        layer->texture = IMG_LoadTexture_RW(render, SDL_RWFromConstMem(rawData, textureSize), 1);/*closes RWops*/

        if IS_NULL(layer->texture)
        {
            fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, IMG_GetError());
            return NULL;
        }
    }
    else if (type == IS_GIF)
    {
        layer->isGif = true;
        layer->anim = CEV_gifAnimLoadRW(SDL_RWFromConstMem(rawData, textureSize), render, 1);/*closes RWops*/

        if(IS_NULL(layer->anim))
        {
            fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, SDL_GetError());
            return NULL;
        }

        layer->texture = CEV_gifTexture(layer->anim);
        CEV_gifLoopMode(layer->anim, GIF_REPEAT_FOR);
    }

    SDL_SetTextureBlendMode(layer->texture, SDL_BLENDMODE_BLEND);

    /*---POST---*/

    free(rawData);

    return layer->texture;
}
