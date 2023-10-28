//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2022      |   0.9    |    creation    **/
//**   CEV    |  02-2023      |   1.0    |  validation    **/
//**********************************************************/



/// USES EMBEDDED RESSOURCES ONLY UNTIL REFERENCEMENT IS IMPLEMENTED
/// AMASK UNUSED UNTIL IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include <CEV_mixSystem.h>
#include <CEV_aniMini.h>
#include <rwtypes.h>
#include "CEV_platform.h"

    /// LOCAL FUNCTIONS

//moves platforme automaticaly
static void L_platformAutoMove(CEV_Platform* thisPlateform, uint32_t now);

//move platform on floor command
static void L_platformCmdMove(CEV_Platform* thisPlateform, uint32_t now);

//true if dst position is reached
static bool L_platformIsAtPos(CEV_Platform* this);

//platform precalculation
static void L_platformPreCalcul(CEV_Platform* this);

//local implementation of constraint
static bool L_constraint(uint32_t mini, uint32_t* value, uint32_t maxi);


/*
void TEST_platform(void)
{//module stress test

    CEV_Input* input = CEV_inputGet();
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    bool LoadRsc = true;

    CEV_PlatformCst pltCst = {0};
    CEV_PlatformCst *pltCstPtr = NULL;
    CEV_Platform    *platformPtr = NULL;

    CEV_Platform platform[2] ={{0},{0}};
    uint32_t now = CEV_localSyncSet();
    SDL_Rect cameraPos = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};


    if(!LoadRsc)
    {
        CEV_aniMiniLoad("testshortanim.ani", &pltCst.anim);

        if(!pltCst.anim)
            fprintf(stderr, "Err at %s / %d : mini load error.\n", __FUNCTION__, __LINE__ );


        CEV_aniMiniDump(pltCst.anim);
        pltCst.isPxlPerfect = false;
        pltCst.ID = PLTFRM_CST_OBJECT;

        platform[1].cst            = &pltCst;
        platform[1].id             = PLTFRM_INST_OBJECT;
        platform[1].isElevator     = true;
        platform[1].numOfFloors    = 3;
        platform[1].timeSet        = 5000;
        platform[1].timePause      = 0;
        platform[1].timeSync       = 0;
        platform[1].posIndexReq    = 0;
        platform[1].floorPos[0] = (SDL_Point){35, 100};
        platform[1].floorPos[1] = (SDL_Point){35, 200};
        platform[1].floorPos[2] = (SDL_Point){135, 200};
        platform[1].floorPos[3] = (SDL_Point){100, 200};
        platform[1].floorPos[4] = (SDL_Point){100, 100};

        CEV_aniMiniClear(&platform[1].anim);
        platform[1].anim = CEV_aniMiniCreateFrom(pltCst.anim);
        CEV_spriteMiniDump(&platform[1].anim);
    }
    else
    {
        pltCstPtr = CEV_platformCstLoad("platformCst.obj");

        if(!pltCstPtr)
            puts("err chargement constantes");
        else
            CEV_aniMiniDump(pltCstPtr->anim);

        platformPtr = CEV_platformLoad("platformInst.obj");

        if(!platformPtr)
            puts("err chargement instance");

        platformPtr->cst = pltCstPtr;
        platform[1] = *platformPtr;
        platform[1].anim = CEV_aniMiniCreateFrom(pltCstPtr->anim);
    }

    platform[0].cst = &pltCst;
    platform[0].isElevator = false;
    platform[0].numOfFloors = 4;
    platform[0].timeSet = 20000;
    platform[0].timePause = 1000;
    platform[0].timeSync = 0;
    platform[0].floorPos[0] = (SDL_Point){0, 100};
    platform[0].floorPos[1] = (SDL_Point){0, 200};
    platform[0].floorPos[2] = (SDL_Point){100, 200};
    platform[0].floorPos[3] = (SDL_Point){100, 100};
    platform[0].floorPos[4] = (SDL_Point){150, 150};

    CEV_aniMiniClear(&platform[0].anim);

    if(!LoadRsc)
        platform[0].anim = CEV_aniMiniCreateFrom(pltCst.anim);
    else
        platform[0].anim = CEV_aniMiniCreateFrom(pltCstPtr->anim);

    CEV_spriteMiniDump(&platform[0].anim);


    for(int i=0; i<2; i++)
        CEV_platformPrecalc(&platform[i]);

    CEV_platformDump(&platform[1]);

    while(!input->window.quitApp)
    {
        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_KP_0])
            platform[1].posIndexReq = 0;

        if(input->key[SDL_SCANCODE_KP_1])
            platform[1].posIndexReq = 1;

        if(input->key[SDL_SCANCODE_KP_2])
            platform[1].posIndexReq = 2;

        now = CEV_localSyncGet();

        for(int i=0; i<2; i++)
        {
            CEV_platformUpdate(&platform[i], cameraPos, now);
            //CEV_platformMove(&platform[i], now);
            //CEV_platformeDisplay(&platform[i], cameraPos);
        }

        if(platform[0].isAtPos)
        {
            SDL_Rect lrect = {20, 20, 10, 10};
            SDL_SetRenderDrawColor(render, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawRect(render, &lrect);
            SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
        }


        SDL_RenderPresent(CEV_videoSystemGet()->render);
        SDL_RenderClear(CEV_videoSystemGet()->render);
    }

    if(!LoadRsc)
    {
        CEV_platformCstSave(&pltCst, "platformCst.obj");
        CEV_platformSave(&platform[1], "platformInst.obj");
    }

    CEV_platformCstClear(&pltCst);
}
*/

    /// USER END FUNCTIONS

    /// platform constant resource functions

void CEV_platformCstDump(CEV_PlatformCst* this)
{//dumps structure content



    puts("*** BEGIN CEV_PlatformCst ***");

    if(IS_NULL(this))
    {
        puts("This CEV_PlatformCst is NULL");
        goto end;
    }

    printf("\tID = %08X\n\tpxlperfect = %s\n\tmask at = %p\n\tanim at = %p\n",
            this->id,
            this->isPxlPerfect? "yes" : "no",
            this->mask,
            this->anim);

    printf("\tdisplay is : ");
    CEV_rectDump(this->display);
    printf("\thitBox is : ");
    CEV_rectDump(this->hitBox);

    /*
    puts("Amask content :");
    CEV_amaskDump(this->mask);
    */

    puts("\tanimation content :");
    CEV_aniMiniDump(this->anim);

end:
    puts("*** END CEV_PlatformCst ***");
}


void CEV_platformCstDestroy(CEV_PlatformCst* this)
{//frees platform constant content and itself

    CEV_platformCstClear(this);

    free(this);
}


void CEV_platformCstClear(CEV_PlatformCst* this)
{//frees /clear platform constant content only

    if(IS_NULL(this))
        return;

    if(!this->animId)
    {
        CEV_aniMiniDestroy(this->anim);
    }

    this->anim  = NULL;
    this->animId = 0;

    if(this->isPxlPerfect)
        CEV_amaskDestroy(this->mask);

    this->mask          = NULL;
    this->id            = PLTFRM_CST_OBJECT;
    this->isPxlPerfect  = false;
    this->display       = CLEAR_RECT;
    this->hitBox        = CLEAR_RECT;
}


void CEV_platformCstAttachAnim(CEV_AniMini* src, CEV_PlatformCst* dst)
{//sets animini as effective

    if(!src || !dst)
    {//any NULL argument
        fprintf(stderr, "Err at %s / %d : Received NULL arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(!dst->animId)
        CEV_aniMiniDestroy(dst->anim);//NULL compliant

    CEV_rectDimCopy(src->clip, &dst->display);
    dst->anim   = src;
    dst->animId = src->id;
}

    /// platform instance functions

void CEV_platformDump(CEV_Platform* this)
{//dumps structure content

    puts("*** BEGIN CEV_Platform ***");

    if(IS_NULL(this))
    {
        puts("This CEV_Platform is NULL");
        goto end;
    }


    printf("\tID = %X\n", this->id);

    printf("\tisElevator = %d\n\ttimeSet = %u\n\ttimeSync = %u\n\ttimePause = %u\n",
            this->isElevator,
            this->timeSet,
            this->timeSync,
            this->timePause);

    puts("\tpositions: \n");

    for(unsigned i=0; i< this->numOfFloors; i++)
        printf("\t\tpos %d : %d, %d\n", i, this->floorPos[i].x, this->floorPos[i].y);

    for(unsigned i=0; i< this->numOfFloors; i++)
        printf("\t\tposSync %d : %d\n", i, this->posSync[i]);

    printf("\tplatform constant at %p\n", this->cst);
    puts("\tconstants contains :");

    CEV_platformCstDump(this->cst);

end:
    puts("*** END CEV_Platform ***");
}


void CEV_platformUpdate(CEV_Platform* this, SDL_Rect camera, uint32_t now)
{//full platforme update move + display

    CEV_platformMove(this, now);
    CEV_platformDisplay(this, camera, now);
    //is position reached ?
    this->isAtPos = L_platformIsAtPos(this);
}


void CEV_platformMove(CEV_Platform* thisPlateform, uint32_t now)
{//moves only platform

    if(thisPlateform->isElevator)
        L_platformCmdMove(thisPlateform, now);
    else
        L_platformAutoMove(thisPlateform, now);
}


void CEV_platformDisplay(CEV_Platform* this, SDL_Rect cameraPos, uint32_t now)
{//displays only platform on render

    //is it even on camera ?
    SDL_Rect worldPos = {.x = this->actPos.x,
                        .y = this->actPos.y,
                        .w = this->sprite.clip.w,
                        .h = this->sprite.clip.h};

    if(SDL_HasIntersection(&cameraPos, &worldPos))
    {
        SDL_Rect blitPos = worldPos;

        //calculating camera relative position
        blitPos.x = this->actPos.x - cameraPos.x;
        blitPos.y = this->actPos.y - cameraPos.y;

        SDL_Rect clip = CEV_spriteMiniUpdate(&this->sprite, now);
        SDL_RenderCopy(CEV_videoSystemGet()->render, this->sprite.cst->pic, &clip, &blitPos);
    }
}


void CEV_platformSetCst(CEV_PlatformCst *src, CEV_Platform *dst)
{//linking instance to constants

    if(!src || !dst)
    {
        fprintf(stderr, "Err at %s / %d : Received NULL argument.\n", __FUNCTION__, __LINE__ );
        return;
    }

    dst->cst = src;
    CEV_spriteMiniFrom(src->anim, &dst->sprite);
}


void CEV_platformPrecalc(CEV_Platform* this)
{//platform precalculation

    L_platformPreCalcul(this);
}


bool CEV_platformIsAtPos(CEV_Platform* this)
{//platefrm has reached its destination

    return this->isAtPos;
}


SDL_Rect CEV_platformHitBoxGet(CEV_Platform* this)
{//gets hitbox pos in world

    SDL_Rect result = CLEAR_RECT;

    if(!this->cst->isPxlPerfect)
    {
        result = (SDL_Rect){.x = this->actPos.x + this->cst->hitBox.x,
                            .y = this->actPos.y + this->cst->hitBox.y,
                            .w = this->cst->hitBox.w,
                            .h = this->cst->hitBox.h};
    }
    else
    {
        NULL;
    }

    return result;
}


void CEV_platformDestroy(CEV_Platform *this)
{//frees content and itself

    CEV_platformClear(this);
    free(this);
}


void CEV_platformClear(CEV_Platform *this)
{//frees / clear content only

    if(IS_NULL(this))
        return;

    CEV_spriteMiniClear(&this->sprite);

    this->id            = PLTFRM_INST_OBJECT;
    this->cstId         = 0;
    this->timeSet       = 0;
    this->timeSync      = 0;
    this->timePause     = 0;
    this->numOfFloors   = 0;
    this->timeStart     = 0;
    this->timeStop      = 0;
    this->timeRef       = 0;
    this->distFull      = 0;

    this->posIndexNxt   = 0;
    this->posIndexAct   = 0;
    this->posIndexReq   = 0;

    this->isElevator    = false;
    this->isAtPos       = false;

    this->actPos        = CLEAR_POINT;
    this->vect          = CLEAR_POINT;
    this->cst           = NULL;

    for(int i=0; i<CEV_PLATFORM_MAX_POS; i++)
    {
        this->floorPos[i]   = CLEAR_POINT;
        this->posSync[i]    = 0;
    }
}



    /// FILES RELATED FUNCTIONS
    /// Constant / resources structure

int CEV_platformCstSave(CEV_PlatformCst *src, char* fileName)
{//saves platform constant to file

    int funcSts = FUNC_OK;
    readWriteErr = 0;

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    CEV_platformCstTypeWrite(src, dst);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    fclose(dst);

end:
    return funcSts;
}


int CEV_platformCstTypeWrite(CEV_PlatformCst* src, FILE* dst)
{//saving platform constant into file with its aniMini

    write_u32le(src->id, dst);
    write_u32le(src->animId, dst);
    write_u8(src->isPxlPerfect, dst);

    if(!src->isPxlPerfect)
    {//hit box
        write_u32le(src->hitBox.x, dst);
        write_u32le(src->hitBox.y, dst);
        write_u32le(src->hitBox.w, dst);
        write_u32le(src->hitBox.h, dst);
    }
    else
    {
        // TODO (drx#1#): CEV_pleteformCstTypeWrite : add Amask loading here
        NULL;
    }

    if(src->animId)
        CEV_aniMiniTypeWrite(src->anim, dst);

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
}


CEV_PlatformCst* CEV_platformCstLoad(char* fileName)
{//loads platform constant from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_PlatformCst* result = CEV_platformCstLoad_RW(src, true);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : CEV_platformLoad_RW returned NULL.\n", __FUNCTION__, __LINE__ );

    return result;
}


CEV_PlatformCst* CEV_platformCstLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads platform from rwops

    CEV_PlatformCst* result = calloc(1, sizeof(CEV_PlatformCst));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err;
    }

    if(CEV_platformCstTypeRead_RW(src, result))
    {//on error
        fprintf(stderr, "Err at %s / %d : CEV_platformCstTypeRead_RW returned failure.\n", __FUNCTION__, __LINE__ );
        goto err_1;
    }

    if(freeSrc)
        SDL_RWclose(src);

    return result;

    ///Deconstructing on error

err_1:
    CEV_platformCstDestroy(result);

err:
    if(freeSrc)
        SDL_RWclose(src);

    return NULL;
}


int CEV_platformCstTypeRead_RW(SDL_RWops* src, CEV_PlatformCst* dst)
{//saving platform constant into file with its aniMini

    dst->id = SDL_ReadLE32(src);

    if (!IS_PLTFRM_CST(dst->id))
    {
        fprintf(stderr, "Err at %s / %d : File does not contain platform constant.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->animId        = SDL_ReadLE32(src);
    dst->isPxlPerfect  = SDL_ReadU8(src);

    if(!dst->isPxlPerfect)
    {//hit box
        dst->hitBox.x = SDL_ReadLE32(src);
        dst->hitBox.y = SDL_ReadLE32(src);
        dst->hitBox.w = SDL_ReadLE32(src);
        dst->hitBox.h = SDL_ReadLE32(src);
    }
    else
    {
        // TODO (drx#1#): CEV_pleteformCstTypeRead_RW : add Amask writing here
        NULL;
    }

    CEV_AniMini* anim = NULL;

    if(dst->animId)
    {
        anim = CEV_aniMiniLoad_RW(src, false);

        if(IS_NULL(anim))
        {
            fprintf(stderr, "Err at %s / %d : CEV_aniMiniLoad_RW returned failure.\n", __FUNCTION__, __LINE__ );
            return FUNC_ERR;
        }

        CEV_platformCstAttachAnim(anim, dst);
    }

    return FUNC_OK;
}


    /// Instance structure

int CEV_platformSave(CEV_Platform* src, char* fileName)
{//saves platform insatnce into file

    int funcSts = FUNC_OK;
    readWriteErr = 0;

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    CEV_platformTypeWrite(src, dst);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    fclose(dst);

end:
    return funcSts;
}


int CEV_platformTypeWrite(CEV_Platform* src, FILE* dst)
{//saves platform instance type into file

    int funcSts = FUNC_OK;

    write_u32le(src->id, dst);            //own id
    write_u32le(src->cstId, dst);         //its constant id
    write_u32le(src->timeSet, dst);       //its full travel time
    write_u32le(src->timeSync, dst);      //offset to time 0
    write_u32le(src->timePause, dst);     //pause time
    write_u32le(src->numOfFloors, dst);   //num of positions

    for(unsigned i = 0; i < src->numOfFloors; i++) //for every position
    {
        write_u32le(src->floorPos[i].x, dst);
        write_u32le(src->floorPos[i].y, dst);
    }

    write_u8(src->isElevator, dst); //if is elevator


    if (readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}


CEV_Platform* CEV_platformLoad(char* fileName)
{//loads platform from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_Platform* result = CEV_platformLoad_RW(src, 1);

    if(IS_NULL(result))
        fprintf(stderr, "Err at %s / %d : CEV_platformLoad_RW returned NULL.\n", __FUNCTION__, __LINE__ );


    return result;
}


CEV_Platform* CEV_platformLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads platform from rwops

    CEV_Platform* result = calloc(1, sizeof(CEV_Platform));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    if (CEV_platformTypeRead_RW(src, result))
    {//on error
        CEV_platformDestroy(result);
        result = NULL;
    }

    if(freeSrc)
        SDL_RWclose(src);

    CEV_platformPrecalc(result);

    return result;
}


int CEV_platformTypeRead_RW(SDL_RWops* src, CEV_Platform* dst)
{//platform RWops reading

    dst->id = SDL_ReadLE32(src);

    //checking we have platform instance file here
    if (PLTFRM_INST_OBJECT != (dst->id & 0xFFFF0000))
    {
        fprintf(stderr, "Err at %s / %d : File does not contain platform instance.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->cstId          = SDL_ReadLE32(src);
    dst->timeSet        = SDL_ReadLE32(src);
    dst->timeSync       = SDL_ReadLE32(src);
    dst->timePause      = SDL_ReadLE32(src);
    dst->numOfFloors    = SDL_ReadLE32(src);

    if(dst->numOfFloors > CEV_PLATFORM_MAX_POS)
        fprintf(stderr, "Err at %s / %d : max num of floors reached : max %d available.\n", __FUNCTION__, __LINE__, CEV_PLATFORM_MAX_POS);


    for(unsigned i = 0; (i < dst->numOfFloors) && (i < CEV_PLATFORM_MAX_POS); i++) //for every position
    {
        dst->floorPos[i].x = SDL_ReadLE32(src);
        dst->floorPos[i].y = SDL_ReadLE32(src);
    }

    dst->isElevator = SDL_ReadU8(src);

    return FUNC_OK;
}


/*static int L_platformTypeWrite_RW(CEV_Platform* this, SDL_RWops* dst, bool embedRsc)
{

    int sdlWriteErr = 0;

    //setting rsc id 0 is rsc embedded
    Uint32 id = embedRsc ? 0 : this->cst->id;

    sdlWriteErr += SDL_WriteLE32(dst, this->cst->id);
    sdlWriteErr += SDL_WriteLE32(dst, this->cst->id);

    for(int i = 0; i<this->numOfFloors; i++)
    {
        sdlWriteErr += SDL_WriteLE32(dst, this->floorPos[i].x);
        sdlWriteErr += SDL_WriteLE32(dst, this->floorPos[i].y);
    }

    if(!id)
        sdlWriteErr += L_platformCstTypeWrite_RW(this->cst, dst, embedRsc);

    return (sdlWriteErr)? FUNC_ERR : FUNC_OK;

}
*/


/*static int L_platformCstTypeWrite_RW(CEV_PlatformCst* this, SDL_RWops* dst, bool embedRsc)
{//saving platform constant into virtual file

    int funcSts = FUNC_OK,
        sdlWriteErr = 0;


    sdlWriteErr += SDL_WriteLE32(dst, this->id);
    sdlWriteErr += SDL_WriteU8(dst, this->isPxlPerfect);

    if(!this->isPxlPerfect)
    {//hit box
        sdlWriteErr += SDL_WriteLE32(dst, this->hitBox.x);
        sdlWriteErr += SDL_WriteLE32(dst, this->hitBox.y);
        sdlWriteErr += SDL_WriteLE32(dst, this->hitBox.w);
        sdlWriteErr += SDL_WriteLE32(dst, this->hitBox.h);
    }

    if(embedRsc)
    {
        CEV_Capsule capsule;

        if(!IS_NULL(this->anim))
        {
            //SP_AnimToCapsule(this->anim, &capsule);
            //CEV_capsuleTypeWrite_RW(&capsule, dst);
        }
        else if (!IS_NULL(this->gif))
        {

        }
    }

    return readWriteErr;
}
*/


    /// LOCALS FUNCTIONS
    /// Instance control

static void L_platformAutoMove(CEV_Platform* this, uint32_t now)
{//updates automatic platform position //ok

    SDL_Point here = this->actPos;

    now += this->timeSync;
    now %= this->timeSet;

    if((now > (this->timeStop + this->timePause)) || (now < this->timeRef))
    {//movement + pause finished or modulo done

        if (CEV_addModulo(INC ,&this->posIndexAct, this->numOfFloors))//now moving on following segment
            this->timeStart = 0;//back to 0 if modulo has looped
        else
            this->timeStart = (this->timeStop + this->timePause);//new time start

        this->posIndexNxt = (this->posIndexAct+1)%this->numOfFloors;//now going to following position
        this->timeStop = this->posSync[this->posIndexAct];
    }

    this->timeRef = now;

    L_constraint(this->timeStart, &now, this->timeStop);

    //time based position
    this->actPos.x = (int)CEV_map(now,
                                this->timeStart,
                                this->timeStop,
                                this->floorPos[this->posIndexAct].x,
                                this->floorPos[this->posIndexNxt].x);

    this->actPos.y = (int)CEV_map(now,
                                this->timeStart,
                                this->timeStop,
                                this->floorPos[this->posIndexAct].y,
                                this->floorPos[this->posIndexNxt].y);

    //updating vector
    this->vect.x = this->actPos.x - here.x;
    this->vect.y = this->actPos.y - here.y;
}


static void L_platformCmdMove(CEV_Platform* this, uint32_t now)
{//moves elevator type platforms

    SDL_Point here = this->actPos;

    //requested position reached
    this->posReached.value = this->isAtPos;

    bool dstReached  = CEV_edgeRise(&this->posReached),
         treatNewReq = (this->posIndexAct != this->posIndexReq) && (this->posIndexAct == this->posIndexNxt);

    /*when arriving at Dst or new request*/
    if (dstReached || treatNewReq)
    {
        this->posIndexAct = this->posIndexNxt;

        if(this->posIndexReq < this->posIndexAct)
        {
            this->timeStart = now;
            this->posIndexNxt--;
            this->timeStop = this->posSync[this->posIndexNxt];
        }
        else if (this->posIndexReq > this->posIndexAct)
        {
            this->timeStart = now;
            this->posIndexNxt++;
            this->timeStop = this->posSync[this->posIndexAct];
        }
    }

    uint32_t elapsed = now - this->timeStart;

    CEV_constraint(0, &elapsed, this->timeStop);

    //time based position
    this->actPos.x = (int)CEV_map(elapsed,
                                0,
                                this->timeStop,
                                this->floorPos[this->posIndexAct].x,
                                this->floorPos[this->posIndexNxt].x);

    this->actPos.y = (int)CEV_map(elapsed,
                                0,
                                this->timeStop,
                                this->floorPos[this->posIndexAct].y,
                                this->floorPos[this->posIndexNxt].y);

    //updating vector
    this->vect.x = this->actPos.x - here.x;
    this->vect.y = this->actPos.y - here.y;
}


static void L_platformPreCalcul(CEV_Platform* this)
{//executes precalc

    this->distFull = 0;

    uint32_t pathTravelTime[CEV_PLATFORM_MAX_POS], //as i to i+1 (from 0 to 1)
             pathDist[CEV_PLATFORM_MAX_POS];

    for(unsigned i=0; i<(this->numOfFloors-this->isElevator); i++)
    {//calculating segment length
        int next        = (i+1)%this->numOfFloors;
        pathDist[i]     = (uint32_t)CEV_pointDist(this->floorPos[i], this->floorPos[next]);
        this->distFull += pathDist[i];
    }

    uint32_t totalPause = this->timePause * this->numOfFloors,
             moveTime   = this->timeSet - totalPause;

    for(unsigned i=0; i<(this->numOfFloors-this->isElevator); i++)
    {//calculating time on segment

        double thisTime = ((double)moveTime * pathDist[i] / this->distFull) + 0.5;
        pathTravelTime[i] = (uint32_t)thisTime;
    }

    uint32_t thisSync = 0;

    if(this->isElevator)
    {
        for(unsigned i=0; i<this->numOfFloors-1; i++)
        {
            thisSync += pathTravelTime[i];
            this->posSync[i] = pathTravelTime[i];
            //printf("posync %d = %u\n", i, this->posSync[i]);
        }

        //this->posSync[this->numOfFloors -1] = this->timeSet - thisSync;
    }
    else
    {
        for(unsigned i=0; i<this->numOfFloors; i++)
        {
            thisSync += pathTravelTime[i];
            this->posSync[i] = thisSync;
            thisSync += this->timePause;
        }
        this->posSync[this->numOfFloors -1] = this->timeSet - this->timePause;
    }

    this->timeStart = 0;
    this->timeStop = this->posSync[0];
    this->posIndexAct = 0;
    this->posIndexNxt = this->isElevator ? 0 : 1;
    this->timeRef = 0;
    this->actPos = this->floorPos[0];
}


static bool L_platformIsAtPos(CEV_Platform* this)
{//platefrm has reached its destination
    return (this->actPos.x == this->floorPos[this->posIndexNxt].x) && (this->actPos.y == this->floorPos[this->posIndexNxt].y);
}


static bool L_constraint(uint32_t mini, uint32_t* value, uint32_t maxi)
{/*keeps value within [mini, maxi]*/

    if (*value < mini)
    {
        *value = mini;
        return true;
    }
    else if (*value > maxi)
    {
        *value = maxi;
        return true;
    }

    return false;
}
