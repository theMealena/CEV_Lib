//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2022      |   0.9    |    creation    **/
//**   CEV    |  02-2023      |   1.0    |  validation    **/
//**********************************************************/

/**log :

CEV 12/2023 : Editable text file conversion added.

*/

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
#include <CEV_txtParser.h>
#include <CEV_texts.h>
#include <CEV_file.h>
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

//TODO (drx#1#12/12/23): Tester les fonctions sur fichiers editables

void TEST_platform(void)
{//module stress test

    CEV_Input* input = CEV_inputGet();
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    bool convert = false;

    //CEV_PlatformCst *pltCstPtr   = NULL;
    CEV_Platform    *platformPtr = NULL;

    SDL_Rect cameraPos = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    if(convert)
    {
        CEV_aniMiniConvertTxtToData("platform/aniMiniEditable.txt", "platform/testshortanim.ani");
        //CEV_platformCstConvertTxtToData("platform/platformCstEditable.txt", "platform/cst.obj");
        CEV_platformConvertTxtToData("platform/platformInstEditable.txt", "platform/inst.obj");
    }

    /*CEV_AniMini* ani = CEV_aniMiniLoad("platform/testshortanim.ani");

    if(!ani)
        printf("failed to load ani\n");*/

    //pltCstPtr = CEV_platformCstLoad("platform/cst.obj");

//    if(!pltCstPtr)
//        puts("err chargement constantes");
//    else
//    {
//        //CEV_platformCstAttachAnim(ani, pltCstPtr);
//        CEV_aniMiniDump(pltCstPtr->anim);
//    }

    platformPtr = CEV_platformLoad("platform/inst.obj");

    if(!platformPtr)
        puts("err chargement instance");

    //CEV_platformSetCst(pltCstPtr, platformPtr);
    CEV_platformPrecalc(platformPtr);


    while(!input->window.quitApp && !input->key[SDL_SCANCODE_ESCAPE])
    {
        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_KP_0])
            platformPtr->posIndexReq = 0;

        if(input->key[SDL_SCANCODE_KP_1])
            platformPtr->posIndexReq = 1;

        if(input->key[SDL_SCANCODE_KP_2])
            platformPtr->posIndexReq = 2;

        uint32_t now = SDL_GetTicks();//CEV_localSyncGet();

        //CEV_spriteMiniBlit(&platformPtr->sprite, cameraPos,now);

        CEV_platformUpdate(platformPtr, cameraPos, now);
        //CEV_platformMove(platformPtr, now);
        //CEV_platformeDisplay(platformPtr, cameraPos);


        if(platformPtr->isAtPos)
        {
            SDL_Rect lrect = {20, 20, 10, 10};
            SDL_SetRenderDrawColor(render, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawRect(render, &lrect);
            SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
        }


        SDL_RenderPresent(CEV_videoSystemGet()->render);
        SDL_RenderClear(CEV_videoSystemGet()->render);
    }


    //CEV_platformCstDestroy(pltCstPtr);
    CEV_platformDestroy(platformPtr);
}


/// USER END FUNCTIONS


    /// platform instance functions

void CEV_platformDump(CEV_Platform* this)
{//dumps structure content

    puts("*** BEGIN CEV_Platform ***");

    if(IS_NULL(this))
    {
        puts("This CEV_Platform is NULL");
        goto end;
    }


    printf("\tid = %08X\n", this->id);
    printf("\tAnim id = %08X\n", this->srcId);

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

    printf("\tplatform animation at %p\n", this->anim);

    if(NOT_NULL(this->anim))
    {
        puts("\tconstants contains :");
        CEV_aniMiniDump(this->anim);
    }

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


int CEV_platformAnimAttach(CEV_AniMini* src, CEV_Platform* dst)
{//attaches animin to plateform

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Received NULL argument.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    if(!dst->srcId)
        CEV_aniMiniDestroy(dst->anim);

    dst->anim = src;

    CEV_spriteMiniFrom(src, &dst->sprite);
    dst->hitbox = src->clip;
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

//    if(!this->anim->isPxlPerfect)
//    {
        result = (SDL_Rect){.x = this->actPos.x + this->hitbox.x,
                            .y = this->actPos.y + this->hitbox.y,
                            .w = this->hitbox.w,
                            .h = this->hitbox.h};
//    }
//    else
//    {
//        NULL;
//    }

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

    if(IS_NULL(this->srcId))
        CEV_aniMiniDestroy(this->anim);

    CEV_spriteMiniClear(&this->sprite);

    this->id            = PLTFRM_TYPE_ID;
    this->srcId         = 0;
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

    this->actPos        = CLEAR_ICOORD;
    this->vect          = CLEAR_ICOORD;
    this->anim          = NULL;

    for(int i=0; i<CEV_PLATFORM_MAX_POS; i++)
    {
        this->floorPos[i]   = CLEAR_ICOORD;
        this->posSync[i]    = 0;
    }
}



    /// FILES RELATED FUNCTIONS

    /// Instance structure


int CEV_platformSave(CEV_Platform* src, char* fileName)
{//saves platform insatnce into file

    if(IS_NULL(src) || IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

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

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    write_u32le(src->id, dst); //own id

    if(src->srcId)
        src->srcId = (src->srcId & 0x0000FFFF) | ANI_TYPE_ID;

    write_u32le(src->srcId, dst);         //its constant id
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

    //hitbox
    write_u32le(src->hitbox.x, dst);
    write_u32le(src->hitbox.y, dst);
    write_u32le(src->hitbox.w, dst);
    write_u32le(src->hitbox.h, dst);

    if(!src->srcId && src->anim)
    {
        CEV_aniMiniTypeWrite(src->anim, dst);
    }

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

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    CEV_Platform* result = calloc(1, sizeof(CEV_Platform));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    if (CEV_platformTypeRead_RW(src, result, false))
    {//on error
        CEV_platformDestroy(result);
        result = NULL;
    }

    if(freeSrc)
        SDL_RWclose(src);

    CEV_platformPrecalc(result);

    return result;
}


int CEV_platformTypeRead_RW(SDL_RWops* src, CEV_Platform* dst, bool freeSrc)
{//platform RWops reading

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->id = SDL_ReadLE32(src);

    //checking we have platform instance file here
    if (!IS_PLTFRM(dst->id))
    {
        fprintf(stderr, "Err at %s / %d : File does not contain platform instance.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->srcId          = SDL_ReadLE32(src);
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
    dst->hitbox.x   = SDL_ReadLE32(src);
    dst->hitbox.y   = SDL_ReadLE32(src);
    dst->hitbox.w   = SDL_ReadLE32(src);
    dst->hitbox.h   = SDL_ReadLE32(src);

    if(!dst->srcId)
    {
        CEV_aniMiniDestroy(dst->anim);
        dst->anim = CEV_aniMiniLoad_RW(src, false);
        CEV_spriteMiniFrom(dst->anim, &dst->sprite);
    }

    if(freeSrc)
        SDL_RWclose(src);

    return FUNC_OK;
}


int CEV_platformConvertTxtToData(const char* srcName, const char* dstName)
{//converts editable into data file

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    uint32_t num = 0; //read value
    double buffer[4]; //multiple parameters line buffer
    char parName[25]; //parameter name when used

    CEV_Text *src   = NULL;
    FILE *dst       = NULL;

    char folderName[FILENAME_MAX] = "\0",
         //fileLine[50],
         hasFolder =  0;

    readWriteErr = 0;
    hasFolder = CEV_fileFolderNameGet(srcName, folderName);

    //loading as CEV_Text to enable quick parsing
    src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : Unable to open file %s.\n ",__FUNCTION__,  __LINE__, srcName);
        return FUNC_ERR;
    }

    //opening destination file
    dst = fopen(dstName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : Unable to create file %s.\n ",__FUNCTION__,  __LINE__, dstName);
        goto err;
    }

    //id
    num = (uint32_t)CEV_txtParseValueFrom(src, "id");
    num = (num & 0x0000FFFF) | PLTFRM_TYPE_ID;
    write_u32le(num, dst);

    //resource id
    num = (uint32_t)CEV_txtParseValueFrom(src, "srcId");
    if(num)
        num = (num & 0x0000FFFF) | ANI_TYPE_ID;
    write_u32le(num, dst);

    num = (uint32_t)CEV_txtParseValueFrom(src, "timeSet");
    write_u32le(num, dst);

    num = (uint32_t)CEV_txtParseValueFrom(src, "timeSync");
    write_u32le(num, dst);

    num = (uint32_t)CEV_txtParseValueFrom(src, "timePause");
    write_u32le(num, dst);

    num = (uint32_t)CEV_txtParseValueFrom(src, "numOfFloor");
    write_u32le(num, dst);

    for(int i=0; i<num; i++)
    {
        sprintf(parName, "[%d]position", i);
        CEV_txtParseValueArrayFrom(src, parName, buffer, 2);

        for(int j=0; j<2; j++)
            write_u32le((uint32_t)buffer[j], dst);
    }

    num = (uint8_t)CEV_txtParseValueFrom(src, "isElevator");
    write_u8(num, dst);

    //hitbox
    CEV_txtParseValueArrayFrom(src, "hitbox", buffer, 4);
    for(int i=0; i<4; i++)
        write_u32le((uint32_t)buffer[i], dst);

    char* fileName = CEV_txtParseTxtFrom(src, "picture");

    if(NOT_NULL(fileName))
    {
        strcat(folderName, fileName);
        CEV_fileInsert(folderName, dst);
    }

    fclose(dst);

err:
    CEV_textDestroy(src);

    if(readWriteErr)
    {
        fprintf(stderr, "Err at %s / %d : read/write err.\n ",__FUNCTION__,  __LINE__, srcName);
    }

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
}


/*static int L_platformTypeWrite_RW(CEV_Platform* this, SDL_RWops* dst, bool embedRsc)
{

    int sdlWriteErr = 0;

    //setting rsc id 0 is rsc embedded
    Uint32 id = embedRsc ? 0 : this->anim->id;

    sdlWriteErr += SDL_WriteLE32(dst, this->anim->id);
    sdlWriteErr += SDL_WriteLE32(dst, this->anim->id);

    for(int i = 0; i<this->numOfFloors; i++)
    {
        sdlWriteErr += SDL_WriteLE32(dst, this->floorPos[i].x);
        sdlWriteErr += SDL_WriteLE32(dst, this->floorPos[i].y);
    }

    if(!id)
        sdlWriteErr += L_platformCstTypeWrite_RW(this->anim, dst, embedRsc);

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

    CEV_ICoord here = this->actPos;

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

    CEV_ICoord here = this->actPos;

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
        pathDist[i]     = (uint32_t)CEV_icoordDist(this->floorPos[i], this->floorPos[next]);
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
