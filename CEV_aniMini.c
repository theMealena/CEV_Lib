//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  25-09-2022   |   1.0    |    creation    **/
//**********************************************************/


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <CEV_mixSystem.h>
#include "CEV_aniMini.h"
#include <CEV_api.h>
#include <CEV_dataFile.h>
#include <CEV_types.h>
#include <rwtypes.h>


static void L_shortAnimGetTextureDim(SDL_Texture* src, CEV_AniMiniCst* dst);

void TEST_shortAnim(void)
{
    bool load = true;

    CEV_Input *input = CEV_inputGet();
    CEV_AniMiniCst animCst;

    if(load)
        CEV_aniMiniLoad("testshortanim.ani", &animCst);
    else
    {
        CEV_aniMiniCstClear(&animCst, false);
        SDL_Texture *picAnim = CEV_textureLoad("Sonic_SP.png");
        CEV_aniMiniSetTexture(picAnim, &animCst);
        animCst.delay = 150;
        CEV_aniMiniSetParam(6, 4, &animCst);
    }

    CEV_aniMiniCstDump(&animCst);

    //CEV_aniMini anim[10] ;

    SDL_Renderer *render = CEV_videoSystemGet()->render;
/*
    for(int i=0; i<10; i++)
    {
        CEV_aniMiniClear(&anim[i]);
        anim[i] = CEV_aniMiniCreateFrom(&animCst);
        anim[i].timeOffset = CEV_map(i, 0, 9, 0, 600);
    }*/

    //CEV_aniMiniCstDump(&animCst);
    //CEV_aniMiniDump(&anim);
    CEV_AniMini anim = animCst.anim;

    bool quit = false;

    while(!quit)
    {

        CEV_inputUpdate();

        quit = input->window.quitApp;

        //anim[5].switchAnim = input->key[SDL_SCANCODE_SPACE];
/*
        for(int i=0; i<10; i++)
        {
            SDL_Rect clip = CEV_aniMiniUpdate(&anim[i], SDL_GetTicks());
            SDL_Rect blit = {(animCst.clip.w+10)*i, 350, clip.w, clip.h};
            SDL_RenderCopy(render, animCst.srcPic, &clip, &blit);

        }*/
            SDL_Rect clip = CEV_aniMiniUpdate(&animCst.anim, SDL_GetTicks());
            SDL_Rect blit = {(animCst.clip.w+10)/**i*/, 350, clip.w, clip.h};
            SDL_RenderCopy(render, animCst.srcPic, &clip, &blit);


        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(20);
    }

    if(CEV_aniMiniSave(&animCst, "testshortanim.ani", true))
    {
        puts("save failed !");
    }

    CEV_aniMiniCstClear(&animCst, true);
}


void CEV_aniMiniCstDump(CEV_AniMiniCst* in)
{//dumps structure content

    puts("***STARTS DUMPING SHORT ANIMATION CONSTANTS***");
    printf("\tis at %p\n", in);
    printf("\tID : %X\n", in->ID);
    printf("\tpicture ID : %X\n", in->srcID);
    printf("\tNum of animation : %d\n", in->numOfAnim);

    for(int i=0; i<in->numOfAnim; i++)
        printf("\tanimation %d num of pic : %d\n", i, in->numOfPic[i]);

    printf("\tdelay : %u\n", in->delay);
    printf("\toffset : %d\n", in->timeOffset);

    printf("\tclip is : ");
    CEV_rectDump(in->clip);
    printf("\tpicture dimension is : ");
    CEV_rectDump(in->picDim);

    printf("\ttexture is at %p\n", in->srcPic);
    puts("***ENDS DUMPING SHORT ANIMATION CONSTANTS***");

}


void CEV_aniMiniDump(CEV_AniMini* in)
{//dumps structure content

    puts("***STARTS DUMPING SHORT ANIMATION INSTANCE***");
    printf("\tis playing : %s\n", in->play? "true" : "false");
    printf("\tactive anim : %s\n", in->switchAnim? "true" : "false");
    printf("\tactive picture : %d\n", in->picAct);
    printf("\ttime offset : %d\n", in->timeOffset);
    printf("\tlinked to constants : %p\n", in->cst);
    puts("***ENDS DUMPING SHORT ANIMATION INSTANCE***");
}


void CEV_aniMiniCstClear(CEV_AniMiniCst *dst, bool freePic)
{//clears cst structure content

    dst->numOfAnim  = 0;
    dst->ID         = 0;
    dst->srcID      = 0;
    dst->delay      = 0;
    dst->timeOffset = 0;
    dst->clip       = CLEAR_RECT;
    dst->picDim     = CLEAR_RECT;

    for(int i=0; i<2; i++)
        dst->numOfPic[i] = 0;

    if(freePic)
    {
        SDL_DestroyTexture(dst->srcPic);
        dst->srcPic = NULL;
    }
}


int CEV_aniMiniLoad(char *fileName, CEV_AniMiniCst *dst)
{//loads short animation from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    return CEV_aniMiniLoad_RW(src, dst, true);
}


int CEV_aniMiniLoad_RW(SDL_RWops* src, CEV_AniMiniCst *dst, bool freeSrc)
{//loads animation from virtual file

    int funcSts = FUNC_OK;
    CEV_Capsule picCap = {0};

    dst->ID         = SDL_ReadLE32(src);
    dst->srcID      = SDL_ReadLE32(src);
    dst->delay      = SDL_ReadLE32(src);
    dst->timeOffset = SDL_ReadLE32(src);
    dst->numOfAnim  = SDL_ReadU8(src);

    for(int i=0; i<2; i++)
        dst->numOfPic[i] = SDL_ReadU8(src);

    //if picture is embedded
    if(!dst->srcID)
    {
        CEV_capsuleRead_RW(src, &picCap);

        if(!IS_PIC(picCap.type))
        {
            fprintf(stderr, "Err at %s / %d : Embedded file is not picture.\n", __FUNCTION__, __LINE__ );
            funcSts = FUNC_ERR;
            goto end;
        }

        SDL_Texture *texture = CEV_capsuleExtract(&picCap, true);

        if(IS_NULL(texture))
        {
            fprintf(stderr, "Err at %s / %d : Picture extraction failed.\n", __FUNCTION__, __LINE__ );
            funcSts = FUNC_ERR;
        }

        CEV_aniMiniSetTexture(texture, dst);
    }

    CEV_aniMiniSetParam(dst->numOfPic[0], dst->numOfPic[1], dst);

    dst->anim = CEV_aniMiniCreateFrom(dst);

end:

    CEV_capsuleClear(&picCap);

    if(freeSrc)
        SDL_RWclose(src);

    return funcSts;
}


int CEV_aniMiniSave(CEV_AniMiniCst *src, char *fileName, bool embedPic)
{//save short animation into file

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    CEV_aniMiniCstTypeWrite(src, dst, embedPic);

    fclose(dst);

    return readWriteErr;
}


int CEV_aniMiniCstTypeWrite(CEV_AniMiniCst *src, FILE* dst, bool embedPic)
{//write short animation into file

    readWriteErr = 0;
    CEV_Capsule picCap;

    write_u32le(src->ID, dst);
    write_u32le(embedPic? 0 : src->srcID, dst);
    write_u32le(src->delay, dst);
    write_s32le(src->timeOffset, dst);
    write_u8(src->numOfAnim, dst);
    write_u8(src->numOfPic[0], dst);
    write_u8(src->numOfPic[1], dst);

    if(embedPic && src->srcPic)
    {
        CEV_textureToCapsule(src->srcPic, &picCap);
        CEV_capsuleWrite(&picCap, dst);
    }

    CEV_capsuleClear(&picCap);

    return readWriteErr;
}


int CEV_aniMiniSetTexture(SDL_Texture* src, CEV_AniMiniCst *dst)
{//setting texture as tileset

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL argument provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->picDim = CEV_textureDimGet(src);
    dst->srcPic = src;

    return FUNC_OK;
}


int CEV_aniMiniSetParam(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMiniCst* dst)
{//setting animation parameters

    dst->numOfAnim = picNum_1? 2 : 1;

    dst->numOfPic[0] = picNum_0;
    dst->numOfPic[1] = picNum_1;

    if(dst->srcPic)
    {
        int maxPicNum = MAX(dst->numOfPic[0], dst->numOfPic[1]);
        dst->clip.w = dst->picDim.w / maxPicNum;
        dst->clip.h = dst->picDim.h / dst->numOfAnim;
    }

    return FUNC_OK;
}


CEV_AniMini CEV_aniMiniCreateFrom(CEV_AniMiniCst* src)
{//fills new struct from constants

    CEV_AniMini result;
    result.switchAnim  = false;
    result.play        = true;
    result.cst         = src;
    result.clip        = src->clip;
    result.timeOffset  = src->timeOffset;

    return result;
}


void CEV_aniMiniClear(CEV_AniMini* src)
{//clears / init src content to default

    src->switchAnim = false;
    src->play       = true;
    src->timeOffset = 0;
    src->clip       = CLEAR_RECT;
    src->cst        = NULL;
}


SDL_Rect CEV_aniMiniUpdate(CEV_AniMini* src, uint32_t now)
{//updates src animation clip

    if(IS_NULL(src->cst))
        return CLEAR_RECT;

    if(src->cst->numOfAnim <2)
        src->switchAnim = false;


    uint32_t    cycleTime   = src->cst->numOfPic[src->switchAnim] * src->cst->delay, //time for src loop
                timeInCycle = (now + src->timeOffset) %cycleTime; //time inside loop

    src->picAct = timeInCycle / src->cst->delay;

    if(src->play)
    {
        src->clip.x = src->clip.w * src->picAct;
        src->clip.y = src->clip.h * src->switchAnim;
    }

    return src->clip;
}

