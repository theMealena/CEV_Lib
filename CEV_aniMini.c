//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  25-09-2022   |   1.0    |    creation    **/
//**********************************************************/


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "CEV_mixSystem.h"
#include "CEV_aniMini.h"
#include "CEV_api.h"
#include "CEV_dataFile.h"
#include "CEV_types.h"
#include "rwtypes.h"
#include "CEV_txtParser.h"



void TEST_shortAnim(void)
{
    bool load = true,
        convert = true;

    CEV_Input *input = CEV_inputGet();
    CEV_AniMini* animCst = calloc(1, sizeof(*animCst));


    if (convert)
    {
        CEV_aniMinConvertTxtToData("aniMini/aniMiniEditable.txt", "aniMini/testshortanim.ani");
    }

    if(load)
        animCst = CEV_aniMiniLoad("aniMini/testshortanim.ani");

    else
    {
        CEV_aniMiniClear(animCst, false);
        SDL_Texture *picAnim = CEV_textureLoad("aniMini/Sonic_SP.png");
        CEV_aniMiniTextureAttach(picAnim, animCst);
        animCst->delay = 150;
        CEV_aniMiniParamSet(6, 1, animCst);
        animCst->sprite = CEV_spriteMiniFrom(animCst);
    }

    CEV_aniMiniDump(animCst);

    //CEV_aniMini anim[10] ;

    SDL_Renderer *render = CEV_videoSystemGet()->render;
/*
    for(int i=0; i<10; i++)
    {
        CEV_aniMiniClear(&anim[i]);
        anim[i] = CEV_aniMiniCreateFrom(&animCst);
        anim[i].timeOffset = CEV_map(i, 0, 9, 0, 600);
    }*/

    //CEV_aniMiniDump(&animCst);
    //CEV_spriteMiniDump(&anim);
    CEV_SpriteMini* anim = &animCst->sprite;

    bool quit = false;

    while(!quit)
    {

        CEV_inputUpdate();

        quit = input->window.quitApp;

        anim->switchAnim = input->key[SDL_SCANCODE_SPACE];
/*
        for(int i=0; i<10; i++)
        {
            SDL_Rect clip = CEV_aniMiniUpdate(&anim[i], SDL_GetTicks());
            SDL_Rect blit = {(animCst.clip.w+10)*i, 350, clip.w, clip.h};
            SDL_RenderCopy(render, animCst.srcPic, &clip, &blit);

        }*/
            SDL_Rect clip = CEV_spriteMiniUpdate(anim, SDL_GetTicks());
            SDL_Rect blit = {20/**i*/, 350, clip.w, clip.h};
            SDL_RenderCopy(render, animCst->srcPic, &clip, &blit);


        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(20);
    }

    if(CEV_aniMiniSave(animCst, "testshortanim.ani", true))
    {
        puts("save failed !");
    }

    CEV_aniMiniDestroy(animCst);
}


void CEV_aniMiniDump(CEV_AniMini* in)
{//dumps structure content

	if(IS_NULL(in))
	{
		printf("This AniMini is NULL\n");
        return;
	}

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
	printf("\town sprite's content is : \n");

    CEV_spriteMiniDump(&in->sprite);

    puts("***ENDS DUMPING SHORT ANIMATION CONSTANTS***");

}


void CEV_spriteMiniDump(CEV_SpriteMini* in)
{//dumps structure content

	if(IS_NULL(in))
	{
		printf("This SpriteMini is NULL\n");
        return;
	}

    puts("***STARTS DUMPING SHORT ANIMATION INSTANCE***");
    printf("\tis playing : %s\n", in->play? "true" : "false");
    printf("\tactive anim : %s\n", in->switchAnim? "true" : "false");
    printf("\tactive picture : %d\n", in->picAct);
    printf("\ttime offset : %d\n", in->timeOffset);
    printf("\tlinked to constants : %p\n", in->cst);
    puts("***ENDS DUMPING SHORT ANIMATION INSTANCE***");
}


void CEV_aniMiniDestroy(CEV_AniMini *src)
{//destroys aniMini and content

	if(IS_NULL(src))
        return;

    CEV_aniMiniClear(src, !src->srcID);

    free(src);
}


void CEV_aniMiniClear(CEV_AniMini *dst, bool freePic)
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


/*
CEV_AniMini* CEV_aniMiniLoadCreate(char *fileName)
{//loads an and alloc aniMini from file

    CEV_AniMini*result = malloc(sizeof(CEV_AniMini));

    if (IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    if(CEV_aniMiniLoad(fileName, result) != FUNC_OK)
    {
        fprintf(stderr, "Err at %s / %d : Unable to load %s.\n", __FUNCTION__, __LINE__, fileName);
        free(result);
        result = NULL;
    }

    return result;
}
*/


CEV_AniMini* CEV_aniMiniLoad(char *fileName)
{//loads short animation from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    return CEV_aniMiniLoad_RW(src, true);
}


CEV_AniMini CEV_aniMiniRead_RW(SDL_RWops* src)
{//reads aniMini from RWops

    CEV_AniMini result = {0},
                *temp  = NULL;

    temp = CEV_aniMiniLoad_RW(src, false);
    result = *temp;

    CEV_aniMiniDestroy(temp);// TODO (drx#1#): ne fonctionne pas si la texture est détruite ?

    return result;
}


CEV_AniMini* CEV_aniMiniLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads animation from virtual file

    CEV_AniMini* result = calloc(1, sizeof(CEV_AniMini));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    CEV_Capsule picCap = {0};

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
        CEV_capsuleTypeRead_RW(src, &picCap);

        if(!IS_PIC(picCap.type))
        {
            fprintf(stderr, "Err at %s / %d : Embedded file is not picture.\n", __FUNCTION__, __LINE__ );
            goto end;
        }

        SDL_Texture *texture = CEV_capsuleExtract(&picCap, true);

        if(IS_NULL(texture))
        {
            fprintf(stderr, "Err at %s / %d : Picture extraction failed.\n", __FUNCTION__, __LINE__ );
            goto end;
        }

        CEV_aniMiniTextureAttach(texture, result);
    }

    CEV_aniMiniParamSet(result->numOfPic[0], result->numOfPic[1], result);

    result->sprite = CEV_spriteMiniFrom(result);

end:
    CEV_capsuleClear(&picCap);

err_1:
    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


int CEV_aniMiniSave(CEV_AniMini *src, char *fileName, bool embedPic)
{//save short animation into file

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    CEV_aniMiniTypeWrite(src, dst, embedPic);

    fclose(dst);

    return readWriteErr;
}


int CEV_aniMinConvertTxtToData(char *srcName, char *dstName)
{
    int funcSts = FUNC_OK;
    char lString[FILENAME_MAX];

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_Text *src = CEV_textTxtLoad(srcName);

    CEV_textDump(src);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : Unable to load CEV_Text.\n", __FUNCTION__, __LINE__ );
        return FUNC_ERR;
    }

    FILE* dst = fopen(dstName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    int aniNum = CEV_txtParseValueFrom(src, "aniNum");

    //id
    uint32_t valu32 = (uint32_t)CEV_txtParseValueFrom(src, "id");
    write_u32le(valu32, dst);
    //src id
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "srcId");
    write_u32le(valu32, dst);
    //delay
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "delay");
    write_u32le(valu32, dst);
    //offset
    int32_t vals32 = (int32_t)CEV_txtParseValueFrom(src, "timeOffset");
    write_s32le(vals32, dst);
    //num of anim
    write_u8((uint8_t)aniNum, dst);

    for(int i=0; i<aniNum; i++)
    {
        sprintf(lString, "[%d]picNum", i);
        uint8_t valu8 = (uint8_t)CEV_txtParseValueFrom(src, lString);
        write_u8(valu8, dst);
    }

    char *picFileName = CEV_txtParseTxtFrom(src, "picture");

    if(NOT_NULL(picFileName))
    {//if picture embedded

        //inserting picture
        CEV_fileFolderNameGet(srcName, lString);
        //strcat(lString, "/");
        strcat(lString, picFileName);

        CEV_Capsule pic = {0};

        CEV_capsuleFromFile(&pic, lString);
        CEV_capsuleTypeWrite(&pic, dst);
        CEV_capsuleClear(&pic);
    }

err_1:
    CEV_textDestroy(src);

    fclose(dst);

    return funcSts;
}


int CEV_aniMiniTypeWrite(CEV_AniMini *src, FILE* dst, bool embedPic)
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
        CEV_capsuleTypeWrite(&picCap, dst);
    }

    CEV_capsuleClear(&picCap);

    return readWriteErr;
}


int CEV_aniMiniTextureAttach(SDL_Texture* src, CEV_AniMini *dst)
{//setting texture as tileset

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL argument provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    if(NOT_NULL(dst->srcPic) && !dst->srcID)
    {
        SDL_DestroyTexture(dst->srcPic);
        dst->srcPic = NULL;
    }

    dst->picDim = CEV_textureDimGet(src);
    dst->srcPic = src;

    return FUNC_OK;
}


int CEV_aniMiniParamSet(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMini* dst)
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


CEV_SpriteMini* CEV_spriteMiniCreateFrom(CEV_AniMini* src)
{//alloc and fills struct from constants

    CEV_SpriteMini* result = malloc(sizeof(CEV_SpriteMini));

    if (IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    *result = CEV_spriteMiniFrom(src);

    return result;
}


CEV_SpriteMini CEV_spriteMiniFrom(CEV_AniMini* src)
{//fills new struct from constants

    CEV_SpriteMini result;
    result.switchAnim  = false;
    result.play        = true;
    result.cst         = src;
    result.clip        = src->clip;
    result.timeOffset  = src->timeOffset;

    return result;
}


void CEV_spriteMiniClear(CEV_SpriteMini* src)
{//clears / init src content to default

    src->switchAnim = false;
    src->play       = true;
    src->timeOffset = 0;
    src->clip       = CLEAR_RECT;
    src->cst        = NULL;
}


SDL_Rect CEV_spriteMiniUpdate(CEV_SpriteMini* src, uint32_t now)
{//updates src animation clip and returns clip

    if(IS_NULL(src->cst))
        return CLEAR_RECT;

    if(src->cst->numOfAnim <2)
        src->switchAnim = false;


    uint32_t    cycleTime   = src->cst->numOfPic[src->switchAnim] * src->cst->delay, //time for src loop
                timeInCycle = 0; //time inside loop

    if(cycleTime)
        timeInCycle = (now + src->timeOffset) %cycleTime;

    if(src->cst->delay)
        src->picAct = timeInCycle / src->cst->delay;
    else
        src->picAct = 0;

    if(src->play)
    {
        src->clip.x = src->clip.w * src->picAct;
        src->clip.y = src->clip.h * src->switchAnim;
    }

    return src->clip;
}

