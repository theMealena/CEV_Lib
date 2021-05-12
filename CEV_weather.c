
/**LOG**/
//06/04/200 CEV : modified free functions to allow NULL as argument causing crash otherwise

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include "CEV_weather.h"
#include "CEV_mixSystem.h"
#include "CEV_file.h"
#include "CEV_dataFile.h"
#include "project_def.h"

/**locals structures**/

typedef struct SSnowFlake
{//snow particle
    CEV_FCoord coord;
    SDL_Rect pos;
    float factor;
    int xMain, angle;
    bool disp;
}
SnowFlake;


typedef struct SRainDrop
{//rain particle
    CEV_FCoord coord,
                vector;
    SDL_Rect pos;
    bool disp;
}
RainDrop;


typedef struct SFallLeaf
{//leaf particle
    CEV_FCoord coord;
    SDL_Rect pos;
    float factor, w;
    int xMain, angle;
    bool disp;
}
FallLeaf;


struct SWeather
{//weather
    uint8_t type;
    bool run,
         anyActive;
    int Vx, Vy;
    unsigned int num, numax;
    int *scrollCorrectionX, *scrollCorrectionY;
    float angle;
    SDL_Rect renderSize, basePic;
    SDL_Texture *texture;
    void *particles;
};

/**local functions**/

/**constructor**/
//inserts picture in file
void L_weatherPictureTypeWrite(char* fileName, FILE* dst);

//type read from file into enum
char L_weatherNameToType(char *name);

/**snow**/
//creates snowflakes table
SnowFlake *L_createSnow(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx);

//sets one snowflake parameters
void L_newFlake(SnowFlake *in, SDL_Rect renderRect, SDL_Rect base, int vx);

//displays snow weather
void L_snowShow(CEV_Weather *in, float min , float max);

//corrects snowFlake position
void L_flakeInScreen(SnowFlake* in, SDL_Rect renderSize, int Vy);

//moves snowflake
void L_snowFlakePos(SnowFlake* in, int vx, int vy);


/**rain**/
//creates raindrop table
RainDrop *L_createRain(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx, int vy);

//displays rain weather
void L_rainShow(CEV_Weather *in, float min , float max);

//corrects raindrop position
void L_dropInScreen(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy);

//moves raindrop
void L_rainDropPos(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy);


/**fall**/
//creates leaves table
FallLeaf *L_createFall(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx);

//sets one leaf parmeters
void L_newLeaf(FallLeaf *in, SDL_Rect renderRect, SDL_Rect base, int vx);

//displays fall
void L_fallShow(CEV_Weather *in, float min , float max);

//correct leaf position on screeen
void L_leafInScreen(FallLeaf* in, SDL_Rect renderSize, int Vy);

//moves one leaf
void L_fallLeafPos(FallLeaf *in, int vx, int vy);

/**common**/
//calculates screen Modulo for scrolling purpose
void L_screenModulo(SDL_Rect *in, SDL_Rect limit);


void CEV_weatherPosCorrectX(CEV_Weather *in, int *ptr)
{
    in->scrollCorrectionX = ptr;
}


void CEV_weatherPosCorrectY(CEV_Weather *in, int *ptr)
{
    in->scrollCorrectionY = ptr;
}


CEV_Weather *CEV_weatherCreate(SDL_Texture *texture, uint8_t type, unsigned int num, int vx, int vy)
{/*creates instance*/

    /*---PRL---*/

    if(NULL == texture)
    {//checking texture
        fprintf(stderr, "Err at %s / %d : texture not provided.\n ", __FUNCTION__,  __LINE__);
        return NULL;
    }

    if(type>=WEATHER_NUM)
    {//cheching weather type request
        fprintf(stderr, "Err at %s / %d : weather type too high, corrected to \"snow\".\n ", __FUNCTION__, __LINE__);
        type = WEATHER_SNOW;
    }

    /*allocating result*/
    CEV_Weather *result = malloc(sizeof(*result));

    if(NULL == result)
    {//on error
        fprintf(stderr, "Err at %s / %d : %s  .\n ", __FUNCTION__,  __LINE__, strerror(errno));
        goto exit;
    }

    /*saving parameters*/
    SDL_QueryTexture(texture, NULL, NULL, &result->basePic.w, &result->basePic.h);

    SDL_RenderGetLogicalSize(CEV_videoSystemGet()->render,
                              &result->renderSize.w,
                              &result->renderSize.h);

    result->texture = texture;
    SDL_SetTextureBlendMode(texture,
                            SDL_BLENDMODE_BLEND);

    result->type = type;
    result->num = num;
    result->numax = num;
    result->scrollCorrectionX = NULL;
    result->scrollCorrectionY = NULL;
    result->Vx = vx;
    result->Vy = vy;
    result->run = false;
    result->anyActive = false;

    /*creating particle instance*/
    switch(type)
    {
        case WEATHER_SNOW :

            result->particles = (void*)L_createSnow(num, result->renderSize, result->basePic, vx);

        break;

        case WEATHER_RAIN :

            result->particles = (void*)L_createRain(num, result->renderSize, result->basePic, vx, vy);
            result->angle = CEV_vectAngle(vx, vy);

        break;

        case WEATHER_FALL :

            result->particles = (void*)L_createFall(num, result->renderSize, result->basePic, vx);

        break;

    }

    if(NULL == result->particles)
    {//on error
        fprintf(stderr, "Err at %s / %d : Couldn't create particles instances.\n ",__FUNCTION__,  __LINE__);
        goto err_1;
    }


exit :
    return result;


err_1:

    CEV_weatherFree(result, false);

    return NULL;
}


void CEV_weatherFree(CEV_Weather *in, bool freePic)
{/*frees instance & content*/

    if(IS_NULL(in))
        return;

    if(freePic)
        SDL_DestroyTexture(in->texture);

    free(in->particles);
    free(in);
}


void CEV_weatherShow(CEV_Weather*in)
{/*displays effect*/

    void (*displayWeather[WEATHER_NUM])(CEV_Weather*, float, float) = {L_snowShow, L_rainShow, L_fallShow};

    if((in == NULL) || (in->texture == NULL))
        return;

    //if(in->run || in->anyActive)
        displayWeather[in->type](in, 0.0, 1.0);

}


void CEV_weatherShowWithLayer(CEV_Weather*in, float min, float max)
{/*displays only selected sized particles*/

    void (*displayWeather[WEATHER_NUM])(CEV_Weather*, float, float) = {L_snowShow, L_rainShow, L_fallShow};

    if(in->texture == NULL)
        return;

    CEV_fconstraint(0.0, &min, max);
    CEV_fconstraint(min, &max, 1.0);

    //if(in->run || in->anyActive)
        displayWeather[in->type](in, min, max);

}


unsigned int CEV_weatherNumSet(CEV_Weather *in, unsigned int num)
{/*modify num of particles to display*/

    if(num > in->numax)
        num = in->numax;

    in->num = num;

    return in->num;
}


unsigned int CEV_weatherNumReset(CEV_Weather *in)
{/*resets num of particle to max*/

    in->num = in->numax;

    return in->num;
}


void CEV_weatherStart(CEV_Weather *in)
{
    in->run = true;
}


void CEV_weatherStop(CEV_Weather *in)
{
    in->run = false;
}


/**FILE constructor**/

int CEV_convertWeatherCSVToData(char* src, char* dst)
{
    puts("preparing to convert weather...");
    FILE* fSrc = NULL,
         *fDst = NULL;

    int funcSts = FUNC_OK,
            vx  = 0,
            vy  = 0;

    char type[20] = "WEATHER_RAIN",
         picName[FILENAME_MAX] ="\0",
         folderName[FILENAME_MAX],
         fileLine[100];

    char hasFolder = CEV_fileFolderNameGet(src, folderName);

    unsigned int numOfParticles = 0;

    readWriteErr = 0;

    printf("opening %s...", src);
    fSrc = fopen(src, "r");
    if(NULL == fSrc)
    {//on error
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }
    puts("ok");

    printf("creating %s...", dst);
    fDst = fopen(dst,"wb");
    if(NULL == fDst)
    {//on error
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }
    puts("ok");

    rewind(fSrc);

    do
        fgets(fileLine, sizeof(fileLine), fSrc);
    while (fileLine[0]=='/');

    sscanf(fileLine, "%s %d %d %d %s\n", type, &numOfParticles, &vx, &vy, picName);

    write_u8(L_weatherNameToType(type), fDst);
    write_u32le(numOfParticles, fDst);
    write_u32le(vx, fDst);
    write_u32le(vy, fDst);

    if(hasFolder)
    {
        strcat(folderName, picName);
        strcpy(picName, folderName);
    }

    L_weatherPictureTypeWrite(picName, fDst);

    if(readWriteErr)
        funcSts = FUNC_ERR;


    fclose(fDst);

err_1:
    fclose(fSrc);

end:

    return funcSts;
}


CEV_Weather *CEV_weatherLoad(char *fileName)
{

    CEV_Weather *result = NULL;

    if(IS_NULL(fileName))
    {//on error
        fprintf(stderr, "Err at %s / %d : fileName is NULL.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    SDL_RWops *ops = SDL_RWFromFile(fileName, "rb");

    if IS_NULL(ops)
    {
        fprintf(stderr, "Err at %s / %d : unable to open file %s: %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
        return NULL;
    }

    /*---POST---*/

    result = CEV_weatherLoad_RW(ops, 1);//frees ops

    return result;
}


CEV_Weather *CEV_weatherLoad_RW(SDL_RWops* ops, char freeSrc)
{

    CEV_Weather *result = NULL;
    SDL_Texture *picture = NULL;

    if(IS_NULL(ops))
    {
        fprintf(stderr, "Err at %s / %d : received NULL ops :%s\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    uint8_t type = SDL_ReadU8(ops);
    uint32_t numOfParticles = SDL_ReadLE32(ops);
    int32_t vx = SDL_ReadLE32(ops),
            vy = SDL_ReadLE32(ops);

    CEV_FileInfo picInfos;

    picInfos.type = SDL_ReadLE32(ops);
    picInfos.size = SDL_ReadLE32(ops);

    if(!IS_PIC(picInfos.type))
    {
        fprintf(stderr, "Err at %s / %d : data is not picture.\n", __FUNCTION__, __LINE__);
        goto err_1;
    }

    picInfos.data = malloc(picInfos.size);

    if IS_NULL(picInfos.data)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate data : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    //reading raw data
    SDL_RWread(ops, picInfos.data, 1, picInfos.size);

    picture = IMG_LoadTexture_RW(CEV_videoSystemGet()->render, SDL_RWFromConstMem(picInfos.data, picInfos.size), 1);/*closes RWops*/

    if IS_NULL(picture)
    {
        fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, IMG_GetError());
        goto err_2;
    }

    SDL_SetTextureBlendMode(picture, SDL_BLENDMODE_BLEND);

    result = CEV_weatherCreate(picture, type, numOfParticles, vx, vy);

    if(IS_NULL(result))
    {
        fprintf(stderr,"Err at %s / %d :unable to create weather instance.\n",__FUNCTION__, __LINE__);
        SDL_DestroyTexture(picture);
    }

err_2:
    free(picInfos.data);

err_1:
    if(freeSrc)
        SDL_RWclose(ops);

    return result;
}


void L_weatherPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic

    printf("inserting picture %s...", fileName);
    CEV_FileInfo buffer;

    CEV_rawLoad(&buffer, fileName);

    CEV_fileInfoTypeWrite(&buffer, dst);

    printf("%s\n", readWriteErr? "nok" : "ok");

    free(buffer.data);
}


char L_weatherNameToType(char *name)
{
    if(!strcmp(name, "WEATHER_RAIN"))
        return WEATHER_RAIN;
    else if (!strcmp(name, "WEATHER_SNOW"))
        return WEATHER_SNOW;
    else if (!strcmp(name, "WEATHER_FALL"))
        return WEATHER_FALL;

    return 0;
}

/**LOCALS**/

/** SNOW **/


SnowFlake *L_createSnow(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    SnowFlake *result = calloc(num, sizeof(*result));

    if(NULL == result)
    {//on error
        fprintf(stderr, "Err at %s / %d :couldn't allocate %d snow instance: %s  .\n ",__FUNCTION__,  __LINE__,num, strerror(errno));
        return NULL;
    }

    for(int i =0; i<num; i++)
    {//settings
        L_newFlake(&result[i], renderRect, base, vx);
    }

    return result;
}


void L_newFlake(SnowFlake *in, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    in->factor  = (float)(rand()%vx);           //randomizing amplitude
    in->coord.z = CEV_frand(0.1, 1.0);          //randomizing size
    in->xMain   = rand()%renderRect.w;          //randomizing main pos
    in->coord.x = in->xMain;
    in->coord.y = (float)(rand()%renderRect.h); //randomizing Y pos
    in->angle   = rand()%360;                   //randomaizing rotation pos
    in->pos     = base;
    in->pos.x   = in->coord.x;
    CEV_rectDimScale(&in->pos, in->coord.z);    //scaling display rect
    in->pos.y   = -in->pos.h;                   //starts with display off screen
    in->disp    = false;
}


void L_snowShow(CEV_Weather *in, float min , float max)
{
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    //in->anyActive = false;

    for(int i=0; i<in->num; i++)
    {
        SnowFlake *Lparticle = (SnowFlake*)in->particles + i;

        if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
        {
            L_snowFlakePos(Lparticle, in->Vx, in->Vy);

            L_flakeInScreen(Lparticle,in->renderSize, in->Vy);

            SDL_Rect temp = Lparticle->pos;

            /*correcting display position for scrolling*/
            temp.x -= (((in->scrollCorrectionX != NULL)? *in->scrollCorrectionX : 0)%(in->renderSize.w + temp.w));
            temp.y -= (((in->scrollCorrectionY != NULL)? *in->scrollCorrectionY : 0)%(in->renderSize.h + temp.h));
            L_screenModulo(&temp, in->renderSize);

            SDL_Point local = {temp.w/2, temp.h/2};

            if(in->run && (temp.y <= 0))
                Lparticle->disp = true;
            else if (!in->run && ((temp.y + temp.h)>= in->renderSize.h))
                Lparticle->disp = false;

            if(Lparticle->disp)
            {
                //in->anyActive = true;

                SDL_RenderCopyEx(render,
                                in->texture,
                                NULL,
                                &temp,
                                (double)Lparticle->angle,
                                &local,
                                SDL_FLIP_NONE);
            }

        }

    }
}


void L_flakeInScreen(SnowFlake* in, SDL_Rect renderSize, int Vy)
{

    if(Vy>0 && (in->pos.y > renderSize.h))
    {//if going down
        in->coord.y = (float)-in->pos.h;
        in->pos.y = in->coord.y;
    }
    else if(Vy<0 && (in->pos.y < -in->pos.h))
    {//if going up
        in->coord.y = (float)renderSize.h;
        in->pos.y = in->coord.y;
    }
}


void L_snowFlakePos(SnowFlake* in, int vx, int vy)
{
    double local = in->angle*PI/2;

    in->coord.y += in->coord.z*vy;
    in->coord.x = (float)cos(local/90)*in->factor + in->xMain;//90 to keep pos/angle modulo
    in->angle += 1;
    in->angle %= 360;

    in->pos.x = in->coord.x;
    in->pos.y = in->coord.y;

}


/**RAIN**/


RainDrop *L_createRain(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx, int vy)
{
    unsigned int eighty,
                fifteen,
                five;

    RainDrop *result = calloc(num, sizeof(*result));


    if(NULL == result)
    {
        fprintf(stderr, "Err at %s / %d :couldn't allocate : %s  .\n ",__FUNCTION__,  __LINE__, strerror(errno));
        return NULL;
    }

    eighty = num *80 / 100; //eighty % of particles in back ground
    fifteen = num *15 / 100; //fifteen % in middle
    five = num - eighty - fifteen; //what's left in foreground

    for(int i =0; i<num; i++)
    {
        if(i<five)
            result[i].coord.z = CEV_frand(0.85, 1.0);
        else if (i<five+fifteen)
            result[i].coord.z = CEV_frand(0.3, 0.85);
        else
            result[i].coord.z = CEV_frand(0.1, 0.3);

        result[i].pos = base;
        CEV_rectDimScale(&result[i].pos, result[i].coord.z);

        result[i].coord.y = (float)(rand()%renderRect.h);
        result[i].coord.x = (float)(rand()%renderRect.w);
        result[i].disp    = false;
    }

    return result;
}


void L_rainShow(CEV_Weather *in, float min , float max)
{

    SDL_Renderer *render = CEV_videoSystemGet()->render;
    //in->anyActive = false;

    for(int i=0; i<in->num; i++)
    {
        RainDrop * Lparticle = (RainDrop*)in->particles + i;

        if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
        {
            L_rainDropPos(Lparticle, in->renderSize, in->Vx, in->Vy);

            L_dropInScreen(Lparticle, in->renderSize, in->Vx, in->Vy);

            SDL_Rect temp = Lparticle->pos;

            /*correcting display position for scrolling*/
            temp.x -= (((in->scrollCorrectionX != NULL)? *in->scrollCorrectionX : 0)%(in->renderSize.w + temp.w));
            temp.y -= (((in->scrollCorrectionY != NULL)? *in->scrollCorrectionY : 0)%(in->renderSize.h + temp.h));
            L_screenModulo(&temp, in->renderSize);

            SDL_Point local = {temp.w/2, temp.h/2};

            if(in->run && (temp.y <= 0))
                Lparticle->disp = true;
            else if(!in->run && (temp.y + temp.w >= in->renderSize.h))
                Lparticle->disp = false;

            if(Lparticle->disp)
            {
                //in->anyActive = true;

                SDL_RenderCopyEx(render,
                                in->texture,
                                NULL,
                                &temp,
                                (double)in->angle,
                                &local,
                                SDL_FLIP_NONE);
            }
        }
    }
}


void L_dropInScreen(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy)
{
    if(Vy>0 && (in->pos.y > renderSize.h))
    {
        in->coord.y = (float)-in->pos.h;
        in->pos.y = in->coord.y;
    }
    else if(Vy<0 && (in->pos.y < -in->pos.h))
    {
        in->coord.y = (float)renderSize.h;
        in->pos.y = in->coord.y;
    }

    if(Vx>0 && (in->pos.x > renderSize.w))
    {
        in->coord.x = (float)-in->pos.w;
        in->pos.x = in->coord.x;
    }
    else if(Vx<0 && (in->pos.x < -in->pos.w))
    {
        in->coord.x = (float)renderSize.w;
        in->pos.x = in->coord.x;
    }
}


void L_rainDropPos(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy)
{
    in->coord.y += Vy * in->coord.z;
    in->coord.x += Vx * in->coord.z;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


/**FALL**/


FallLeaf *L_createFall(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    FallLeaf *result = calloc(num, sizeof(*result));

    if(NULL == result)
    {
        fprintf(stderr, "Err at %s / %d : couldn't allocate : %s.\n ",__FUNCTION__,  __LINE__, strerror(errno));
        return NULL;
    }

    for(int i =0; i<num; i++)
    {
        L_newLeaf(&result[i], renderRect, base, vx);
        result[i].coord.y = (float)(rand()%renderRect.h);
    }

    return result;
}


void L_newLeaf(FallLeaf *in, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    in->factor = (float)(rand()%vx);
    in->coord.z = CEV_frand(0.3, 1.0);
    in->xMain = rand()%renderRect.w;
    in->w = (float)rand()/(float)RAND_MAX;
    in->angle = rand()%360;
    in->pos = base;
    CEV_rectDimScale(&in->pos, in->coord.z);
    in->disp = false;
}


void L_fallShow(CEV_Weather *in, float min , float max)
{
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    //in->anyActive = false;

    for(int i=0; i<in->num; i++)
    {
        FallLeaf *Lparticle = (FallLeaf*)in->particles + i;

        if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
        {
            SDL_RendererFlip flip = SDL_FLIP_NONE;

            L_fallLeafPos(Lparticle, in->Vx, in->Vy);
            L_leafInScreen(Lparticle, in->renderSize, in->Vy);

            SDL_Rect temp = Lparticle->pos;

            /*correcting modulo for scrolling*/
            temp.x -= (((in->scrollCorrectionX != NULL)? *in->scrollCorrectionX : 0)%(in->renderSize.w + temp.w));
            temp.y -= (((in->scrollCorrectionY != NULL)? *in->scrollCorrectionY : 0)%(in->renderSize.h + temp.h));
            L_screenModulo(&temp, in->renderSize);

            temp.w *= (float)cos((double)Lparticle->w);

            if(temp.w <0.0)
            {
                temp.w = -temp.w;
                flip = SDL_FLIP_HORIZONTAL;
            }

            SDL_Point local={temp.w/2, temp.h/2};

            if(in->run && (temp.y <= 0))
                Lparticle->disp = true;
            else if (!in->run && ((temp.y + temp.h/2) >= in->renderSize.h))
                Lparticle->disp = false;

            if(Lparticle->disp)
            {
                //in->anyActive = true;
                SDL_RenderCopyEx(render,
                                in->texture,
                                NULL,
                                &temp,
                                Lparticle->angle,
                                &local,
                                flip);
            }

        }
    }
}


void L_leafInScreen(FallLeaf* in, SDL_Rect renderSize, int Vy)
{
    if(Vy>0 && (in->pos.y > renderSize.h))
    {
        in->coord.y = (float)-in->pos.h;
        in->pos.y = in->coord.y;
    }
    else if(Vy<0 && (in->pos.y < -in->pos.h))
    {
        in->coord.y = (float)renderSize.h;
        in->pos.y = in->coord.y;
    }
}


void L_fallLeafPos(FallLeaf *in, int vx, int vy)
{
    double local = in->angle/2*PI;

    in->coord.y += in->coord.z*vy;
    in->coord.x = (float)cos(local/90)*in->factor + in->xMain;
    in->w += .01;

    if(in->w >= (2*PI))
        in->w = 0.0;

    in->angle +=1;
    in->angle %= 360;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


void L_screenModulo(SDL_Rect *in, SDL_Rect limit)
{
    if(in->x  > limit.w)
        in->x -= limit.w + in->w;
    else if (in->x < -in->w)
        in->x += limit.w + in->w;

    if(in->y  > limit.h)
        in->y -= limit.h + in->h;
    else if (in->y < -in->h)
        in->y += limit.h + in->h;
}



