//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  2022/03/06   |   1.0    | rev & test / added to CEV_lib
//**********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include "CEV_mixSystem.h"
#include "CEV_api.h"
#include "rwtypes.h"
#include "CEV_camera.h"

//reads camera from virtual file
static int L_cameraTypeRead_RW(SDL_RWops* src, CEV_Camera *dst);

//write camera into virtual file
static int L_cameraTypeWrite(CEV_Camera *src, FILE *dst);

//reads camera parameter from virtual file
static int L_cameraParamTypeRead_RW(SDL_RWops* src, CEV_CameraParam *dst);

//reads camera parameter from file
static int L_cameraParamTypeRead(FILE* src, CEV_CameraParam *dst);

//write camera into file
static int L_cameraParamTypeWrite(CEV_CameraParam *src, FILE* dst);

//makes camera following the followMe point
static void L_cameraFollow(CEV_CameraParam *in);

//makes camera auto scrolling
static void L_cameraAutoScroll(CEV_CameraParam *in);

//calculate which side to display open field
static void L_cameraDirectionCalc(CEV_CameraParam *in);

//calculate where FollowMe point should be
static int L_cameraFollowPos(CEV_CameraParam *in);

//sets off cam status
static int L_cameraOffScene(CEV_Camera *src);

//calculates where the camera should be
static void L_cameraTheoricPos(CEV_CameraParam *in);


void TEST_camera(void)
{

    SDL_Renderer* render = CEV_videoSystemGet()->render;
    CEV_Input *input = CEV_inputGet();

    CEV_FCoord followThis = CLEAR_FCOORD;
    SDL_Rect constraint   = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    CEV_Camera camera;
    CEV_cameraInit(&camera, &followThis, constraint, 500, CAMERA_THIRD);
    CEV_cameraDimensionSet(&camera, SCREEN_WIDTH /10, SCREEN_HEIGHT /10);

    camera.param[CEV_X].velMax = camera.param[CEV_Y].velMax = 10;
    CEV_cameraDirectionLock(&camera, CAMERA_NONE);
    //CEV_cameraScrollSet(&camera, 1, CAMERA_RIGHT, 2);
    CEV_cameraOpenFieldAuto(&camera, 500, CEV_X);
    bool quit = false;

    while (!quit)
    {
        CEV_inputUpdate();

        quit = input->window.quitApp || input->key[SDL_SCANCODE_ESCAPE];

        followThis = CEV_pointToFcoord(input->mouse.pos);
        CEV_cameraUpdate(&camera);

        SDL_SetRenderDrawColor(render, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render);

        SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(render, &camera.scrollActPos);
        SDL_RenderPresent(render);

        if(input->key[SDL_SCANCODE_SPACE])
        {
            printf("Rect %d, %d, %d, %d\n", camera.scrollActPos.x, camera.scrollActPos.y, camera.scrollActPos.w, camera.scrollActPos.h);
            printf("Follow me : %f.2, %f.2\n", followThis.x, followThis.y);
            CEV_cameraOpenFieldSet(&camera, CAMERA_LEFT);
            input->key[SDL_SCANCODE_SPACE] = false;
        }

        SDL_Delay(100);
    }
}


void CEV_cameraInit(CEV_Camera *in, CEV_FCoord* followPt, SDL_Rect constraint, unsigned int changeTime, CEV_CameraMode mode)
{//Init new camera

    for(int i=0; i<2; i++)
    {//setting axis parameters

        in->param[i].dirAutoScroll = 0;
        in->param[i].autoScrollVel = 0;
        in->param[i].dirLock = 0;
        in->param[i].velMax = 0;
        in->param[i].autoReverse = false;
        in->param[i].mode = mode? mode : CAMERA_THIRD; //avoiding 0
        CEV_timerInit(&in->param[i].timer, changeTime);
    }

    in->param[CEV_X].camDim     = SCREEN_WIDTH;
    in->param[CEV_X].followThis = &followPt->x;
    in->param[CEV_X].dirAct     = CAMERA_POSITIVE;
    in->param[CEV_X].posAct     = &in->scrollActPos.x;


    in->param[CEV_Y].camDim     = SCREEN_HEIGHT;
    in->param[CEV_Y].followThis = &followPt->y;
    in->param[CEV_Y].dirAct     = CAMERA_POSITIVE;
    in->param[CEV_Y].posAct     = &in->scrollActPos.y;

    in->constraint = constraint;

    in->scrollActPos = (SDL_Rect){
                            .x = *in->param[CEV_X].followThis - (in->param[CEV_X].camDim / in->param[CEV_X].mode),
                            .y = *in->param[CEV_Y].followThis - (in->param[CEV_Y].camDim / in->param[CEV_Y].mode),
                            .w = in->param[CEV_X].camDim,
                            .h = in->param[CEV_Y].camDim };

    in->param[CEV_X].posCalc = in->scrollActPos.x;
    in->param[CEV_Y].posCalc = in->scrollActPos.y;

    CEV_rectConstraint(&in->scrollActPos, in->constraint);
}


CEV_Camera *CEV_cameraLoad(char *fileName)
{//Loads camera from file

    CEV_Camera* result = NULL;
    SDL_RWops *rwops = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(rwops))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    result = CEV_cameraLoad_RW(rwops, true);

    if(IS_NULL(rwops))
    {
        fprintf(stderr, "Err at %s / %d : Read write error occured.\n", __FUNCTION__, __LINE__);
    }

    return result;
}


int CEV_cameraSave(CEV_Camera *src, const char* fileName)
{//saves camera into file
    int funcSts = FUNC_OK;

    FILE *dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    if (L_cameraTypeWrite(src, dst) != FUNC_OK)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
    }

    fclose(dst);

    return funcSts;
}


CEV_Camera *CEV_cameraLoad_RW(SDL_RWops *src, bool freeSrc)
{//Loads camera from RWops

    CEV_Camera *result = calloc(1, sizeof(CEV_Camera));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    if (L_cameraTypeRead_RW(src, result) != FUNC_OK);
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err;
    }

    if(freeSrc)
        SDL_RWclose(src);

    return result;

err:
    if(freeSrc)
        SDL_RWclose(src);

    free(result);

    return NULL;

}


int CEV_cameraUpdate(CEV_Camera *in)
{//updates camera position & status

    for(int i=0; i<2; i++)
    {
        //recalculating open field side
        if(in->param[i].autoReverse)
            L_cameraDirectionCalc(&in->param[i]);

        if(in->param[i].dirAutoScroll)
            L_cameraAutoScroll(&in->param[i]);
        else
            L_cameraFollow(&in->param[i]);

        in->param[i].followPrev = *in->param[i].followThis;
    }

    CEV_rectConstraint(&in->scrollActPos, in->constraint);

    return L_cameraOffScene(in);
}


void CEV_cameraReset(CEV_Camera *in)
{//restores camera on follow point

    for(int i=0; i<2; i++)
    {
        L_cameraTheoricPos(&in->param[i]);
        *in->param[i].posAct = in->param[i].posCalc;
    }

    CEV_rectConstraint(&in->scrollActPos, in->constraint);
}


void CEV_cameraDimensionSet(CEV_Camera *src, int w, int h)
{//sets camera width / height

    if(w>0)
        src->param[CEV_X].camDim = src->scrollActPos.w = w;
    else
        src->param[CEV_X].camDim = src->scrollActPos.w = SCREEN_WIDTH;


    if(h>0)
        src->param[CEV_Y].camDim = src->scrollActPos.h = h;
    else
        src->param[CEV_Y].camDim = src->scrollActPos.h = SCREEN_HEIGHT;

}


void CEV_cameraOpenFieldAuto(CEV_Camera *in, unsigned int time, int axis)
{//enables automatic open field management

    bool thisAxis = axis;

    in->param[thisAxis].autoReverse = true;
    in->param[thisAxis].timer.preset = time;
}


void CEV_cameraOpenFieldSet(CEV_Camera *in, int direction)
{//Sets camera open field direction

    int x = direction & (CAMERA_LEFT | CAMERA_RIGHT),
        y = (direction & (CAMERA_UP | CAMERA_DOWN))>>2;

    if (x)
    {
        in->param[CEV_X].dirAct = direction;
        in->param[CEV_X].autoReverse = false;
    }

    if (y)
    {
        in->param[CEV_Y].dirAct = direction;
        in->param[CEV_Y].autoReverse = false;
    }
}


void CEV_cameraDirectionLock(CEV_Camera *in, int direction)
{//Locks camera movement to direction

    int x = direction & (CAMERA_LEFT | CAMERA_RIGHT),
        y = (direction & (CAMERA_UP | CAMERA_DOWN))>>2;

    if (x)
    {
        in->param[CEV_X].dirLock = x;
        in->param[CEV_X].autoReverse = false;
    }
    else
        in->param[CEV_X].dirLock = 0;

    if (y)
    {
        in->param[CEV_Y].dirLock = y;
        in->param[CEV_Y].autoReverse = false;
    }
    else
        in->param[CEV_Y].dirLock = 0;

}


void CEV_cameraMaxVelocitySet(CEV_Camera* in, int velMax, int direction)
{//sets camera maximum velocity

    if(direction & (CAMERA_LEFT | CAMERA_RIGHT))
        in->param[CEV_X].velMax = abs(velMax);

    if(direction & (CAMERA_UP | CAMERA_DOWN))
        in->param[CEV_Y].velMax = abs(velMax);
}


void CEV_cameraFollowModeSet(CEV_Camera *in, CEV_CameraMode mode, int axis)
{//Sets camera following mode : half, third, quarter

    bool thisAxis = axis;

    if((mode>=CAMERA_MED) && (mode <=CAMERA_QUART))
        in->param[thisAxis].mode = mode;
}


void CEV_cameraScrollSet(CEV_Camera* in, bool autoScroll, int direction, int velocity)
{//Sets camera auto scroll parameters

    if (direction & (CAMERA_LEFT | CAMERA_RIGHT))
    {
        in->param[CEV_X].dirAutoScroll  = autoScroll? direction : 0;
        in->param[CEV_X].autoScrollVel  = autoScroll? velocity : 0;
        in->param[CEV_X].autoReverse    = !autoScroll;
    }

    if (direction & (CAMERA_UP | CAMERA_DOWN))
    {
        in->param[CEV_Y].dirAutoScroll  = autoScroll? direction>>2 : 0;
        in->param[CEV_Y].autoScrollVel  = autoScroll? velocity : 0;
        in->param[CEV_Y].autoReverse    = !autoScroll;
    }
}


static int L_cameraTypeRead_RW(SDL_RWops* src, CEV_Camera *dst)
{//reads camera type from RWops


    for(int i = 0; i<2; i++)
        L_cameraParamTypeRead_RW(src, &dst->param[i]);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


static int L_cameraTypeRead(FILE* src, CEV_Camera *dst)
{//reads camera type from file

    int funcSts = FUNC_OK;

    for(int i = 0; i<2; i++)
        L_cameraParamTypeRead(src, &dst->param[i]);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


static int L_cameraTypeWrite(CEV_Camera *src, FILE *dst)
{//Writes camera type into file

    for(int i = 0; i<2; i++)
        L_cameraParamTypeWrite(&src->param[i], dst);

    return readWriteErr;
}


static int L_cameraParamTypeRead_RW(SDL_RWops* src, CEV_CameraParam *dst)
{//Reads camera parameters from RWops

    dst->autoReverse        = SDL_ReadU8(src);
    dst->mode               = (CEV_CameraMode)SDL_ReadU8(src);
    dst->autoScrollVel      = SDL_ReadLE32(src);
    dst->dirAct             = (int)SDL_ReadLE32(src);
    dst->dirLock            = (int)SDL_ReadLE32(src);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


static int L_cameraParamTypeRead(FILE* src, CEV_CameraParam *dst)
{//Reads camera parameters from file

    dst->autoReverse        = read_u8(src);
    dst->mode               = (CEV_CameraMode)read_u8(src);
    dst->autoScrollVel      = read_s32le(src);
    dst->dirAct             = (int)read_s32le(src);
    dst->dirLock            = (int)read_s32le(src);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


static int L_cameraParamTypeWrite(CEV_CameraParam *src, FILE* dst)
{//Write camera parameters into file

    readWriteErr = 0;

    write_u8(src->autoReverse, dst);
    write_u8((uint8_t)src->mode, dst);
    write_u8((uint8_t)src->autoScrollVel, dst);
    write_u32le(src->dirAct, dst);
    write_u32le(src->dirLock, dst);

    return readWriteErr;
}


static void L_cameraFollow(CEV_CameraParam *in)
{// single axis following calculations

    //calculating camera position from follow position
    L_cameraTheoricPos(in);

    float pos = *in->posAct;

    if ((!in->dirLock)
        || ((in->dirLock & CAMERA_POSITIVE) && (in->posCalc < *in->posAct))
        || ((in->dirLock & CAMERA_NEGATIVE) && (in->posCalc > *in->posAct))
        )
    {
        CEV_reachValue(&pos, in->posCalc, in->velMax);
        *in->posAct = pos;
    }
}


static void L_cameraAutoScroll(CEV_CameraParam *in)
{//makes camera auto scrolling

    int position = L_cameraFollowPos(in);

    switch(in->dirAutoScroll)
    {
        case CAMERA_NEGATIVE :

            if((position - in->followPrev) > in->autoScrollVel)
            {
                L_cameraFollow(in);
            }
            else
                *in->posAct -= in->autoScrollVel;
        break;

        case CAMERA_POSITIVE :

            if((in->followPrev - position) > in->autoScrollVel)
            {
                L_cameraFollow(in);
                puts("point ahead\n");
            }
            else
            {
                *in->posAct += in->autoScrollVel;
                puts("point behind\n");
            }
        break;

        default :
        break;
    }
}


static void L_cameraDirectionCalc(CEV_CameraParam *in)
{//calculate which side to display open field

    if(*in->followThis > in->followPrev)
        in->dirCalc = CAMERA_POSITIVE;

    else if(*in->followThis < in->followPrev)
        in->dirCalc = CAMERA_NEGATIVE;

    else
        in->dirCalc = CAMERA_NONE;

    if(in->autoReverse && !in->dirLock && !in->dirAutoScroll)
    {
        /* timer update */
        in->timer.run = in->dirCalc && (in->dirCalc != in->dirAct);

        /* direction change when timer done */
        if(CEV_timerTon(&in->timer))
            in->dirAct = in->dirCalc;
    }
    //else
        //in->dirAct = in->dirCalc;
}


static void L_cameraTheoricPos(CEV_CameraParam *in)
{//where should the camera be ?

    if(in->dirAct & CAMERA_POSITIVE) //going right / down
        in->posCalc = *in->followThis - (in->camDim / in->mode);
    else//CAMERA_NEGATIVE //going left / up
        in->posCalc = *in->followThis - (in->camDim - (in->camDim / in->mode));
}


static int L_cameraFollowPos(CEV_CameraParam *in)
{//where should the follow point be ?

    int value = 0;

    if(in->dirAct & CAMERA_POSITIVE) //right / down
        value = *in->posAct + (in->camDim / in->mode);
    else//left / up
        value = *in->posAct + (in->camDim - (in->camDim / in->mode));

    return value;
}


static int L_cameraOffScene(CEV_Camera *in)
{//follow point is off camera value
    int result = 0;

    //returns any off-camera position
    if (*in->param[CEV_X].followThis < in->scrollActPos.x)
        result |= CAMERA_LEFT;
    else if (*in->param[CEV_X].followThis >= in->scrollActPos.x + in->scrollActPos.w)
        result |= CAMERA_RIGHT;
    if (*in->param[CEV_Y].followThis < in->scrollActPos.y)
        result |= CAMERA_UP;
    else if (*in->param[CEV_Y].followThis>= in->scrollActPos.y + in->scrollActPos.h)
        result |= CAMERA_DOWN;

    return result;
}
