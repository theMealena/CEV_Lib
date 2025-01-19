//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    02-2023    |   2.0    |    creation    **/
//**   CEV    |    03-2023    |   2.1    | modification   **/
//**********************************************************/

///LOG
//06/04/2017 CEV    : modified free functions to allow NULL as argument causing crash otherwise.
//20/02/2023 CEV    : V2.0
//                      Revision, replaced most "manual operations" by functions provided by CEV_Lib.
//                      added / modified function content & names in CEV_lib standard.
//                      CEV_Camera can be attached for scrolling display
//                      Doxy comments added
//                      tested, stressed and validated
//                      0 warning with C11 (-Wall)
//03/03/2023 CEV    : CEV_weatherConvertToData() : conversion from txt file now uses parsing



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>

#include "rwtypes.h"
#include "CEV_weather.h"
#include "CEV_mixSystem.h"
#include "CEV_file.h"
#include "CEV_dataFile.h"
#include "project_def.h"
#include "CEV_camera.h"
#include "CEV_txtParser.h"

/// locals structures definitions**/


/** \brief Single particle definition.
 */
typedef struct S_CEV_Particle
{
    CEV_FCoord coord,   /**< actual calculated absolute position / z used as deepness of field. */
                vector; /**< actual vector. */

    SDL_Rect pos;       /**< display position scaled by Z coord. */

    int xMain,          /**< main X position of which particle moves COSINUS around. */
        angle;          /**< display rotation angle. */

    float factor,       /**< VX randomly scaled. */
          dispWidth;    /**< display width, used for vertical axis rotation effect. */

    bool disp;          /**< to be displayed or not. */
}
CEV_Particle;


/// local functions

/// constructor

/** \brief inserts picture in file (CEV_weatherConvertToData usage).
 *
 * \param fileName : char* as picture file name to insert.
 * \param dst : FILE* to insert picture into (@actual position).
 *
 * \return void
 */
static void L_weatherTxtPictureTypeWrite(char* fileName, FILE* dst);


/** \brief Converts string like particle type into enum value
 *
 * \param name : char* as particle type string.
 *
 * \return int as appropriate enum, -1 if type unknown.
 */
static int L_weatherNameToType(char* name);


/// snow dedicated functions

/** \brief Fills Particle as snowflake.
 *
 * \param in : Particle* to be filled.
 * \param renderDim : SDL_Rect as render dimensions.
 * \param vx : int as X velocity vector (used for signess).
 *
 * \return void.
 */
static void L_flakeInit(CEV_Particle* in, SDL_Rect renderDim, int vx);


/** \brief Updates snowflake particle status.
 *
 * \param in : Particle* to update.
 * \param vx : int as delta X amplitude around main X position.
 * \param vy : int as Y velocity.
 *
 * \return void.
 *
 *  Is not called if particle does not belong to [min, max].
 */
static void L_flakePosNext(CEV_Particle* in, int vx, int vy);


/// rain dedicated functions

/** \brief Fills Particle as raindrop.
 *
 * \param in : Particle* to be filled.
 * \param renderDim : SDL_Rect as render dimensions.
 * \param angle : int as angle of Vx/Vy.
 *
 * \return void.
 */
static void L_dropInit(CEV_Particle* in, SDL_Rect renderDim, int angle);


/** \brief Updates raindrop particle status.
 *
 * \param in : Particle* to update.
 * \param Vx : int as X velocity vector (used for signess).
 * \param Vy : int as Y velocity vector (used for signess).
 *
 * \return void.
 *
 *  Is not called if particle does not belong to [min, max].
 */
static void L_dropPosNext(CEV_Particle* in, int Vx, int Vy);


/// fall dedicated functions

/** \brief Fills Particle as leaf.
 *
 * \param in : Particle* to be filled.
 * \param renderDim : SDL_Rect as render dimensions.
 * \param vx : int as X velocity vector (used for signess).
 *
 * \return void.
 */
static void L_leafInit(CEV_Particle* in, SDL_Rect renderDim, int vx);


/** \brief Updates leaf particle status.
 *
 * \param in : Particle* to update.
 * \param vx : int as X velocity vector (used for signess).
 * \param vy : int as Y velocity.
 *
 * \return void.
 *
 *  Is not called if particle does not belong to [min, max].
 */
static void L_leafPosNext(CEV_Particle* in, int vx, int vy);


/// common (particle) locals

/** \brief Translates particle's real pos to display pos within renderer
 *
 * \param in : CEV_Particle* to translate.
 * \param renderDim : SDL_Rect as render size.
 * \param offScreen : int as off-screen offset.
 *
 * \return bool : true if particle display pos is off screen.
 */
static bool L_particlePosToDisplay(CEV_Particle* in, SDL_Rect renderDim, int offScreen);


/** \brief Sets Texture related parameters when texture attached
 *
 * \param in : CEV_Particle* to update.
 * \param texutreDim : SDL_Rect as texture's dimension.
 *
 * \return void.
 */
static void L_particleAttachTexture(CEV_Particle* in, SDL_Rect texutreDim);


///module testing and stress
void TEST_weather(void)
{//test / stress / validation

    bool load       = true, //loads file / creates otherwise
         convert    = true; //convert from txt before reloading

    //CEV_RsrcFile resources;
    //CEV_rsrcLoad("dataFile/data.dat", &resources);

    char *picFile = "weather/snowflake2.png";
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    CEV_Input* input = CEV_inputGet();
    CEV_Weather* weather = NULL;
    //CEV_GifAnim* anim;
    CEV_Camera cam = {0};
    cam.constraint = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    cam.posInScreen = cam.posFromWorld = cam.scrollActPos = cam.constraint;

    if(convert)
    {
        CEV_anyConvertToData("weather/weatherEditable.txt", NULL/*"weather/weatherTest.wtr"*/);
    }


    if(!load)
    {
        SDL_Texture *particle = CEV_textureLoad(picFile);
        //anim = CEV_gifAnimLoad("ms0.gif", render);
        //CEV_gifLoopMode(anim, GIF_FORWARD);


        weather = CEV_weatherCreate(WEATHER_FALL, 50, -5, 2);
        CEV_weatherAttachTexture(particle, weather);
    }
    else
    {
        //test from capsule
        /*
        CEV_Capsule caps;
        CEV_capsuleFromFile(&caps, "weatherTest.wtr");

        weather = (CEV_Weather*) CEV_capsuleExtract(&caps, true);
        */

        //direct load
        weather = CEV_weatherLoad("weather/0F000001.wtr");//CEV_weatherFetchById(0x0F000002, &resources);

        CEV_weatherDump(weather, false);
    }

    CEV_weatherAttachCamera(&cam, weather);

    bool quit = false;

    while(!quit)
    {
        CEV_inputUpdate();
        //CEV_gifAnimAuto(anim);

        if(input->key[SDL_SCANCODE_ESCAPE] || input->window.quitApp)
            quit = true;

        if(input->key[SDL_SCANCODE_RETURN])
            CEV_weatherStart(weather);

        if(input->key[SDL_SCANCODE_SPACE])
            CEV_weatherStop(weather);

        if(input->key[SDL_SCANCODE_RIGHT])
            cam.scrollActPos.x+=2;

        if(input->key[SDL_SCANCODE_LEFT])
            cam.scrollActPos.x-=2;

        if(input->key[SDL_SCANCODE_DOWN])
            cam.scrollActPos.y+=2;

        if(input->key[SDL_SCANCODE_UP])
            cam.scrollActPos.y-=2;

        if(input->key[SDL_SCANCODE_LSHIFT])
            CEV_weatherParticleMaxSize(0.6, weather);

        if(input->key[SDL_SCANCODE_RSHIFT])
            CEV_weatherParticleMinSize(0.5, weather);

        if(input->key[SDL_SCANCODE_LALT])
        {
            weather->type = WEATHER_SNOW;
            CEV_weatherParticleBuild(weather);
        }


        CEV_weatherShow(weather);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);
    }

    if(!load)
    {//saving

        CEV_Capsule caps = {0};
        CEV_capsuleFromFile(&caps, picFile);

        CEV_weatherSave(weather, &caps, "weatherTest.wtr");

        CEV_capsuleClear(&caps);
    }

    CEV_weatherDestroy(weather, true);

    //CEV_rsrcClear(&resources);
    //CEV_gifDestroy(anim);
}


/// USER END FUNCTIONS

void CEV_weatherDump(CEV_Weather* this, bool dumpParticles)
{//dumps weather structure content to stdout

    puts("*** BEGIN CEV_Weather ***");

    if(IS_NULL(this))
    {
        puts("This CEV_Weather is NULL");
        goto end;
    }

    printf("run is %s, anyActive is %s\n", this->run?"true":"false", this->anyActive?"true":"false");

    printf("id is : %u\nsrc_Id is: %u\ntype is: %d\n",
            this->id,
            this->picId,
            this->type);

    printf("Vx is %d, Vy is %d\n", this->Vx, this->Vy);


    printf("num is : %u\nnumax is: %u\nangle is %f\noffScreen is %u\n",
            this->num,
            this->numax,
            this->angle,
            this->offScreen);

    printf("texture at: %p\nparticles at: %p\n",
            this->pic,
            this->particles);

    puts("Render size is : ");
        CEV_rectDump(this->renderDim);

    if(this->pic)
    {
        puts("Texture size is : ");
        CEV_rectDump(this->picDim);
    }
    else
    {
        puts("Texture not attached");
    }

    if(this->scrollCorrectionX)
    {
        printf("scrollCorrectionX is: %d at %p\n",
                *this->scrollCorrectionX,
                this->scrollCorrectionX);
    }
    else
    {
        puts("scrollCorrectionX not attached");
    }

    if(this->scrollCorrectionY)
    {
        printf("scrollCorrectionY is: %d at %p\n",
                *this->scrollCorrectionY,
                this->scrollCorrectionY);
    }
    else
    {
        puts("scrollCorrectionY not attached");
    }

    if(dumpParticles)
    {
        for(unsigned i=0; i<this->numax; i++)
        {
            printf("Particle %d holds :\n", i);
            CEV_weatherParticleDump(this->particles +i);
        }
    }

end:
    puts("*** END CEV_Weather ***");

}


void CEV_weatherParticleDump(CEV_Particle* this)
{//dumps particle structure content to stdout

    puts("*** BEGIN CEV_Particle ***");

    if(IS_NULL(this))
    {
        puts("This CEV_Particle is NULL");
        goto end;
    }

    printf("real Pos is : %f %f %f\n",
            this->coord.x,
            this->coord.y,
            this->coord.z);

    printf("vector is : %f %f %f\n",
            this->vector.x,
            this->vector.y,
            this->vector.z);

    puts("blit Pos is :");
    CEV_rectDump(this->pos);

    printf("Main pos : %d\nangle : %d\nfactor : %f\ndisp width : %f\n",
            this->xMain,
            this->angle,
            this->factor,
            this->dispWidth);

    printf("is %s\n", this->disp? "activated" : "deactivated");

end:
    puts("*** END CEV_Particle ***");
}


CEV_Weather* CEV_weatherCreate(uint8_t type, unsigned int num, int vx, int vy)
{//creates instance

    if(type >= WEATHER_NUM)
    {//cheching weather type request
        fprintf(stderr, "Err at %s / %d : invalid weather type, corrected to \"snow\".\n ", __FUNCTION__, __LINE__);
        type = WEATHER_SNOW;
    }

    //allocating result
    CEV_Weather *result = calloc(1, sizeof(*result));

    if(IS_NULL(result))
    {//on error
        fprintf(stderr, "Err at %s / %d : %s.\n ", __FUNCTION__,  __LINE__, strerror(errno));
        goto exit;
    }

    //setting parameters
    SDL_RenderGetLogicalSize(CEV_videoSystemGet()->render,
                              &result->renderDim.w,
                              &result->renderDim.h);

    result->picDim              = CLEAR_RECT;
    result->pic                 = NULL;
    result->type                = type;
    result->num                 = num;
    result->numax               = num;
    result->scrollCorrectionX   = NULL;
    result->scrollCorrectionY   = NULL;
    result->Vx                  = vx;
    result->Vy                  = vy;
    result->run                 = false;
    result->anyActive           = false;
    result->angle               = CEV_vectAngle(vx, vy);
    result->offScreen           = 0;

    //creating particle instance
    result->particles = calloc(num, sizeof(CEV_Particle));

    if(IS_NULL(result->particles))
    {//on error
        fprintf(stderr, "Err at %s / %d : Couldn't create particles instances.\n ",__FUNCTION__,  __LINE__);
        goto err_1;
    }

    CEV_weatherParticleBuild(result);

exit:
    return result;

err_1:
    free(result);

    return NULL;
}


void CEV_weatherDestroy(CEV_Weather* in, bool freePic)
{//frees instance & content

    if(IS_NULL(in))
        return;

    CEV_weatherClear(in, freePic);

    free(in);
}


void CEV_weatherClear(CEV_Weather* in, bool freePic)
{//clears / free content

    if(IS_NULL(in))
        return;

    if(freePic)
    {
        SDL_DestroyTexture(in->pic);
        in->pic = NULL;
    }

    free(in->particles);
    in->particles = NULL;

    *in = (CEV_Weather){0};
}


void CEV_weatherShow(CEV_Weather* in)
{//displays effect

    CEV_weatherShowWithLayer(in, 0.0, 1.0);
}


void CEV_weatherShowWithLayer(CEV_Weather* in, float min, float max)
{//displays only selected sized particles

    void (*partNxtPos[WEATHER_NUM])(CEV_Particle*, int, int) = {L_flakePosNext, L_dropPosNext, L_leafPosNext};

    if(IS_NULL(in))
        return;

    if(IS_NULL(in->pic))
    {
        fprintf(stderr, "Err at %s / %d : no SDL_Texture attached.\n", __FUNCTION__, __LINE__ );
        return;
    }

    CEV_fconstraint(0.0, &min, max);
    CEV_fconstraint(min, &max, 1.0);

    bool LanyActive = false;
    SDL_Renderer *render = CEV_videoSystemGet()->render;

    //if(in->run || in->anyActive)
    //{
        for(unsigned i=0; i<in->num; i++)
        {
            CEV_Particle *Lparticle = in->particles + i;

            if((Lparticle->coord.z >= min) && (Lparticle->coord.z <= max))
            {
                //calculating next position
                partNxtPos[in->type](Lparticle, in->Vx, in->Vy);

                //correcting modulo for scrolling
                Lparticle->pos.x -= (IS_NULL(in->scrollCorrectionX)?
                                        0 :
                                        (int)(*in->scrollCorrectionX * Lparticle->coord.z))
                                            %(in->renderDim.w + in->offScreen);

                Lparticle->pos.y -= (IS_NULL(in->scrollCorrectionY)?
                                        0 :
                                        (int)(*in->scrollCorrectionY * Lparticle->coord.z))
                                            %(in->renderDim.h + in->offScreen);

                //activating particle when offscreen
                if(L_particlePosToDisplay(Lparticle, in->renderDim, in->offScreen))
                {
                    Lparticle->disp = in->run;
                }

                if(Lparticle->disp)
                {
                    SDL_Rect temp = Lparticle->pos;//copied to keep original untouched

                    SDL_RendererFlip flip = SDL_FLIP_NONE;

                    if(in->type == WEATHER_FALL)
                    {
                        temp.w *= (float)cos((double)Lparticle->dispWidth);
                        if(temp.w < 0.0)
                        {
                            temp.w = -temp.w;
                            flip = SDL_FLIP_HORIZONTAL;
                        }
                    }

                    SDL_Point local={temp.w/2, temp.h/2};


                        SDL_RenderCopyEx(render,
                                        in->pic,
                                        NULL,
                                        &temp,
                                        Lparticle->angle,
                                        &local,
                                        flip);

                        LanyActive = true;
                }

            }
        }
    //}

    //in->anyActive = true;// LanyActive;
}


unsigned int CEV_weatherParticleNumSet(CEV_Weather* in, unsigned int num)
{//modify num of particles to display

    if(NOT_NULL(in))
    {
        if(num > in->numax)
            num = in->numax;

        in->num = num;

        return in->num;
    }

    return 0;
}


unsigned int CEV_weatherParticleNumReset(CEV_Weather* in)
{//resets num of particle to max

    if(NOT_NULL(in))
    {
        in->num = in->numax;
        return in->num;
    }

    return 0;
}


void CEV_weatherStart(CEV_Weather* in)
{//starts weather to display

    if(NOT_NULL(in))
        in->run = true;
}


void CEV_weatherStop(CEV_Weather* in)
{//stops weather to display

    if(NOT_NULL(in))
        in->run = false;
}


void CEV_weatherParticleBuild(CEV_Weather* dst)
{//builds / rebuild particles

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL argument.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(IS_NULL(dst->particles))
    {
        fprintf(stderr, "Err at %s / %d : Particles not allocated.\n", __FUNCTION__, __LINE__ );
        return;
    }

    //rain repartition
    int eighty = dst->num *80 / 100,         //eighty % of particles in back ground
        fifteen = dst->num *15 / 100,        //fifteen % in middle
        five = dst->num - eighty - fifteen;  //what's left in foreground


    for(unsigned i=0; i<dst->num; i++)
    {
        switch(dst->type)
        {
            case WEATHER_SNOW :

                L_flakeInit(&dst->particles[i], dst->renderDim, dst->Vx);

            break;

            case WEATHER_RAIN :

                if(i<five)
                    dst->particles[i].coord.z = CEV_frand(0.85, 1.0);
                else if (i<five+fifteen)
                    dst->particles[i].coord.z = CEV_frand(0.3, 0.85);
                else
                    dst->particles[i].coord.z = CEV_frand(0.1, 0.3);

                L_dropInit(&dst->particles[i], dst->renderDim, dst->angle);
                //dst->particles = L_rainCreate(num, dst->renderDim, dst->picDim, vx, vy);
            break;

            case WEATHER_FALL :

                L_leafInit(&dst->particles[i], dst->renderDim, dst->Vx);
                //dst->particles = L_fallCreate(num, dst->renderDim, dst->picDim, vx);
            break;

        }
    }
}


void CEV_weatherParticleMaxSize(float src, CEV_Weather* dst)
{//sets particles maximum size ratio

    for(unsigned i=0; i<dst->numax; i++)
    {
        //limiting display scale
        dst->particles[i].coord.z = MIN(dst->particles[i].coord.z, src);
        //copying original texture dim
        CEV_rectDimCopy(dst->picDim, &dst->particles[i].pos);
        //scaling display dim
        CEV_rectDimScale(&dst->particles[i].pos, dst->particles[i].coord.z);
    }
}


void CEV_weatherParticleMinSize(float src, CEV_Weather* dst)
{//sets particles minimum size ratio

    for(unsigned i=0; i<dst->numax; i++)
    {
        //limiting display scale
        dst->particles[i].coord.z = MAX(dst->particles[i].coord.z, src);
        //copying original texture dim
        CEV_rectDimCopy(dst->picDim, &dst->particles[i].pos);
        //scaling display dim
        CEV_rectDimScale(&dst->particles[i].pos, dst->particles[i].coord.z);
    }
}


size_t CEV_weatherParticleSizeOf(void)
{//returns particle structure size

    return sizeof(CEV_Particle);
}


void CEV_weatherAttachTexture(SDL_Texture* src, CEV_Weather* dst)
{//attaches texture to weather

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }


    //replacing texture by destroying any existing one
    //if(NOT_NULL(dst->pic))
    //{
        //SDL_DestroyTexture(dst->pic);
    //}

    dst->picDim     = CEV_textureDimGet(src);
    dst->pic        = src;
    dst->offScreen  = 2 * CEV_pointDist(CLEAR_POINT,
                                        (SDL_Point){dst->picDim.w, dst->picDim.h});

    for(unsigned i=0; i<dst->numax; i++)
        L_particleAttachTexture(dst->particles +i, dst->picDim);

}


void CEV_weatherAttachCamera(CEV_Camera* src, CEV_Weather* dst)
{//attaches camera

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }

    dst->scrollCorrectionX = &src->scrollActPos.x;
    dst->scrollCorrectionY = &src->scrollActPos.y;
    dst->renderDim         = (SDL_Rect){0, 0, src->scrollActPos.w, src->scrollActPos.h};
}


/// FILE related

CEV_Weather* CEV_weatherLoad(char* fileName)
{// loads weather file

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

    result = CEV_weatherLoad_RW(ops, 1);//frees ops

    return result;
}


CEV_Weather* CEV_weatherLoad_RW(SDL_RWops* src, char freeSrc)
{// loads weather from RWops

    CEV_Weather *result = NULL;
    SDL_Texture *picture = NULL;

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : received NULL src :%s\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    //reading parameters into locals
    uint32_t
            id              = SDL_ReadLE32(src),
            picId           = SDL_ReadLE32(src),
            numOfParticles  = SDL_ReadLE32(src);

    int32_t
            vx = SDL_ReadLE32(src),
            vy = SDL_ReadLE32(src);

    uint8_t type = SDL_ReadU8(src);

    CEV_Capsule capsPict;

    if(!picId)
    {//extracting picture if embedded

        CEV_capsuleTypeRead_RW(src, &capsPict);

        if(!IS_PIC(capsPict.type))
        {
            fprintf(stderr, "Err at %s / %d : Embedded capsule is not picture.\n", __FUNCTION__, __LINE__);
            goto err_1;
        }

        picture = CEV_capsuleExtract(&capsPict, true);

        if IS_NULL(picture)
        {
            fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, IMG_GetError());
            goto end;//capsPict content should be freed at that point
        }
    }

    result = CEV_weatherCreate(type, numOfParticles, vx, vy);

    if(IS_NULL(result))
    {
        fprintf(stderr,"Err at %s / %d : Unable to create weather instance.\n",__FUNCTION__, __LINE__);
        goto err_2;
    }

    CEV_weatherAttachTexture(picture, result);
    result->id      = id;
    result->picId   = picId;

    goto end;

//error exit
err_2:
    SDL_DestroyTexture(picture);

err_1:
    CEV_capsuleClear(&capsPict);

end:
    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


int CEV_weatherSave(CEV_Weather* src, CEV_Capsule* picture, char* fileName)
{//saves CEV_weather into file

    readWriteErr    = 0;

    if(IS_NULL(src) || IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL arg.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : unable to create file : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    CEV_weatherTypeWrite(src, picture, dst);

    return CEV_weatherTypeWrite(src, picture, dst);
}


int CEV_weatherTypeWrite(CEV_Weather* src, CEV_Capsule* picture, FILE* dst)
{//writes CEV_weather into file

    readWriteErr = 0;

    write_u32le(src->id, dst);
    write_u32le(IS_NULL(picture)? 0 : src->picId, dst);
    write_u32le(src->num, dst);
    //write_u32le(src->numax, dst);
    write_s32le(src->Vx, dst);
    write_s32le(src->Vy, dst);
    write_u8(src->type, dst);

    if(NOT_NULL(picture))
        CEV_capsuleTypeWrite(picture, dst);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_weatherConvertToData(char* srcName, char* dstName)
{//creates file from CSV descriptor file

    int funcSts = FUNC_OK;

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_Text* src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load CEV_Text from src file.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    FILE* dst = fopen(dstName,"wb");

    if(IS_NULL(dst))
    {//on error
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err;
    }

    funcSts = CEV_weatherConvertTxtToDataFile(src, dst, srcName);

    fclose(dst);

err:
    CEV_textDestroy(src);

    return funcSts;
}


int CEV_weatherConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName)
{

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    int vx  = 0,
        vy  = 0;

    uint32_t numOfParticles = 0,
             id             = 0,
             srcId          = 0;

    char *type,
        *picName;



    type    = CEV_txtParseTxtFrom(src, "type");
    picName = CEV_txtParseTxtFrom(src, "picture");
    numOfParticles = CEV_txtParseValueFrom(src, "partNum");
    id      = IS_NULL(picName)? CEV_txtParseValueFrom(src, "id") : 0;
    srcId   = CEV_txtParseValueFrom(src, "srcId");
    vx      = CEV_txtParseValueFrom(src, "Xvector");
    vy      = CEV_txtParseValueFrom(src, "Yvector");

    id = (id & 0x00FFFFFF) | WTHR_TYPE_ID;

    write_u32le(id, dst); //id
    write_u32le(srcId , dst); //pic embedded or not
    write_u32le(numOfParticles, dst);
    write_u32le(vx, dst);
    write_u32le(vy, dst);
    write_u8(L_weatherNameToType(type), dst);

    if(NOT_NULL(picName))
    {
        char filePath[FILENAME_MAX];
        CEV_fileFolderNameGet(srcName, filePath);
        strcat(filePath, picName);
        L_weatherTxtPictureTypeWrite(filePath, dst);
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


/**LOCALS**/

/** FILES (CSV) **/

static void L_weatherTxtPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic from CSV file

    CEV_Capsule buffer;

    CEV_capsuleFromFile(&buffer, fileName);

    CEV_capsuleTypeWrite(&buffer, dst);

    CEV_capsuleClear(&buffer);
}


static int L_weatherNameToType(char* name)
{//converts type string to equivalent enum

    if(!strcmp(name, "WEATHER_RAIN"))
        return WEATHER_RAIN;
    else if (!strcmp(name, "WEATHER_SNOW"))
        return WEATHER_SNOW;
    else if (!strcmp(name, "WEATHER_FALL"))
        return WEATHER_FALL;

    return -1;
}


/** SNOW **/

static void L_flakeInit(CEV_Particle* in, SDL_Rect renderDim, int vx)
{//initialize single snowflake particle

    in->factor      = CEV_frand(0.0, (double)vx);           //randomizing amplitude
    in->coord.z     = CEV_frand(0.1, 1.0);                  //randomizing size
    in->xMain       = rand()%renderDim.w;                   //randomizing main X pos
    in->coord.x     = in->xMain;
    in->coord.y     = CEV_frand(0.0, (double)renderDim.h);   //randomizing Y pos
    in->angle       = rand()%360;                           //randomizing rotation pos
    in->disp        = false;                                //starts undisplayed
    //in->pos         = texutreDim;                         //copying texture dimensions
    //CEV_rectDimScale(&in->pos, in->coord.z);              //scaling display rect
    //in->pos.x       = in->coord.x;

}


static void L_flakePosNext(CEV_Particle* in, int vx, int vy)
{//calculates single snowflake next position
    double local = in->angle*PI/2;

    in->coord.y += in->coord.z * vy;

    in->coord.x = (float)cos(local/90) * in->factor + in->xMain; //90 to keep pos/angle modulo
    in->angle = (in->angle+1)%360;

    in->pos.x = in->coord.x;
    in->pos.y = in->coord.y;
}


/**RAIN**/

static void L_dropInit(CEV_Particle* in, SDL_Rect renderDim, int angle)
{//initialize single raindrop particle

        in->coord.y = (float)(rand()%renderDim.h);
        in->coord.x = (float)(rand()%renderDim.w);
        in->disp    = false;
        in->angle   = angle;
        //in->pos = texutreDim;
        //CEV_rectDimScale(&in->pos, in->coord.z);
}


static void L_dropPosNext(CEV_Particle* in, int Vx, int Vy)
{//calculates single drop next position

    in->coord.y += Vy * in->coord.z;
    in->coord.x += Vx * in->coord.z;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


/**FALL**/

static void L_leafInit(CEV_Particle* in, SDL_Rect renderDim, int Vx)
{//initialize single leaf particle

    in->factor      = (float)(rand()%Vx);
    in->coord.z     = CEV_frand(0.3, 1.0);
    in->coord.y     = (float)(rand()%renderDim.h);
    in->xMain       = rand()%renderDim.w;
    in->dispWidth   = (float)CEV_frand(0.0, 2*PI);
    in->angle       = rand()%360;
    in->disp        = false;
    //in->pos         = texutreDim;
    //CEV_rectDimScale(&in->pos, in->coord.z);

}


static void L_leafPosNext(CEV_Particle* in, int Vx, int Vy)
{//calculates single leaf next position

    in->angle = (in->angle+1)%360;

    double local = in->angle * PI/2;

    in->coord.y += in->coord.z * Vy;
    in->coord.x = (float)cos(local/90.0)*in->factor + in->xMain;
    in->dispWidth += .01;

    if(in->dispWidth >= (2*PI))
        in->dispWidth = 0.0;

    in->pos.x = (int)in->coord.x;
    in->pos.y = (int)in->coord.y;
}


/**PARTICLES - common**/

static bool L_particlePosToDisplay(CEV_Particle* in, SDL_Rect renderDim, int offScreen)
{//calculates particle position on display

    bool isOffScreen = false;

    if(in->pos.y > renderDim.h)
    {
        in->pos.y -= renderDim.h + offScreen;
        in->coord.y -= renderDim.h + offScreen;
    }
    else if(in->pos.y <= -offScreen)
    {
        in->pos.y += renderDim.h + offScreen;
        in->coord.y += renderDim.h + offScreen;
    }

    if(in->pos.y <= -offScreen/2)
        isOffScreen = true;

    if(in->pos.x > renderDim.w)
    {
        in->pos.x -= renderDim.w + offScreen;
        in->coord.x -= renderDim.w + offScreen;
    }
    else if(in->pos.x < -offScreen)
    {
       in->pos.x += renderDim.w + offScreen;
       in->coord.x += renderDim.w + offScreen;
    }

    return isOffScreen;
}


static void L_particleAttachTexture(CEV_Particle* in, SDL_Rect texutreDim)
{//sets texture related parameters

    in->pos         = texutreDim;               //copying texture dimensions
    CEV_rectDimScale(&in->pos, in->coord.z);    //scaling display rect
    in->pos.x       = in->coord.x;
}

