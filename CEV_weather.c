//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**********************************************************/

/**LOG**/
//06/04/2017 CEV    : modified free functions to allow NULL as argument causing crash otherwise.
//20/02/2023 CEV    : revision, replaced most "manual operations" by functions provided by CEV_Lib.
//                    added / modified function in CEV_lib standard.
//                      CEV_Camera can be attached for scrolling display
//                    Doxy comments added


/*
landmark used is as :
    0
    .___x
   /|
  / |
 z  y

with (float)Z = 1.0 as nearest, 0.0.. as furthest
*/

// TODO (drx#1#): trouver une solution pour gérer le anyActive et éviter des calculs quand il n'y a rien à afficher

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <rwtypes.h>
#include <CEV_weather.h>
#include <CEV_mixSystem.h>
#include <CEV_file.h>
#include <CEV_dataFile.h>
#include <project_def.h>
#include <CEV_camera.h>

/// locals structures definitions**/



typedef struct Sparticle
{
    CEV_FCoord coord,   /**< actual calculated absolute position / z used as deepness of field. */
                vector; /**< actual vector. */

    SDL_Rect pos;       /**< display position scaled by Z coord. */

    int xMain,          /**< main X position of which particle moves COSINUS around. */
        angle,          /**< display rotation angle. */
        offScreen;      /**< Offscreen offet to get sure particle is fully invisible */

    float factor,       /**< VX randomly scaled. */
          dispWidth;    /**< display width, used for vertical axis rotation effect. */

    bool disp;          /**< to be displayed or not. */
}
Particle;


/** \brief Single snowflake-like particle instance definition.
*
* \note Falls on COSINUS-like wave with Z-around rotation.
*/
typedef struct SSnowFlake
{//snow particle

    CEV_FCoord coord;   /**< actual calculated absolute position / z used as deepness of field. */

    SDL_Rect pos;       /**< display position scaled by Z coord. */

    float factor;       /**< randomized x amplitude movement / COSINUS factor. */

    int xMain,          /**< main X position of which particle moves COSINUS around. */
        angle,          /**< display rotation angle. */
        offScreen;      /**< Offscreen offet to get sure particle is fully invisible */

    bool disp;          /**< to be displayed or not */
}
SnowFlake;


/** \brief Single raindrop-like particle instance definition.
*
* \note Falls straight on defined angle.
 */
typedef struct SRainDrop
{//rain particle
    CEV_FCoord coord,   /**< actual calculated absolute position / z used as deepness of field. */
                vector; /**< actual vector. */

    SDL_Rect pos;       /**< display position scaled by Z coord. */

    int offScreen;      /**< Offscreen offet to get sure particle is fully invisible */

    bool disp;          /**< to be displayed or not. */
}
RainDrop;


/** \brief Single leaf-like particle instance definition.
*
* \note Falls on COSINUS-like wave with ZY-around rotation.
 */
typedef struct SFallLeaf
{//leaf particle
    CEV_FCoord coord;   /**< actual calculated absolute position / z used as deepness of field. */

    SDL_Rect pos;       /**< display position scaled by Z coord. */

    float factor,       /**< display scale. */
          dispWidth;    /**< display width, used for vertical axis rotation effect. */

    int xMain,          /**< main X position of which particle moves COSINUS like around. */
        angle,          /**< display rotation angle. */
        offScreen;      /**< Offscreen offet to get sure particle is fully invisible */

    bool disp;          /**< to be displayed or not. */
}
FallLeaf;


/** \brief Weather main handler structure.
 */
struct SWeather
{//weather
    uint8_t type;   /**< particle type from enum. */

    bool run,       /**< weather calculation / display is active. */
         anyActive; /**< 1 particle at least is still alive (used when stopped). */

    int Vx,         /**< x value / x vector / x amplitude. */
        Vy;         /**< y value / y vector / y amplitude. */

    unsigned int num,       /**< actual num of particles to display. */
                numax;      /**< max num of particles available for display. */

    int *scrollCorrectionX, /**< enables scroll correction, link to camera x pos. */
        *scrollCorrectionY; /**< enables scroll correction, link to camera y pos. */

    float angle;            /**< RAIN ONLY as falling rain angle / direction. */

    SDL_Rect renderSize,    /**< main render size. */
             textureSize;   /**< particle picture size. */

    SDL_Texture *texture;   /**< particle pic. */

    void *particles;        /**< array of numax particles instances. */
};


/// local functions

/// constructor

/** \brief inserts picture in file (CEV_convertWeatherCSVToData usage).
 *
 * \param fileName : char* as picture file name to insert.
 * \param dst : FILE* to insert picture into (@actual position).
 *
 * \return void
 */
static void L_weatherPictureTypeWrite(char* fileName, FILE* dst);


/** \brief Converts string like particle type into enum value
 *
 * \param name : char* as particle type string.
 *
 * \return int as appropriate enum, -1 if type unknown.
 */
static int L_weatherNameToType(char *name);

/// snow dedicated functions

// TODO (drx#1#): remove renderRect arg and fetch render dim inside function.
/** \brief Creates / fills snowflakes instance table.
 *
 * \param num : unsigned int as num of particle to create.
 * \param renderRect : SDL_Rect as render dimensions.
 * \param base : SDL_Rect as texture base dimensions (unscaled).
 * \param vx : int as delta X amplitude around main X position.
 *
 * \return SnowFlake* on success, NULL on error.
 */
static SnowFlake* L_snowCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx);


/** \brief Sets / Fills flake instance values.
 *
 * \param in : SnowFlake* to be set.
 * \param renderRect : SDL_Rect as render dimensions.
 * \param base : SDL_Rect as texture base dimensions (unscaled).
 * \param vx : int as delta X amplitude around main X position.
 *
 * \return void
 */
static void L_flakeCreate(SnowFlake *in, SDL_Rect renderRect, SDL_Rect base, int vx);


/** \brief Displays snow-like weather
 *
 * \param in : CEV_Weather* to be displayed.
 * \param min : float as min size of particles to be displayed.
 * \param max : float as max size of particles to be displayed.
 *
 * \return void
 *
 * \note Only particles scaled within [min, max] are updated / displayed.
 */
static void L_snowShow(CEV_Weather *in, float min , float max);


/** \brief Calculates display relative position from absolute position.
 *
 * \param in : SnowFlake* to perform calculation on.
 * \param renderSize : SDL_Rect as render dimensions.
 * \param Vy  :int as Y velocity.
 *
 * \return void
 *
 * \note Called by L_snowShow(), is not called if particle does not belong to [min, max].
 */
static bool L_flakePosToDisplay(SnowFlake* in, SDL_Rect renderSize);


/** \brief Updates pos & angle to next state.
 *
 * \param in : SnowFlake* to update.
 * \param vx : int as delta X amplitude around main X position.
 * \param vy : int as Y velocity.
 *
 * \return void
 *
 * \note Called by L_snowShow(), is not called if particle does not belong to [min, max].
 */
static void L_flakePosNext(SnowFlake* in, int vx, int vy);

void L_flakeMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

void L_flakeMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

/// rain dedicated functions

/** \brief Creates / fills rain instance table.
 *
 * \param num : unsigned int as num of particle to create.
 * \param renderRect : SDL_Rect as render dimensions.
 * \param base : SDL_Rect as texture base dimensions (unscaled).
 * \param vx : int as delta X amplitude around main X position.
 * \param vy : int as Y velocity.
 *
 * \return RainDrop* on success, NULL on error.
 */
static RainDrop* L_rainCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx, int vy);


/** \brief Displays rain-like weather
 *
 * \param in : CEV_Weather* to be displayed.
 * \param min : float as min size of particles to be displayed.
 * \param max : float as max size of particles to be displayed.
 *
 * \return void
 *
 * \note Only particles scaled within [min, max] are updated / displayed.
 */
static void L_rainShow(CEV_Weather *in, float min , float max);


/** \brief Calculates display relative position from absolute position.
 *
 * \param in : RainDrop* to perform calculation on.
 * \param renderSize : SDL_Rect as render dimensions.
 * \param Vx  :int as X velocity vector (used for signess).
 * \param Vy  :int as Y velocity vector (used for signess).
 *
 * \return void
 *
 * \note Called by L_rainShow(), is not called if particle does not belong to [min, max].
 */
static bool L_dropPosToDisplay(RainDrop* in, SDL_Rect renderSize);


/** \brief Updates pos & angle to next state.
 *
 * \param in : RainDrop* to update.
 * \param renderSize : SDL_Rect as render dimensions.
 * \param Vx  :int as X velocity vector (used for signess).
 * \param Vy  :int as Y velocity vector (used for signess).
 *
 * \return void
 *
 * \note Called by L_rainShow(), is not called if particle does not belong to [min, max].
 */
static void L_dropPosNext(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy);


void L_dropMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

void L_dropMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

/// fall dedicated functions

 /** \brief Creates / fills fall-like instance table.
 *
 * \param num : unsigned int as num of particle to create.
 * \param renderRect : SDL_Rect as render dimensions.
 * \param base : SDL_Rect as texture base dimensions (unscaled).
 * \param vx : int as delta X amplitude around main X position.
 *
 * \return FallLeaf* on success, NULL on error.
 */
static FallLeaf *L_fallCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx);


/** \brief Sets / Fills leaf instance values.
 *
 * \param in : FallLeaf* to be set.
 * \param renderRect : SDL_Rect as render dimensions.
 * \param base : SDL_Rect as texture base dimensions (unscaled).
 * \param vx : int as delta X amplitude around main X position.
 *
 * \return void
 */
static void L_leafCreate(FallLeaf *in, SDL_Rect renderRect, SDL_Rect base, int vx);


/** \brief Displays fall-like weather
 *
 * \param in : CEV_Weather* to be displayed.
 * \param min : float as min size of particles to be displayed.
 * \param max : float as max size of particles to be displayed.
 *
 * \return void
 *
 * \note Only particles scaled within [min, max] are updated / displayed.
 */
static void L_fallShow(CEV_Weather *in, float min , float max);


/** \brief Calculates display relative position from absolute position.
 *
 * \param in : FallLeaf* to perform calculation on.
 * \param renderSize : SDL_Rect as render dimensions.
 * \param Vy  :int as Y velocity.
 *
 * \return void
 *
 * \note Called by L_fallShow(), is not called if particle does not belong to [min, max].
 */
static bool L_leafPosToDisplay(FallLeaf* in, SDL_Rect renderSize);


/** \brief Updates pos & angle to next state.
 *
 * \param in : FallLeaf* to update.
 * \param vx : int as delta X amplitude around main X position.
 * \param vy : int as Y velocity.
 *
 * \return void
 *
 * \note Called by L_fallShow(), is not called if particle does not belong to [min, max].
 */
static void L_leafPosNext(FallLeaf *in, int vx, int vy);

void L_leafMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

void L_leafMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim);

///common locals

/** \brief Recalculate particle position with display's modulo
 *
 * \param in : SDL_Rect* as particle pos to be modulo'd
 * \param limit : SDL_Rect as "to fit in" rectangle on which modulo is applied.
 *
 * \return void
 */
static bool L_screenModulo(SDL_Rect *in, SDL_Rect limit, int offScreen);


///module testing and stress

// TODO (drx#1#): Vérifier les offscreen pour la disparition / apparition eds particules ...
//+ apparition en haut écran si particules montantes au moment du start.
//
void TEST_weather(void)
{

    SDL_Renderer *render = CEV_videoSystemGet()->render;
    CEV_Input* input = CEV_inputGet();

    SDL_Texture *particle = CEV_textureLoad("snowflake2.png");

    CEV_Weather* weather = CEV_weatherCreate(particle, WEATHER_SNOW, 1000, 35, 2);

    CEV_Camera cam;

    CEV_weatherCameraSet(&cam, weather);

    bool quit = false;


    while(!quit)
    {
        uint32_t now = SDL_GetTicks();

        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_ESCAPE] || input->window.quitApp)
            quit = true;

        if(input->key[SDL_SCANCODE_RETURN])
            CEV_weatherStart(weather);

        if(input->key[SDL_SCANCODE_SPACE])
            CEV_weatherStop(weather);

        if(input->key[SDL_SCANCODE_RIGHT])
            cam.scrollActPos.x+=10;

        if(input->key[SDL_SCANCODE_LEFT])
            cam.scrollActPos.x-=10;

        if(input->key[SDL_SCANCODE_DOWN])
            cam.scrollActPos.y+=10;

        if(input->key[SDL_SCANCODE_UP])
            cam.scrollActPos.y-=10;

        if(input->key[SDL_SCANCODE_LSHIFT])
            CEV_weatherMaxSize(0.8, weather, weather->textureSize);

        if(input->key[SDL_SCANCODE_RSHIFT])
            CEV_weatherMinSize(0.5, weather, weather->textureSize);


        CEV_weatherShow(weather);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        printf("elapsed = %u\n", SDL_GetTicks()-now);

    }

    CEV_weatherDestroy(weather, true);
}

/// USER END

void CEV_weatherCameraSet(CEV_Camera *src, CEV_Weather* dst)
{//attaches camera

    dst->scrollCorrectionX = &src->scrollActPos.x;
    dst->scrollCorrectionY = &src->scrollActPos.y;
}


CEV_Weather *CEV_weatherCreate(SDL_Texture *texture, uint8_t type, unsigned int num, int vx, int vy)
{//creates instance

    if(IS_NULL(texture))
    {//checking texture
        fprintf(stderr, "Err at %s / %d : texture not provided.\n ", __FUNCTION__,  __LINE__);
        return NULL;
    }

    if(type >= WEATHER_NUM)
    {//cheching weather type request
        fprintf(stderr, "Err at %s / %d : weather type too high, corrected to \"snow\".\n ", __FUNCTION__, __LINE__);
        type = WEATHER_SNOW;
    }

    //allocating result
    CEV_Weather *result = calloc(1, sizeof(*result));

    if(NULL == result)
    {//on error
        fprintf(stderr, "Err at %s / %d : %s.\n ", __FUNCTION__,  __LINE__, strerror(errno));
        goto exit;
    }

    //setting parameters
    result->textureSize = CEV_textureDimGet(texture);

    SDL_RenderGetLogicalSize(CEV_videoSystemGet()->render,
                              &result->renderSize.w,
                              &result->renderSize.h);

    result->texture = texture;

    SDL_SetTextureBlendMode(texture,
                            SDL_BLENDMODE_BLEND);

    result->type                = type;
    result->num                 = num;
    result->numax               = num;
    result->scrollCorrectionX   = NULL;
    result->scrollCorrectionY   = NULL;
    result->Vx                  = vx;
    result->Vy                  = vy;
    result->run                 = false;
    result->anyActive           = false;

    //creating particle instance
    switch(type)
    {
        case WEATHER_SNOW :

            result->particles = (void*)L_snowCreate(num, result->renderSize, result->textureSize, vx);

        break;

        case WEATHER_RAIN :

            result->particles = (void*)L_rainCreate(num, result->renderSize, result->textureSize, vx, vy);
            result->angle = CEV_vectAngle(vx, vy);

        break;

        case WEATHER_FALL :

            result->particles = (void*)L_fallCreate(num, result->renderSize, result->textureSize, vx);

        break;

    }

    if(IS_NULL(result->particles))
    {//on error
        fprintf(stderr, "Err at %s / %d : Couldn't create particles instances.\n ",__FUNCTION__,  __LINE__);
        goto err_1;
    }


exit :
    return result;


err_1:

    CEV_weatherDestroy(result, false);

    return NULL;
}


void CEV_weatherDestroy(CEV_Weather *in, bool freePic)
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


void CEV_weatherMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    switch (dst->type)
    {
        case WEATHER_RAIN :
            L_dropMaxSize(src, dst, textureDim);
        break;

        case WEATHER_SNOW :
            L_flakeMaxSize(src, dst, textureDim);
        break;

        case WEATHER_FALL :
            L_leafMaxSize(src, dst, textureDim);
        break;
    }
}

void CEV_weatherMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    switch (dst->type)
    {
        case WEATHER_RAIN :
            L_dropMinSize(src, dst, textureDim);
        break;

        case WEATHER_SNOW :
            L_flakeMinSize(src, dst, textureDim);
        break;

        case WEATHER_FALL :
            L_leafMinSize(src, dst, textureDim);
        break;
    }
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

    sscanf(fileLine, "%s %u %d %d %s\n", type, &numOfParticles, &vx, &vy, picName);

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

    CEV_Capsule picInfos;

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


static void L_weatherPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic

    printf("inserting picture %s...", fileName);
    CEV_Capsule buffer;

    CEV_capsuleLoad(&buffer, fileName);

    CEV_capsuleWrite(&buffer, dst);

    printf("%s\n", readWriteErr? "nok" : "ok");

    free(buffer.data);
}


static int L_weatherNameToType(char *name)
{//converts type string to equivalent enum

    if(!strcmp(name, "WEATHER_RAIN"))
        return WEATHER_RAIN;
    else if (!strcmp(name, "WEATHER_SNOW"))
        return WEATHER_SNOW;
    else if (!strcmp(name, "WEATHER_FALL"))
        return WEATHER_FALL;

    return -1;
}

/**LOCALS**/

/** SNOW **/


static SnowFlake *L_snowCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    SnowFlake *result = calloc(num, sizeof(*result));

    if(NULL == result)
    {//on error
        fprintf(stderr, "Err at %s / %d :couldn't allocate %d snow instance: %s  .\n ",__FUNCTION__,  __LINE__,num, strerror(errno));
        return NULL;
    }

    for(int i =0; i<num; i++)
    {//settings
        L_flakeCreate(&result[i], renderRect, base, vx);
    }

    return result;
}


static void L_flakeCreate(SnowFlake *in, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    in->factor      = (float)(rand()%vx);           //randomizing amplitude
    in->coord.z     = CEV_frand(0.1, 1.0);          //randomizing size
    in->xMain       = rand()%renderRect.w;          //randomizing main X pos
    in->coord.x     = in->xMain;
    in->coord.y     = CEV_frand(0.0,(float)renderRect.h); //randomizing Y pos
    in->angle       = rand()%360;                         //randomizing rotation pos
    in->pos         = base;
    in->pos.x       = in->coord.x;
    CEV_rectDimScale(&in->pos, in->coord.z);    //scaling display rect
    in->offScreen   = 2 * CEV_pointDist(CLEAR_POINT, (SDL_Point){base.w, base.h}); //display diagonal is offscreen
    in->disp        = false;
}


static void L_snowShow(CEV_Weather *in, float min , float max)
{
    bool LanyActive = false;
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    //in->anyActive = false;
    if(in->run || in->anyActive)
    {
        for(int i=0; i<in->num; i++)
        {

            SnowFlake *Lparticle = (SnowFlake*)in->particles + i;

            if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
            {
                //calculating next position (absolute)
                L_flakePosNext(Lparticle, in->Vx, in->Vy);

                //calculating position on display (relative)
                //correcting display position for scrolling compensation

                Lparticle->pos.x -= (IS_NULL(in->scrollCorrectionX)?
                                        0 :
                                        (int)(*in->scrollCorrectionX * Lparticle->coord.z))
                                            %(in->renderSize.w + Lparticle->offScreen);

                Lparticle->pos.y -= (IS_NULL(in->scrollCorrectionY)?
                                        0 :
                                        (int)(*in->scrollCorrectionY * Lparticle->coord.z))
                                            %(in->renderSize.h + Lparticle->offScreen);

                if(L_flakePosToDisplay(Lparticle,in->renderSize))
                    Lparticle->disp = in->run;

                SDL_Point local = {Lparticle->pos.w/2, Lparticle->pos.h/2};

                if(Lparticle->disp)
                {
                    SDL_RenderCopyEx(render,
                                    in->texture,
                                    NULL,
                                    &Lparticle->pos,
                                    (double)Lparticle->angle,
                                    &local,
                                    SDL_FLIP_NONE);

                    LanyActive = true;
                }
            }
        }
    }

    in->anyActive = LanyActive;
}


static bool L_flakePosToDisplay(SnowFlake* in, SDL_Rect renderSize)
{
    bool isOffScreen = false;

    if(in->pos.y > renderSize.h)
    {
        in->pos.y -= renderSize.h + in->offScreen;
        in->coord.y -= renderSize.h + in->offScreen;
    }
    else if(in->pos.y < -in->offScreen)
    {
        in->pos.y += renderSize.h + in->offScreen;
        in->coord.y += renderSize.h + in->offScreen;
    }

    if(in->pos.y <= -in->offScreen/2)
        isOffScreen = true;

    if(in->pos.x > renderSize.w)
    {
        in->pos.x -= renderSize.w + in->offScreen;
    }
    else if(in->pos.x < -in->offScreen)
    {
       in->pos.x += renderSize.w + in->offScreen;
    }

    return isOffScreen;
}


static void L_flakePosNext(SnowFlake* in, int vx, int vy)
{
    double local = in->angle*PI/2;

    in->coord.y += in->coord.z * vy;

    in->coord.x = (float)cos(local/90) * in->factor + in->xMain; //90 to keep pos/angle modulo
    in->angle = (in->angle+1)%360;

    in->pos.x = in->coord.x;
    in->pos.y = in->coord.y;
}


void L_flakeMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    SnowFlake *this = (SnowFlake*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MIN(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }
}


void L_flakeMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    SnowFlake *this = (SnowFlake*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MAX(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }
}


/**RAIN**/


static RainDrop *L_rainCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx, int vy)
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

        result[i].offScreen = 2 * CEV_pointDist(CLEAR_POINT, (SDL_Point){base.w, base.h});//display diagonal is offscreen
        result[i].coord.y = (float)(rand()%renderRect.h);
        result[i].coord.x = (float)(rand()%renderRect.w);
        result[i].disp    = false;
    }

    return result;
}


static void L_rainShow(CEV_Weather *in, float min , float max)
{
    bool LanyActive = false;
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    //in->anyActive = false;

    if(in->run || in->anyActive)
    {
        for(int i=0; i<in->num; i++)
        {
            RainDrop * Lparticle = (RainDrop*)in->particles + i;

            if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
            {
                L_dropPosNext(Lparticle, in->renderSize, in->Vx, in->Vy);

                //correcting display position with scrolling
                Lparticle->pos.x -= (IS_NULL(in->scrollCorrectionX)?
                                        0 :
                                        (int)(*in->scrollCorrectionX * Lparticle->coord.z))
                                            %(in->renderSize.w + Lparticle->offScreen);

                Lparticle->pos.y -= (IS_NULL(in->scrollCorrectionY)?
                                        0 :
                                        (int)(*in->scrollCorrectionY * Lparticle->coord.z))
                                            %(in->renderSize.h + Lparticle->offScreen);

                //display allowed when off screen not to brutaly appear
                if(L_dropPosToDisplay(Lparticle, in->renderSize))
                    Lparticle->disp = in->run;

                SDL_Point local = {Lparticle->pos.w/2, Lparticle->pos.h/2}; // as center of rotation

                if(Lparticle->disp)
                {
                    SDL_RenderCopyEx(render,
                                    in->texture,
                                    NULL,
                                    &Lparticle->pos,
                                    (double)in->angle,
                                    &local,
                                    SDL_FLIP_NONE);

                    LanyActive = true;
                }
            }
        }
    }
    in->anyActive = LanyActive;
}


static bool L_dropPosToDisplay(RainDrop* in, SDL_Rect renderSize)
{
    bool isOffScreen = false;

    if(in->pos.y > renderSize.h)
    {
        in->pos.y -= renderSize.h + in->offScreen;
        in->coord.y -= renderSize.h + in->offScreen;
    }
    else if(in->pos.y < -in->offScreen)
    {
        in->pos.y += renderSize.h + in->offScreen;
        in->coord.y += renderSize.h + in->offScreen;
    }

    if(in->pos.y <= -in->offScreen/2)
        isOffScreen = true;

    if(in->pos.x > renderSize.w)
    {
        in->pos.x -= renderSize.w + in->offScreen;
        in->coord.x -= renderSize.w + in->offScreen;
    }
    else if(in->pos.x < -in->offScreen)
    {
       in->pos.x += renderSize.w + in->offScreen;
       in->coord.x += renderSize.w + in->offScreen;
    }

    return isOffScreen;
}


static void L_dropPosNext(RainDrop* in, SDL_Rect renderSize, int Vx, int Vy)
{
    in->coord.y += Vy * in->coord.z;
    in->coord.x += Vx * in->coord.z;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


void L_dropMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    RainDrop *this = (RainDrop*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MIN(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }
}


void L_dropMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    RainDrop *this = (RainDrop*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MAX(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }
}


/**FALL**/


static FallLeaf *L_fallCreate(unsigned int num, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    FallLeaf *result = calloc(num, sizeof(*result));

    if(NULL == result)
    {
        fprintf(stderr, "Err at %s / %d : couldn't allocate : %s.\n ",__FUNCTION__,  __LINE__, strerror(errno));
        return NULL;
    }

    for(int i =0; i<num; i++)
    {
        L_leafCreate(&result[i], renderRect, base, vx);
        result[i].coord.y = (float)(rand()%renderRect.h);
    }

    return result;
}


static void L_leafCreate(FallLeaf *in, SDL_Rect renderRect, SDL_Rect base, int vx)
{
    in->factor      = (float)(rand()%vx);
    in->coord.z     = CEV_frand(0.3, 1.0);
    in->xMain       = rand()%renderRect.w;
    in->dispWidth   = (float)rand()/RAND_MAX;
    in->angle       = rand()%360;
    in->pos         = base;
    CEV_rectDimScale(&in->pos, in->coord.z);
    in->offScreen   = 2 * CEV_pointDist(CLEAR_POINT, (SDL_Point){base.w, base.h}); //display diagonal is offscreen
    in->disp        = false;
}


static void L_fallShow(CEV_Weather *in, float min , float max)
{//displays fall like weather

    bool LanyActive = false;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if(in->run ||in->anyActive)
    {
        for(int i=0; i<in->num; i++)
        {
            FallLeaf *Lparticle = (FallLeaf*)in->particles + i;

            if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
            {

                L_leafPosNext(Lparticle, in->Vx, in->Vy);

                //correcting modulo for scrolling
                Lparticle->pos.x -= (IS_NULL(in->scrollCorrectionX)?
                                        0 :
                                        (int)(*in->scrollCorrectionX * Lparticle->coord.z))
                                            %(in->renderSize.w + Lparticle->offScreen);

                Lparticle->pos.y -= (IS_NULL(in->scrollCorrectionY)?
                                        0 :
                                        (int)(*in->scrollCorrectionY * Lparticle->coord.z))
                                            %(in->renderSize.h + Lparticle->offScreen);

                //activating particle when offscreen
                if(L_leafPosToDisplay(Lparticle, in->renderSize))
                    Lparticle->disp = in->run;

                SDL_Rect temp = Lparticle->pos;//copied to keep original width untouched

                SDL_RendererFlip flip = SDL_FLIP_NONE;
                temp.w *= (float)cos((double)Lparticle->dispWidth);
                if(temp.w < 0.0)
                {
                    temp.w = -temp.w;
                    flip = SDL_FLIP_HORIZONTAL;
                }

                SDL_Point local={temp.w/2, temp.h/2};

                if(Lparticle->disp)
                {
                    SDL_RenderCopyEx(render,
                                    in->texture,
                                    NULL,
                                    &temp,
                                    Lparticle->angle,
                                    &local,
                                    flip);

                    LanyActive = true;
                }

            }
        }
    }

    in->anyActive = LanyActive;
}


static bool L_leafPosToDisplay(FallLeaf* in, SDL_Rect renderSize)
{
    bool isOffScreen = false;

    if(in->pos.y > renderSize.h)
    {
        in->pos.y -= renderSize.h + in->offScreen;
        in->coord.y -= renderSize.h + in->offScreen;
    }
    else if(in->pos.y < -in->offScreen)
    {
        in->pos.y += renderSize.h + in->offScreen;
        in->coord.y += renderSize.h + in->offScreen;
    }

    if(in->pos.y <= -in->offScreen/2)
        isOffScreen = true;

    if(in->pos.x > renderSize.w)
    {
        in->pos.x -= renderSize.w + in->offScreen;
    }
    else if(in->pos.x < -in->offScreen)
    {
       in->pos.x += renderSize.w + in->offScreen;
    }

    return isOffScreen;
}


static void L_leafPosNext(FallLeaf *in, int vx, int vy)
{
    double local = in->angle * PI/2;

    in->coord.y += in->coord.z * vy;
    in->coord.x = (float)cos(local/90)*in->factor + in->xMain;
    in->dispWidth += .01;

    if(in->dispWidth >= (2*PI))
        in->dispWidth = 0.0;

    in->angle = (in->angle+1)%360;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


void L_leafMaxSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    FallLeaf *this = (FallLeaf*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MIN(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }
}


void L_leafMinSize(float src, CEV_Weather* dst, SDL_Rect textureDim)
{
    FallLeaf *this = (FallLeaf*)dst->particles;

    for(int i=0; i< dst->numax; i++)
    {
        this[i].coord.z = MAX(this->coord.z, src);
        this[i].pos = textureDim;
        CEV_rectDimScale(&this[i].pos, this[i].coord.z);
    }

}


static bool L_screenModulo(SDL_Rect *in, SDL_Rect limit, int offScreen)
{
    bool isOffScreen = false;

    if(in->x  > limit.w + offScreen)
    {
        in->x = -offScreen;
        isOffScreen = true;
    }
    else if (in->x < -offScreen)
    {
        in->x = limit.w + offScreen;
        isOffScreen = true;
    }

    if(in->y > limit.h + offScreen)
    {
        in->y = -offScreen;
        isOffScreen = true;
    }
    else if (in->y < -offScreen)
    {
        in->y = limit.h + offScreen;
        isOffScreen = true;
    }

    return isOffScreen;
}



