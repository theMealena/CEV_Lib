//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    06-2019    |   1.0    |    creation    **/
//**   CEV    |    12-2019    |   1.0.1  | GIF integrated **/
//**   CEV    |    04-2020    |   1.0.2  | free corrected **/
//**   CEV    |    06-2021    |   1.0.3  | features added **/
//**   CEV    |    06-2021    |   1.0.31 | modifications  **/
//**********************************************************/

/**LOG**/
//21/12/2019 CEV : added to display GIF animation
//06/04/2020 CEV : modified destroy functions to allow NULL as argument causing crash otherwise
//17/05/2021 CEV : structures dump added
//06/06/2021 CEV : structure modified X/Y axis parallax with options.
//03/03/2023 CEV : L_paraLayerTextureGet_RW modified, using new CEV_lib abilities.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <rwtypes.h>

#include "CEV_mixSystem.h"
#include "CEV_file.h"
#include "CEV_api.h"
#include "CEV_parallax.h"
#include "CEV_dataFile.h"
#include "CEV_texts.h"
#include "CEV_txtParser.h"


/** \brief Repeatedly blits texture on X
 *
 * \param render : SDL_Renderer* dst renderer.
 * \param texture : SDL_Texture* src texture.
 * \param blitPos : SDL_Rect as blit position on screen.
 * \param camWidth : int as camera width in pxl.
 *
 * \return void
 */
static void L_blitXrepeat(SDL_Renderer *render, SDL_Texture *texture, SDL_Rect blitPos, int camWidth);


/** \brief Repeatedly blits texture on Y
 *
 * \param render : SDL_Renderer* dst renderer.
 * \param texture : SDL_Texture* src texture.
 * \param blitPos : SDL_Rect as blit position on screen.
 * \param camHeight : int as camera height in pxl.
 *
 * \return void
 */
static void L_blitYrepeat(SDL_Renderer *render, SDL_Texture *texture, SDL_Rect blitPos, int camHeight);


/** \brief Writes single layer into RWops file.
 *
 * \param thisLayer : CEV_ParaLayer* to write into file.
 * \param dst : SDL_RWops* to write into.
 *
 * \return int : 0 on success, any value otherwise.
 */
static int L_paralayerWrite_RW(CEV_ParaLayer *thisLayer, SDL_RWops* dst);



/** \brief convert CSV file into data file.
 *
 * \param src : FILE* as CSV_file to read from.
 * \param dst : FILE* as file to write into.
 * \param folder : char* folder name.
 * \param layer : int as layer to convert.
 *
 * \return void
 *
 */
static void L_parallaxLayerTxtToData(CEV_Text *src, FILE *dst, char *folder, int layer);


/** \brief Inserts picture "fileName" into dst file.
 *
 * \param fileName : char* picture's file name.
 * \param dst : FILE* to save picture into.
 *
 * \return void
 */
static void L_parallaxPictureTypeWrite(char* fileName, FILE* dst);


/** \brief fetches layer pic from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_ParaLayer* to store result into.
 *
 * \return SDL_Texture* on success, NULL on failure.
 *
 */
static SDL_Texture *L_paraLayerTextureGet_RW(SDL_RWops* src, CEV_ParaLayer* dst);


/** \brief Converts PRLX mode as string to value.
 *
 * \param str : char* with litteral mode.
 *
 * \return int as value from PRLX_MODE.
 */
static int L_prlxModeStrToValue(char* str);


void TEST_parallax(void)
{//test / stress library

    CEV_parallaxConvertTxtToData("prlx\\parseTest.txt", "prlx\\bcgd01.plx");

    CEV_Parallax * thisPrlx = CEV_parallaxLoad("prlx\\bcgd01.plx");

    SDL_Rect
        world  = {0, 0, 800, 5000},
        camera = {0, 0, thisPrlx->layers[0].picSize.w/2, thisPrlx->layers[0].picSize.h};

    thisPrlx->cameraPos = &camera;
    thisPrlx->worldDim  = world;

    SDL_Rect showCam = CLEAR_RECT;
        CEV_rectDimCopy(camera, &showCam);

    CEV_Input* input = CEV_inputGet();
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    CEV_parallaxDump(thisPrlx);

    SDL_Color renderColor = {0, 0, 0, SDL_ALPHA_OPAQUE};
    SDL_Color drawColor ={.r=255,.g=0,.b=0,SDL_ALPHA_OPAQUE};

    bool quit = false;

    while(!quit)
    {
        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_RIGHT])
        {
            camera.x+=10;
            printf("cam.x = %d\n", camera.x);
        }

        if(input->key[SDL_SCANCODE_LEFT])
        {
            camera.x-=10;
            printf("cam.x = %d\n", camera.x);
        }

        if(input->key[SDL_SCANCODE_UP])
        {
            camera.y-=10;
            printf("cam.y = %d\n", camera.y);
        }

        if(input->key[SDL_SCANCODE_DOWN])
        {
            camera.y+=10;
            printf("cam.y = %d\n", camera.y);
        }

        if(input->key[SDL_SCANCODE_ESCAPE])
            quit = true;

        if(input->key[SDL_SCANCODE_F10])
        {
            CEV_screenSwitch();
            input->key[SDL_SCANCODE_F10] = false;
        }

        CEV_rectConstraint(&camera, world);

        CEV_parallaxShowAll(thisPrlx);

        CEV_renderColorSet(render, drawColor);
        SDL_RenderDrawRect(render, &showCam);
        CEV_renderColorSet(render, renderColor);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(10);
    }
}



CEV_Parallax* CEV_parallaxCreate(int numOfLayer, SDL_Rect *cameraPos)
{//allocates new parallax structure

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

    result->layers = calloc(numOfLayer, sizeof(CEV_ParaLayer));

    if(IS_NULL(result->layers))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate.\n ",__FUNCTION__,  __LINE__);
        free(result);
        return NULL;
    }

    result->cameraPos = cameraPos;
    result->numOfLayer = numOfLayer;

    return result;
}


void CEV_parallaxDestroy(CEV_Parallax *in)
{//free parallax structure content and itself

    if(IS_NULL(in))
    {
        fprintf(stderr, "Err at %s / %d :  arg is NULL, nothing to be done.\n", __FUNCTION__, __LINE__ );
        return;
    }

    CEV_parallaxClear(in);

    free(in);
}


void CEV_parallaxClear(CEV_Parallax *in)
{//clears CEV_Parallax content

    if(IS_NULL(in))
    {
        fprintf(stderr, "Err at %s / %d :  arg is NULL, nothing to be done.\n", __FUNCTION__, __LINE__ );
        return;
    }

    for(int i=0; i < in->numOfLayer; i++)
    {
        CEV_parallaxLayerClear(&in->layers[i]);
    }

    free(in->layers);
    CEV_memSet(in, 0, sizeof(CEV_Parallax));
}


void CEV_parallaxLayerClear(CEV_ParaLayer* in)
{//clears CEV_Parallax layer content / sets to 0

    if(IS_NULL(in))
    {
        fprintf(stderr, "Err at %s / %d :  arg is NULL, nothing to be done.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(in->isGif)
    {
        CEV_gifAnimFree(in->anim);//destroys gif + texture
    }
    else
    {
        SDL_DestroyTexture(in->texture);
    }

    CEV_memSet(in, 0, sizeof(CEV_ParaLayer));
}


void CEV_parallaxShowAll(CEV_Parallax *in)
{//displays all layres at once

    if(IS_NULL(in))
        return;

    for (int i=0; i<in->numOfLayer; i++)
        CEV_parallaxShowLayer(in, i);
}


void CEV_parallaxShowLayer(CEV_Parallax *in, unsigned index)
{//displays selected layer

    if(IS_NULL(in) || (index >= in->numOfLayer))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__ );
        return;
    }

    CEV_ParaLayer *thisLayer = &in->layers[index];

    SDL_Rect
        blitPos = thisLayer->picSize,
        camera  = *in->cameraPos,
        world   = in->worldDim;

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if(thisLayer->isGif)
        CEV_gifAnimAuto(thisLayer->anim);

    /*treating X*/

    if(thisLayer->axisPar[0].isRepeat)
    {
        //adding self velocity with modulo
        thisLayer->axisPar[0].pos = CEV_fModulo(thisLayer->axisPar[0].pos + thisLayer->axisPar[0].vel, blitPos.w);

        blitPos.x = -((int)(camera.x * thisLayer->axisPar[0].ratio) % blitPos.w)
                    + thisLayer->axisPar[0].pos
                    -((thisLayer->axisPar[0].ratio > 0.0)?
                                blitPos.w : 0.0);
    }
    else
    {
        switch(thisLayer->axisPar[0].posMode)
        {
            case PRLX_TOP :
            case PRLX_BOTTOM :
            case PRLX_AUTO :
                blitPos.x = (int)CEV_map(camera.x, 0, world.w - camera.w, 0, camera.w - blitPos.w);
            break;

            case PRLX_LEFT : //origin at left of world.
                blitPos.x = (world.w - camera.x - camera.w) * thisLayer->axisPar[0].ratio;
            break;

            case PRLX_RIGHT : //origin at right of world
                blitPos.x = camera.x * thisLayer->axisPar[0].ratio;
            break;

            case PRLX_CENTERED :
            break;
        }
    }

    int offset;
    /*treating Y*/
    if(thisLayer->axisPar[1].isRepeat)
    {
        //adding self velocity with modulo
        thisLayer->axisPar[1].pos = CEV_fModulo(thisLayer->axisPar[1].pos + thisLayer->axisPar[1].vel, blitPos.h);
        blitPos.y = -(((int)(camera.y * thisLayer->axisPar[1].ratio) % blitPos.h))
                    + thisLayer->axisPar[1].pos;
                    //-((thisLayer->axisPar[1].ratio >= 0.0)?
                                //blitPos.h : 0.0);
    }
    else
    {
        switch(thisLayer->axisPar[1].posMode)
        {
            case PRLX_RIGHT:
            case PRLX_LEFT :
            case PRLX_AUTO :
                blitPos.y = (int)CEV_map(camera.y, 0, world.h - camera.h, 0, camera.h - blitPos.h);
            break;

            case PRLX_BOTTOM : //origin at bottom of world.
				offset = (world.h - camera.h - camera.y - 1) * thisLayer->axisPar[1].ratio;;
                blitPos.y = (camera.h - blitPos.h) + offset;
            break;

            case PRLX_TOP : //origin at top of world
                //offset = camera.y * thisLayer->axisPar[1].ratio;
                blitPos.y = -camera.y * thisLayer->axisPar[1].ratio;
            break;

            case PRLX_CENTERED :
            break;
        }
    }

    if (thisLayer->axisPar[0].isRepeat && !thisLayer->axisPar[1].isRepeat)
    {
        //clip.h = camera.h;
        L_blitXrepeat(render, thisLayer->texture, blitPos, camera.w);
    }

    else if (thisLayer->axisPar[1].isRepeat && !thisLayer->axisPar[0].isRepeat)
    {
        L_blitYrepeat(render, thisLayer->texture, blitPos, camera.h);
    }
    else
        SDL_RenderCopy(render, thisLayer->texture, NULL, &blitPos);

}


void CEV_parallaxDump(CEV_Parallax *in)
{//dumps to stdout

    if(IS_NULL(in))
    {
        fprintf(stderr, "Err at %s / %d :  arg is NULL, nothing to be done.\n", __FUNCTION__, __LINE__ );
        return;
    }

    printf("numOfLayer = %d\n", in->numOfLayer);

    if(in->cameraPos)
        printf("camera at %p with %d, %d, %d, %d\n", in->cameraPos,
                                                    in->cameraPos->x,
                                                    in->cameraPos->y,
                                                    in->cameraPos->w,
                                                    in->cameraPos->h);
    else
        printf("camera is NULL\n");

    printf("world with %d, %d, %d, %d\n", in->worldDim.x,
                                        in->worldDim.y,
                                        in->worldDim.w,
                                        in->worldDim.h);

    if(in->layers)
    {
        printf("layers at %p\n", in->layers);

        for(int i=0; i<in->numOfLayer; i++)
        {
            printf("Layer %d content :\n", i);
            CEV_parallaxLayerDump(&in->layers[i]);
        }
    }
    else
        puts("Layers are not allocated");
}


void CEV_parallaxLayerDump(CEV_ParaLayer* in)
{//dumps to stdout

    printf("texture at : %p\n", in->texture);
    printf("picsize with %d, %d, %d, %d\n", in->picSize.x,
                                            in->picSize.y,
                                            in->picSize.w,
                                            in->picSize.h);

    for(int i=0; i<2; i++)
    {
        printf("Axis %c :\n", i?'y':'x');

        printf("ratio = %f, pos = %f, vel = %f\n", in->axisPar[i].ratio,
                                                    in->axisPar[i].pos,
                                                    in->axisPar[i].vel);

        printf("isrepeat = %s\n", in->axisPar[i].isRepeat? "true" :"false");
        printf("display mode = %d\n", in->axisPar[i].posMode);
    }

    printf("isGif = %s\n", in->isGif? "true" :"false");
    if(in->anim)
        printf("anim at %p\n", in->anim);
    else
        puts("anim is NULL");


    printf("x = %f, y = %f\n", in->axisPar[0].pos, in->axisPar[1].pos);
}



/**** file related functions ****/


int CEV_parallaxConvertTxtToData(const char* srcName, const char* dstName)
{//converts CSV file into data file

        /*---DECLARATIONS---*/
    uint32_t num = 0;

    CEV_Text *src   = NULL;
    FILE *dst       = NULL;

    char folderName[FILENAME_MAX] = "\0",
         fileLine[50],
         hasFolder =  0;

    readWriteErr = 0;
    hasFolder = CEV_fileFolderNameGet(srcName, folderName);

    //loading as CEV_Text to enable quick parsing
    src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : Unable to open file %s.\n ",__FUNCTION__,  __LINE__, srcName);
        goto err;
    }
    else
        CEV_textDump(src);

    //opening destination file
    dst = fopen(dstName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create file %s.\n ",__FUNCTION__,  __LINE__, dstName);
        goto err_1;
    }

    //number of layers
    num = (uint32_t)CEV_txtParseValueFrom(src, "layer num");
    write_u32le(num, dst);

    for(int i=0; i<num; i++)
    {
        L_parallaxLayerTxtToData(src, dst, hasFolder? folderName : NULL, i);
    }

    if (readWriteErr)
    {
        fprintf(stderr, "Err at %s / %d : read / write error.\n ",__FUNCTION__,  __LINE__);
        goto err_2;
    }

    CEV_textDestroy(src);
    fclose(dst);

    return FUNC_OK;

        /*---ERROR---*/
err_2:
    fclose(dst);

err_1:
    CEV_textDestroy(src);

err:
    return FUNC_ERR;
}


CEV_Parallax* CEV_parallaxLoad(const char* fileName)
{//load parallax data file
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


int CEV_parallaxSave(CEV_Parallax* src, const char* fileName)
{//save parallax into file

    int funcSts = FUNC_OK;

    FILE* dst = fopen(fileName, "wb");

    CEV_Capsule caps;

    CEV_parallaxToCapsule(src, &caps);

    if(fwrite(caps.data, 1, caps.size, dst) != caps.size)
    {
        fprintf(stderr, "Err at %s / %d : could not write into file %s : %d.\n", __FUNCTION__, __LINE__, fileName, ferror(dst));
        funcSts = FUNC_ERR;
    }

    fclose(dst);
    CEV_capsuleClear(&caps);

    return funcSts;
}


CEV_Parallax* CEV_parallaxLoad_RW(SDL_RWops* ops, uint8_t freeSrc)
{//loads parallax from RWops

    CEV_Parallax* result = NULL;
    uint32_t temp32;
    float* toFloat = (float*)&temp32;

    result = malloc(sizeof(CEV_Parallax));

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate result : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    SDL_RWseek(ops, 0, RW_SEEK_SET);

    result->numOfLayer = SDL_ReadLE32(ops); //number of layer in file

    result->layers = calloc(result->numOfLayer , sizeof(CEV_ParaLayer));

    if IS_NULL(result->layers)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate layers : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_2;
    }

    for (int i =0; i<result->numOfLayer; i++)
    {
        result->layers[i].picSize = (SDL_Rect){0,0,0,0};

        for(int j=0; j<2;j++)
        {
            result->layers[i].axisPar[j].pos = 0;
            temp32  = SDL_ReadLE32(ops);//reading ratio
            result->layers[i].axisPar[j].ratio = *toFloat;//to float

            temp32   = SDL_ReadLE32(ops);//velocity
            result->layers[i].axisPar[j].vel = *toFloat;//to float

            result->layers[i].axisPar[j].isRepeat = SDL_ReadU8(ops);
            result->layers[i].axisPar[j].posMode = SDL_ReadU8(ops);
        }

        L_paraLayerTextureGet_RW(ops, &result->layers[i]);

        SDL_QueryTexture(result->layers[i].texture,
                            NULL,
                            NULL,
                            &result->layers[i].picSize.w,
                            &result->layers[i].picSize.h);

    }

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


int CEV_parallaxWrite_RW(CEV_Parallax* src, SDL_RWops* dst)
{//writes parallax into RWops file

    readWriteErr += SDL_WriteBE32(dst, src->numOfLayer);

    for (int i=0; i<src->numOfLayer; i++)
        readWriteErr += L_paralayerWrite_RW(&src->layers[i], dst);

    return readWriteErr;
}


int CEV_parallaxToCapsule(CEV_Parallax* src, CEV_Capsule *dst)
{//creates capsule from structure

    int funcSts = FUNC_OK;

    //creating virtual file
    SDL_RWops* vFile = SDL_AllocRW();

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate vFile : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    funcSts = CEV_parallaxWrite_RW(src, vFile);

    if(funcSts != FUNC_OK)
    {
        fprintf(stderr, "Err at %s / %d : unable to write into virtuel file.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    size_t size = SDL_RWsize(vFile);
    dst->size = size;
    dst->type = IS_PLX;
    dst->data = malloc(size);

    if(IS_NULL(dst->data))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    SDL_RWread(vFile, dst->data, size, 1);

end:
    SDL_RWclose(vFile);
    return funcSts;
}


/*** Local functions ***/

static void L_blitXrepeat(SDL_Renderer *render, SDL_Texture *texture, SDL_Rect blitPos, int camWidth)
{//repeatedly blits texture on X

    while(blitPos.x < camWidth)
    {
        SDL_RenderCopy(render, texture, NULL, &blitPos);
        blitPos.x += blitPos.w;
    }
}


static void L_blitYrepeat(SDL_Renderer *render, SDL_Texture *texture, SDL_Rect blitPos, int camHeight)
{//repeatedly blits texture on Y

    while(blitPos.y < camHeight)
    {
        SDL_RenderCopy(render, texture, NULL, &blitPos);
        blitPos.y += blitPos.h;
    }
}


static int L_paralayerWrite_RW(CEV_ParaLayer *thisLayer, SDL_RWops* dst)
{//writes parallax layer into RWops file

    int sdlRWerr = 1;
    uint32_t temp32 = 0;
    float* toFloat = (float*)&temp32;

    for (int i=0; i<2; i++)
    {
        *toFloat = thisLayer->axisPar[i].ratio;
        sdlRWerr *= SDL_WriteLE32(dst, temp32);
        *toFloat = thisLayer->axisPar[i].vel;
        sdlRWerr *= SDL_WriteLE32(dst, temp32);
        sdlRWerr *= SDL_WriteU8(dst, thisLayer->axisPar[i].isRepeat);
        sdlRWerr *= SDL_WriteU8(dst, thisLayer->axisPar[i].posMode);
    }

    return sdlRWerr;
}


static void L_parallaxLayerTxtToData(CEV_Text *src, FILE *dst, char *folder, int layer)
{//converts csv into data

    float ratio, vel;

    char fileName[FILENAME_MAX] = "\0",
         lString[FILENAME_MAX],
         fileLine[100];

    char  repeat, posMode;

    for(int i=0; i<2; i++)
    {
        char axis = i?'Y' :'X';
        //ratio
        sprintf(lString, "[%d]%cratio", layer, axis);
        ratio = (float)CEV_txtParseValueFrom(src, lString);
        //printf("%s is %d\n", lString, ratio);
        //velocity
        sprintf(lString, "[%d]%cvel", layer, axis);
        vel = (float)CEV_txtParseValueFrom(src, lString);
        //printf("%s is %d\n", lString, vel);
        //repeat
        sprintf(lString, "[%d]%crepeat", layer, axis);
        repeat = (char)CEV_txtParseValueFrom(src, lString);
        //printf("%s is %d\n", lString, repeat);
        //positionning
        sprintf(lString, "[%d]%cposition", layer, axis);
        posMode = L_prlxModeStrToValue(CEV_txtParseTxtFrom(src, lString));
        //printf("%s is %d\n", lString, posMode);


        write_f32le(ratio, dst);//writing scroll ratio
        write_f32le(vel, dst);  //writing velocity
        write_u8(repeat, dst);  //writting repeat mode
        write_u8(posMode, dst); //writting position mode
    }

    sprintf(lString, "[%d]picture", layer);

    char *picName = CEV_txtParseTxtFrom(src, lString);


    if(folder != NULL)
    {
        strcpy(fileName, folder);
        strcat(fileName, picName);
    }
    else
        strcpy(fileName, picName);

    L_parallaxPictureTypeWrite(fileName, dst);//inserting pic into file
}


static void L_parallaxPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic named fileName

    printf("inserting picture %s...", fileName);
    CEV_Capsule buffer;

    CEV_capsuleFromFile(&buffer, fileName);

    CEV_capsuleTypeWrite(&buffer, dst);

    printf("%s\n", readWriteErr? "nok":"ok");

    CEV_capsuleClear(&buffer);
}


static SDL_Texture *L_paraLayerTextureGet_RW(SDL_RWops* ops, CEV_ParaLayer* layer)
{//extracts picture from ops file

    /*---DECLARATIONS---*/
/*
    uint32_t        textureSize = 0,
                    type        = 0;
    SDL_Renderer    *render     = CEV_videoSystemGet()->render;
    void            *rawData    = NULL;
*/
    /*---PRL---*/

    if IS_NULL(ops)
        return NULL;

    /*---EXECUTION---*/

    CEV_Capsule img = {0};

    CEV_capsuleTypeRead_RW(ops, &img);
/*
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
*/
    if (IS_PIC(img.type))
    {
        layer->texture = CEV_capsuleExtract(&img, true);//frees data

        if IS_NULL(layer->texture)
        {
            fprintf(stderr,"Err at %s / %d : unable to extract capsule content.\n",__FUNCTION__, __LINE__);
            return NULL;
        }
    }
    else if (img.type == IS_GIF)
    {
        layer->isGif = true;
        layer->anim = CEV_capsuleExtract(&img, true);//frees data

        if(IS_NULL(layer->anim))
        {
            fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, SDL_GetError());
            return NULL;
        }

        layer->texture = CEV_gifTextureGet(layer->anim);
        CEV_gifLoopMode(layer->anim, GIF_REPEAT_FOR);
    }
    else
        CEV_capsuleClear(&img);

    return layer->texture;
}


static int L_prlxModeStrToValue(char* str)
{
    char* ref[PRLX_LAST] = PRLX_TYPE_NAMES;

    for (int i = 0; i < PRLX_LAST; i++)
    {
        if (!strcmp(str, ref[i]))
            return i;
    }

    return -1;//default value
}
