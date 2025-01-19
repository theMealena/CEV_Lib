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
#include "CEV_file.h"



void TEST_shortAnim(void)
{

    bool load = true,
        convert = true;

    CEV_Input *input        = CEV_inputGet();
    CEV_AniMini* animation  = NULL;

    if (convert)
    {
        CEV_aniMiniConvertToData("aniMini/aniMiniEditable.txt", "aniMini/testshortanim.ani");
    }

    if(load)
        animation = CEV_aniMiniLoad("aniMini/testshortanim.ani");

    else
    {
        animation               = calloc(1, sizeof(*animation));
        SDL_Texture *picAnim    = CEV_textureLoad("aniMini/Sonic_SP.png");
        CEV_aniMiniAttachTexture(picAnim, animation);
        animation->delay        = 150;
        CEV_aniMiniParamSet(6, 4, animation);
        //CEV_spriteMiniFrom(animation, &animation->sprite);
    }

    //CEV_aniMiniExport(animation, "aniMini/animIniExport.txt");

    CEV_aniMiniDump(animation);

    //CEV_aniMini anim[10] ;

    SDL_Renderer *render = CEV_videoSystemGet()->render;
/*
    for(int i=0; i<10; i++)
    {
        CEV_aniMiniClear(&anim[i]);
        anim[i] = CEV_aniMiniCreateFrom(&animation);
        anim[i].timeOffset = CEV_map(i, 0, 9, 0, 600);
    }*/

    //CEV_aniMiniDump(&animation);
    //CEV_spriteMiniDump(&anim);
    CEV_SpriteMini* sprite[20] = {NULL};

    printf("size of table is %d\n", sizeof(sprite));

    bool play;

    for(unsigned i=0; i<(sizeof(sprite)/sizeof(void*)); i++)
    {
        sprite[i] = CEV_spriteMiniCreateFrom(animation);
        CEV_spriteMiniAttachSwitchCmd(&input->key[SDL_SCANCODE_SPACE], sprite[i]);
        CEV_spriteMiniAttachPlayCmd(&play, sprite[i]);
        sprite[i]->timeOffset = CEV_irand(0, animation->delay*animation->numOfFrame[0]);
    }

    bool quit = false;

    while(!quit)
    {
        uint32_t now = SDL_GetTicks();

        CEV_inputUpdate();

        quit = input->window.quitApp || input->key[SDL_SCANCODE_ESCAPE];
        play = !input->key[SDL_SCANCODE_RETURN];
        //sprite->switchAnim = input->key[SDL_SCANCODE_SPACE];

        for(unsigned i=0; i<(sizeof(sprite)/sizeof(void*)); i++)
        {
            SDL_Rect clip = CEV_spriteMiniUpdate(sprite[i], now);
            SDL_Rect blit = {(animation->clip.w)*i, 350, clip.w, clip.h};
            SDL_RenderCopy(render, animation->pic, &clip, &blit);
        }

        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(20);
    }

    if(CEV_aniMiniSave(animation, "testshortanim.ani"))
    {
        puts("save failed !");
    }

    CEV_aniMiniDestroy(animation);

    for(unsigned i=0; i<(sizeof(sprite)/sizeof(void*)); i++)
        free(sprite[i]);
}


void CEV_aniMiniDump(CEV_AniMini* this)
{//dumps structure content

    puts("*** BEGIN CEV_AniMini  ***");

    if(IS_NULL(this))
	{
		puts("This CEV_AniMini is NULL");
        goto end;
	}
    printf("\tis at %p\n", this);
    printf("\tID : %08X\n", this->id);
    printf("\tpicture id : %08X\n", this->picId);
    printf("\tNum of animation : %d\n", this->numOfAnim);

    for(int i=0; i<this->numOfAnim; i++)
        printf("\tanimation %d num of pic : %d\n", i, this->numOfFrame[i]);

    printf("\tdelay : %u\n", this->delay);
    printf("\toffset : %d\n", this->timeOffset);

    printf("\tclip is : ");
    CEV_rectDump(this->clip);
    printf("\tpicture dimension is : ");
    CEV_rectDump(this->picDim);

    printf("\ttexture is at %p\n", this->pic);
	// #1 printf("\town sprite's content is : \n");

    // #1 CEV_spriteMiniDump(&this->sprite);

end:
    puts("***END CEV_AniMini ***");

}


void CEV_spriteMiniDump(CEV_SpriteMini* this)
{//dumps structure content

    puts("*** BEGIN CEV_SpriteMini ***");

    if(IS_NULL(this))
	{
		puts("This SpriteMini is NULL");
        goto end;
	}

    printf("\tis playing : %s\n", this->play? "true" : "false");
    printf("\tactive anim : %s\n", this->switchAnim? "true" : "false");
    printf("\tactive picture : %d\n", this->frameAct);
    printf("\ttime offset : %d\n", this->timeOffset);
    printf("\tlinked to constants : %p\n", this->cst);

end:
    puts("*** END CEV_SpriteMini ***");
}


void CEV_aniMiniDestroy(CEV_AniMini *src)
{//destroys aniMini and content

	if(IS_NULL(src))
        return;

    CEV_aniMiniClear(src);

    free(src);
}


void CEV_aniMiniClear(CEV_AniMini *dst)
{//clears cst structure content

    if(!dst->picId)
    {
        SDL_DestroyTexture(dst->pic);
    }

    *dst = (CEV_AniMini){0};
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


CEV_AniMini* CEV_aniMiniLoad(const char *fileName)
{//loads short animation from file

    if(IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    FILE* src = fopen(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    CEV_AniMini* result = calloc(1, sizeof(CEV_AniMini));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    if(CEV_aniMiniTypeRead(src, result))
    {
        fprintf(stderr, "Err at %s / %d : Failed to read from file.\n", __FUNCTION__, __LINE__ );
        free(result);
        result = NULL;
    }

end:
    fclose(src);

    return result;

}


CEV_AniMini* CEV_aniMiniLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads animation from virtual file

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    CEV_AniMini* result = calloc(1, sizeof(CEV_AniMini));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    CEV_aniMiniTypeRead_RW(src, result, false);

end:
    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


int CEV_aniMiniSave(CEV_AniMini *src, const char *fileName)
{//save short animation into file

    if(IS_NULL(src) || IS_NULL(fileName))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    CEV_aniMiniTypeWrite(src, dst);

    fclose(dst);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_aniMiniTypeRead(FILE* src, CEV_AniMini* dst)
{//reads aniMini from RWops

    int funcSts = FUNC_OK;
    CEV_Capsule picCap = {0};

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        funcSts = ARG_ERR;
        goto end;
    }

    readWriteErr = 0;

    dst->id         = read_u32le(src);
    dst->picId      = read_u32le(src);
    dst->delay      = read_u32le(src);
    dst->timeOffset = read_u32le(src);
    dst->numOfAnim  = read_u8(src);

    for(int i=0; i<dst->numOfAnim; i++)
        dst->numOfFrame[i] = read_u8(src);

    if(!dst->picId)
    {//if picture is embedded
        CEV_capsuleTypeRead(src, &picCap);

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
            goto end;
        }

        CEV_aniMiniAttachTexture(texture, dst);
    }

    CEV_aniMiniParamSet(dst->numOfFrame[0], dst->numOfFrame[1], dst);

    // #1 CEV_spriteMiniFrom(dst, &dst->sprite);

end:
    CEV_capsuleClear(&picCap);

    return funcSts;
}


int CEV_aniMiniTypeWrite(CEV_AniMini *src, FILE* dst)
{//writes short animation into file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;
    CEV_Capsule picCap;

    readWriteErr = 0;

    //checking consistancy
    if(!src->picId && IS_NULL(src->pic))
    {
        fprintf(stderr, "Err at %s / %d : Animini picture is missing. Attributing default id.\n", __FUNCTION__, __LINE__ );
        src->picId  = CEV_ftypeToId(IS_PNG);
        funcSts     = FUNC_ERR;
    }

    write_u32le(src->id, dst);
    write_u32le(src->picId, dst);
    write_u32le(src->delay, dst);
    write_s32le(src->timeOffset, dst);
    write_u8(src->numOfAnim, dst);
    write_u8(src->numOfFrame[0], dst);
    write_u8(src->numOfFrame[1], dst);

    if(!src->picId && src->pic)
    {
        CEV_textureToCapsule(src->pic, &picCap);
        CEV_capsuleTypeWrite(&picCap, dst);
    }

    CEV_capsuleClear(&picCap);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}


int CEV_aniMiniTypeRead_RW(SDL_RWops* src, CEV_AniMini* dst, bool freeSrc)
{//reads aniMini from RWops

    int funcSts = FUNC_OK;
    CEV_Capsule picCap = {0};

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        funcSts = ARG_ERR;
        goto end;
    }

    dst->id         = SDL_ReadLE32(src);
    dst->picId      = SDL_ReadLE32(src);
    dst->delay      = SDL_ReadLE32(src);
    dst->timeOffset = SDL_ReadLE32(src);
    dst->numOfAnim  = SDL_ReadU8(src);

    for(int i=0; i<dst->numOfAnim; i++)
        dst->numOfFrame[i] = SDL_ReadU8(src);

    if(!dst->picId)
    {//if picture is embedded
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

        CEV_aniMiniAttachTexture(texture, dst);
    }

    CEV_aniMiniParamSet(dst->numOfFrame[0], dst->numOfFrame[1], dst);

    // #1 CEV_spriteMiniFrom(dst, &dst->sprite);

end:
    if(freeSrc)
        SDL_RWclose(src);

    CEV_capsuleClear(&picCap);

    return funcSts;
}


int CEV_aniMiniTypeWrite_RW(CEV_AniMini* src, SDL_RWops* dst)
{//writes into RWops

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;
    CEV_Capsule picCap;

    //readWriteErr = 0;

    //checking consistancy
    if(!src->picId && IS_NULL(src->pic))
    {
        fprintf(stderr, "Err at %s / %d : Animini picture is missing. Attributing default id.\n", __FUNCTION__, __LINE__ );
        src->picId  = CEV_ftypeToId(IS_PNG);
        funcSts     = FUNC_ERR;
    }

    SDL_WriteLE32(dst, src->id);
    SDL_WriteLE32(dst, src->picId);
    SDL_WriteLE32(dst, src->delay);
    SDL_WriteLE32(dst, src->timeOffset);
    SDL_WriteU8(dst, src->numOfAnim);
    SDL_WriteU8(dst, src->numOfFrame[0]);
    SDL_WriteU8(dst, src->numOfFrame[1]);

    if(!src->picId && src->pic)
    {
        CEV_textureToCapsule(src->pic, &picCap);
        CEV_capsuleTypeWrite_RW(&picCap, dst);
    }

    CEV_capsuleClear(&picCap);

    //if(readWriteErr)
        //funcSts = FUNC_ERR;

    return funcSts;
}


int CEV_aniMiniConvertToData(const char *srcName, const char *dstName)
{//converts editable text file into data file

    int funcSts = FUNC_OK;

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_Text *src = CEV_textTxtLoad(srcName);

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

    funcSts = CEV_aniMiniConvertTxtToDataFile(src, dst, srcName);

    fclose(dst);

err_1:
    CEV_textDestroy(src);

    return funcSts;
}


int CEV_aniMiniConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName)
{//writes to data file from CEV_Text.

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    char lString[FILENAME_MAX];

    int aniNum = CEV_txtParseValueFrom(src, "aniNum");

    //id
    uint32_t valu32 = (uint32_t)CEV_txtParseValueFrom(src, "id");
    valu32 = (valu32 & 0x00FFFFFF) | ANI_TYPE_ID;
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

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int CEV_aniMiniExport(CEV_AniMini *src, const char *dstName)
{//exports a editable text file

    if(IS_NULL(src) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    FILE* dst = NULL;

    dst = fopen(dstName, "w");

    if (IS_NULL(dst))
    {
        printf("Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    fprintf(dst, "this = 0x%08X"    , ANI_TYPE_ID);
    fprintf(dst, "aniNum = %d\n"    , src->numOfAnim);
    fprintf(dst, "id = %08X\n"      , src->id);
    fprintf(dst, "srcId = %08X\n"   , src->picId);
    fprintf(dst, "[0]picNum = %d\n" , src->numOfFrame[0]);
    fprintf(dst, "[1]picNum = %d\n" , src->numOfFrame[1]);
    fprintf(dst, "delay = %d\n"     , src->delay);
    fprintf(dst, "timeOffset = %d\n", src->timeOffset);

    if(!src->picId && NOT_NULL(src->pic))
    {
        char folderName[FILENAME_MAX],
             fileName[L_tmpnam+10];

        tmpnam(fileName);
        strcat(fileName, "png");

        fprintf(dst, "picture = %s", fileName+1);

        if(CEV_fileFolderNameGet(dstName, folderName))
        {
            strcat(folderName, fileName+1);//this +1 exits the \ provided by tmpnam
        }

        CEV_textureSavePNG(src->pic, folderName);
    }

    fclose(dst);

    return FUNC_OK;
}


int CEV_aniMiniAttachTexture(SDL_Texture* src, CEV_AniMini *dst)
{//setting texture as tileset

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    if(!dst->picId)
    {
        SDL_DestroyTexture(dst->pic);
        dst->pic = NULL;
    }

    //dst->picId = srcId;
    dst->picDim = CEV_textureDimGet(src);
    dst->pic    = src;

    //updating internal calculations.
    CEV_aniMiniParamSet(dst->numOfFrame[0], dst->numOfFrame[1], dst);
	//updating embedded sprite
    // #1 CEV_spriteMiniFrom(dst, &dst->sprite);

    return FUNC_OK;
}


int CEV_aniMiniParamSet(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMini* dst)
{//setting animation parameters

    dst->numOfAnim = picNum_1? 2 : 1;

    dst->numOfFrame[0] = picNum_0? picNum_0 : 1 ;
    dst->numOfFrame[1] = picNum_1;

    if(dst->pic)
    {
        dst->clip.w = dst->picDim.w / MAX(dst->numOfFrame[0], dst->numOfFrame[1]);
        dst->clip.h = dst->picDim.h / dst->numOfAnim;
    }

    //updating embedded spriteMini with new values
    // #1 CEV_spriteMiniFrom(dst, &dst->sprite);

    return FUNC_OK;
}


CEV_SpriteMini* CEV_spriteMiniCreateFrom(CEV_AniMini* src)
{//alloc and fills struct from constants

    CEV_SpriteMini* result = calloc(1, sizeof(CEV_SpriteMini));

    if (IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    CEV_spriteMiniFrom(src, result);

    return result;
}


void CEV_spriteMiniDestroy(CEV_SpriteMini* this)
{//destroys instance

    free(this);
}


int CEV_spriteMiniFrom(CEV_AniMini* src, CEV_SpriteMini* dst)
{//fills new struct from constants

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL Arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    CEV_spriteMiniClear(dst);
    dst->cst        = src;
    dst->clip       = src->clip;
    dst->timeOffset = src->timeOffset;

    return FUNC_OK;
}


void CEV_spriteMiniClear(CEV_SpriteMini* src)
{//clears / init src content to default

    src->switchPtr  = NULL;
    src->playPtr    = NULL;
    src->switchAnim = false;
    src->play       = true;
    src->timeOffset = 0;
    src->clip       = CLEAR_RECT;
    src->cst        = NULL;
}


SDL_Rect CEV_spriteMiniUpdate(CEV_SpriteMini* this, uint32_t now)
{//updates this animation clip and returns clip

    //checking args
    if(IS_NULL(this) || IS_NULL(this->cst))
        return CLEAR_RECT;

    //external cmds
    if(NOT_NULL(this->switchPtr))
        this->switchAnim = *(this->switchPtr);

    if(NOT_NULL(this->playPtr))
        this->play = *(this->playPtr);

    //consistancy
    if(this->cst->numOfAnim <2)
        this->switchAnim = false;

    uint32_t    cycleTime   = this->cst->numOfFrame[this->switchAnim] * this->cst->delay, //time for this loop
                timeInCycle = 0; //time inside loop

    if(cycleTime)//avoiding div by 0
        timeInCycle = (now + this->timeOffset) %cycleTime;

    if(this->cst->delay && this->play)//avoiding div by 0
        this->frameAct = timeInCycle / this->cst->delay;
    else
        this->frameAct = 0;

    if(this->play)
    {
        this->clip.x = this->clip.w * this->frameAct;
        this->clip.y = this->clip.h * this->switchAnim;
    }

    return this->clip;
}


void CEV_spriteMiniBlit(CEV_SpriteMini* this, SDL_Rect dstRect, uint32_t now)
{//updates / blits miniSprite

    SDL_Renderer* render = CEV_videoSystemGet()->render;

    SDL_Rect blit   = dstRect,
             clip   = CEV_spriteMiniUpdate(this, now);

    CEV_rectDimCopy(clip, &blit);

    SDL_RenderCopy(render, this->cst->pic, &clip, &blit);
}


void CEV_spriteMiniPlay(CEV_SpriteMini* this)
{//plays sprite

    this->play = true;
}


void CEV_spriteMiniStop(CEV_SpriteMini* this)
{//stops sprite

    this->play = false;
}


void CEV_spriteMiniAttachPlayCmd(bool* playCmd, CEV_SpriteMini* dst)
{//attaches external play command

    dst->playPtr = playCmd;
}


void CEV_spriteMiniAttachSwitchCmd(bool* switchCmd, CEV_SpriteMini* dst)
{//attaches external switching command

    dst->switchPtr = switchCmd;
}

