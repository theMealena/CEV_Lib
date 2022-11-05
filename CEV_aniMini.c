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
{//testing / debug

    bool load = true;

    CEV_Input *input = CEV_inputGet();
    CEV_AniMiniCst* animCstPtr;
    CEV_AniMiniCst animCst = {0};

    if(load)
    {
        animCstPtr = CEV_aniMiniLoad("testshortanim.ani");
        animCst = *animCstPtr;
    }
    else
    {
        CEV_aniMiniCstClear(&animCst);
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
    CEV_AniMini anim = {0};

    anim = animCst.anim;

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

    if(CEV_aniMiniCstSave(&animCst, "testshortanim.ani", true))
    {
        puts("save failed !");
    }

    if(load)
        CEV_aniMiniCstDestroy(animCstPtr);
    else
        CEV_aniMiniCstClear(&animCst);
}


void CEV_aniMiniCstDump(CEV_AniMiniCst* in)
{//dumps structure content

    puts("***STARTS DUMPING SHORT ANIMATION CONSTANTS***");
    printf("\tis at %p\n", in);
    printf("\tID : %X\n", in->ID);
    printf("\tressources ID : %X\n", in->srcID);
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
    printf("\tclip is : "); CEV_rectDump(in->clip);
    printf("\tblit is : "); CEV_rectDump(in->blit);
    printf("\tlinked to constants : %p\n", in->cst);
    puts("***ENDS DUMPING SHORT ANIMATION INSTANCE***");
}


void CEV_aniMiniCstDestroy(CEV_AniMiniCst *this)
{//destroys content and itself

    if(IS_NULL(this))
        return;

    CEV_aniMiniCstClear(this);
    free(this);
}

void CEV_aniMiniCstClear(CEV_AniMiniCst *this)
{//clears cst structure content

    if(IS_NULL(this))
        return;

    this->numOfAnim  = 0;
    this->ID         = 0;
    this->srcID      = 0;
    this->delay      = 0;
    this->timeOffset = 0;
    this->clip       = CLEAR_RECT;
    this->picDim     = CLEAR_RECT;

    for(int i=0; i<2; i++)
        this->numOfPic[i] = 0;

    SDL_DestroyTexture(this->srcPic);
    this->srcPic = NULL;

    CEV_aniMiniClear(&this->anim);
}


CEV_AniMiniCst* CEV_aniMiniLoad(char *fileName)
{//loads short animation (cst) from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    if(CEV_aniMiniLoad_RW(src, true))
        fprintf(stderr, "Err at %s / %d : CEV_aniMiniLoad_RW failed.\n", __FUNCTION__, __LINE__ );
}


CEV_AniMiniCst* CEV_aniMiniLoad_RW(SDL_RWops* src,  bool freeSrc)
{//loads animation from virtual file

    CEV_Capsule picCap = {0};

    CEV_AniMiniCst* result = calloc(1, sizeof(CEV_AniMiniCst));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err;
    }

    result->clip       = CLEAR_RECT;
    result->ID         = SDL_ReadLE32(src);
    result->srcID      = SDL_ReadLE32(src);
    result->delay      = SDL_ReadLE32(src);
    result->timeOffset = SDL_ReadLE32(src);
    result->numOfAnim  = SDL_ReadU8(src);

    for(int i=0; i<2; i++)
        result->numOfPic[i] = SDL_ReadU8(src);

    //if picture is embedded
    if(!result->srcID)
    {
        CEV_capsuleRead_RW(src, &picCap);

        if(!IS_PIC(picCap.type))
        {
            fprintf(stderr, "Err at %s / %d : Embedded file is not picture.\n", __FUNCTION__, __LINE__ );
            goto err_1;
        }

        SDL_Texture *texture = CEV_capsuleExtract(&picCap, true);

        if(IS_NULL(texture))
        {
            fprintf(stderr, "Err at %s / %d : Picture extraction failed.\n", __FUNCTION__, __LINE__ );
            goto err_2;
        }

        CEV_aniMiniSetTexture(texture, result);
    }
    else
    {
        NULL;
    }

    CEV_aniMiniSetParam(result->numOfPic[0], result->numOfPic[1], result);

    result->anim = CEV_aniMiniCreateFrom(result);

//end
    CEV_capsuleClear(&picCap);

    if(freeSrc)
        SDL_RWclose(src);

    return result;

//quitting on error
err_2:
    CEV_capsuleClear(&picCap);

err_1:
    free(result);

err:
    if(freeSrc)
        SDL_RWclose(src);

    return NULL;
}


int CEV_aniMiniCstSave(CEV_AniMiniCst *src, char *fileName, bool embedPic)
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


void CEV_aniMiniPicForce(CEV_aniMini* dst, int picIndex, bool switchView)
{//forces pic display
    dst->switchAnim = switchView;
    dst->picAct = (picIndex < dst->cst->numOfPic[switchView])? picIndex : 0;
    dst->play = false;
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


void CEV_aniMiniClear(CEV_AniMini* this)
{//clears / init this content to default

    if(IS_NULL(this))
        return;

    this->switchAnim = false;
    this->play       = true;
    this->timeOffset = 0;
    this->clip       = CLEAR_RECT;
    this->blit       = CLEAR_RECT;
    this->cst        = NULL;
}


SDL_Rect CEV_aniMiniUpdate(CEV_AniMini* src, uint32_t now)
{//updates src animation clip and return clip

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


bool CEV_aniMiniPlay(CEV_AniMini* src, bool play)
{//enables / disable animation
    return (src->play = play);
}
