//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2022      |   0.9    |    creation    **/
//**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include <CEV_mixSystem.h>
#include <CEV_aniMini.h>
#include <CEV_txtParser.h>
#include <CEV_types.h>
#include <CEV_file.h>
#include <rwtypes.h>
#include "CEV_button.h"
#include "CEV_platform.h"


/*** locals **/

/** \brief sets values to preset // starts animation
 *
 * \param this : CEV_SwitchButton* to setup
 *
 * \return void
 */
static void L_switchButtonPreset(CEV_SwitchButton* this);


/** \brief updates structure content
 *
 * \param this  : CEV_SwitchButton* to anim
 *
 * \return void
 */
static void L_switchButtonAnim(CEV_SwitchButton* this);


/** \brief converts litteral mode into numeric for file saving.
 *
 * \param str : const char* as button mode.
 *
 * \return uint32_t as mode in numeric value or 0 if not found / error.
 */
static uint32_t L_switchButtonModeStrToNum(const char* str);


void TEST_switchButton(void)
{//module test and stress

#include <CEV_dataFile.h>
    CEV_Input *input = CEV_inputGet();
    CEV_inputClear();
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    bool quit = false;

    int test = 0;
    bool btest = false;

    bool createBtn = 1;

    CEV_SwitchButton *button = NULL;

    if(createBtn)
    {
        button = calloc(1, sizeof(CEV_SwitchButton));
        button->id = SWITCHBTN_TYPE_ID +1;
        button->animId = 0;
        button->anim = CEV_aniMiniLoad("animini/testshortanim.ani");
        button->anim->delay = 1000;
        button->pos = (CEV_ICoord){20, 20, 0};
        button->accessMode = SWITCHBTN_CMD_LOGIC;
        CEV_spriteMiniFrom(button->anim, &button->sprite );
        L_switchButtonPreset(button);
        button->value = 5;
        button->isReversible = true;
    }
    else
    {
        CEV_switchButtonConvertToData("bouton/boutonEditable.txt","bouton/testswitchbutton.obj");

        CEV_Capsule capTest = {0};
        CEV_capsuleFromFile(&capTest, "bouton/testswitchbutton.obj");

        button = /*CEV_capsuleExtract(&capTest, true);//*/CEV_switchButtonLoad("bouton/testswitchbutton.obj");

        CEV_capsuleClear(&capTest);
    }

    CEV_switchButtonAttachValue(&test, button);
    CEV_switchButtonAttachBool(&btest, button);

    CEV_switchButtonDump(button);
    CEV_Edge spacebar = CLEAR_EDGE;

    while(!quit)
    {
        CEV_inputUpdate();
        quit = input->window.quitApp;

        spacebar.value = input->key[SDL_SCANCODE_SPACE];
        CEV_edgeUpdate(&spacebar);

        if(spacebar.re)
            button->isActivated ^=1;

        CEV_switchButtonUpdate(button, NULL);
        printf("value = %d\n", test);
        //CEV_switchButtonDump(button);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(20);
    }

    if(createBtn)
        CEV_switchButtonSave(button, "bouton/testswitchbutton.obj");

    CEV_switchButtonDestroy(button);
}


void CEV_switchButtonDump(CEV_SwitchButton* this)
{//dumps content

    char* mode[SWITCHBTN_NUMOF_CMD] = SWITCHBTN_CMD_MODES;

    puts("*** BEGIN CEV_SwitchButton ***");

    if(IS_NULL(this))
    {
        puts("This switch button is NULL");
        goto end;
    }

    printf("\tis at address = %p\n", this);

    printf("\tid is : %08X\n\tdst_Id is: %08X\n\tvalue is: %d\n",
            this->id,
            this->ctrlId,
            this->value);

    printf("\tmode is : %s\n", this->accessMode? mode[this->accessMode-1] : "not set");

    printf("\tlinked bool at: %p\n\tlinked num at: %p\n\tis activated: %u\n\tis reversible: %u\n",
            this->bDst,
            this->nDst,
            this->isActivated,
            this->isReversible);

    printf("\tanim is at: %p\n", this->anim);
    puts("\tanim contains :");
    CEV_aniMiniDump(this->anim);

    printf("\tpos is %d ; %d ; %d\n", this->pos.x, this->pos.y, this->pos.z);
    puts("blit is");
    CEV_rectDump(this->blit);

end:
    puts("****END CEV_SwitchButton****");
}

//creates new instance
CEV_SwitchButton* CEV_switchButtonCreate(void);


void CEV_switchButtonDestroy(CEV_SwitchButton* this)//NULL compliant
{//destroys content and itself

    if(IS_NULL(this))
        return;

    CEV_switchButtonClear(this);
    free(this);
}


void CEV_switchButtonClear(CEV_SwitchButton* this)
{//clears content

    if(IS_NULL(this))
        return;

    if(!this->animId)
    {//if embedded
        CEV_aniMiniDestroy(this->anim);
    }

    if(!this->chunkId)
    {//if embedded
        CEV_waveClose(this->sound);
    }

    *this = (CEV_SwitchButton){0};
}


void CEV_switchButtonAttachBool(bool* src, CEV_SwitchButton* dst)
{//attaches bool value to button

    if(NOT_NULL(src) && NOT_NULL(dst))
        dst->bDst = src;
}


void CEV_switchButtonAttachValue(int* src, CEV_SwitchButton* dst)
{//attaches int value to button

    if(NOT_NULL(src) && NOT_NULL(dst))
        dst->nDst = src;
}


void CEV_switchButtonAttachAnim(CEV_AniMini* src, CEV_SwitchButton* dst)
{//attach animini to button

    if(!src || !dst)
    {//any NULL argument
        fprintf(stderr, "Err at %s / %d : Received NULL arg.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(!dst->animId)//was embedded, otherwise is held somewhere else
        CEV_aniMiniDestroy(dst->anim);//NULL compliant

    CEV_rectDimCopy(src->clip, &dst->blit);
    CEV_spriteMiniFrom(src, &dst->sprite);
    dst->anim 	= src;
	dst->animId = src->id;
}


CEV_SwitchButton* CEV_switchButtonLoad(char* fileName)
{//loads from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    CEV_SwitchButton* result = CEV_switchButtonLoad_RW(src, true);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : CEV_switchButtonLoad_RW returned NULL.\n", __FUNCTION__, __LINE__ );
    }

    return result;
}


CEV_SwitchButton* CEV_switchButtonLoad_RW(SDL_RWops* src, bool freeSrc)//freed weither succeeds or fails
{//loads from RWops

    CEV_SwitchButton* result = calloc(1, sizeof(CEV_SwitchButton));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    if(CEV_switchButtonTypeRead_RW(src, result, false))
    {//on error
        fprintf(stderr, "Err at %s / %d : CEV_switchButtonTypeRead_RW returned failure.\n", __FUNCTION__, __LINE__ );
        goto err;
    }

    if(freeSrc)
        SDL_RWclose(src);

    return result;

err:
    if(freeSrc)
        SDL_RWclose(src);

    CEV_switchButtonDestroy(result);
    return NULL;
}


int CEV_switchButtonSave(CEV_SwitchButton* src, char* fileName)
{//saves to file

    int funcSts = FUNC_OK;
    readWriteErr = 0;

    FILE* dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    CEV_switchButtonTypeWrite(src, dst);

    if (readWriteErr)
    {
        fprintf(stderr, "Err at %s / %d : R/W error occured.\n", __FUNCTION__, __LINE__ );
        funcSts = FUNC_ERR;
    }

    fclose(dst);

end:
    return funcSts;
}


int CEV_switchButtonTypeWrite(CEV_SwitchButton* src, FILE* dst)
{//writes structure to file

    int funcSts = FUNC_OK;

    write_u32le(src->id, dst);      //own id
    write_u32le(src->ctrlId, dst);  //its dst id
    write_u32le(src->animId, dst);  //its animation id
    write_u32le(src->value, dst);   //its value
    write_u32le(src->accessMode, dst);//access mode to target
    write_u32le(src->pos.x, dst);   //x pos in world
    write_u32le(src->pos.y, dst);   //y pos in world
    write_u32le(src->hitBox.x, dst);//relative hitbox.x
    write_u32le(src->hitBox.y, dst);//relative hitbox.y
    write_u32le(src->hitBox.w, dst);//relative hitbox.w
    write_u32le(src->hitBox.h, dst);//relative hitbox.h
    write_u8(src->isReversible, dst); //is reversible

    if(!src->animId)//means to embed animini
    {
        CEV_aniMiniTypeWrite(src->anim, dst);
    }

    if (readWriteErr)
    {//on error
        fprintf(stderr, "Err at %s / %d : R/W error occured.\n", __FUNCTION__, __LINE__ );
        funcSts = FUNC_ERR;
    }

    return funcSts;
}


int CEV_switchButtonTypeRead_RW(SDL_RWops* src, CEV_SwitchButton* dst, bool freeSrc)
{//reads structure from RWops

    //CEV_switchButtonClear(dst);

    dst->id = SDL_ReadLE32(src); //its ID

    if (SWITCHBTN_TYPE_ID != (dst->id & 0xFFFF0000))
    {
        fprintf(stderr, "Err at %s / %d : File does not contain switchButton.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->ctrlId         = SDL_ReadLE32(src);//destination ID
    dst->animId         = SDL_ReadLE32(src);//its animini ID
    dst->value          = SDL_ReadLE32(src);//value
    dst->accessMode     = SDL_ReadLE32(src);//access mode to target
    dst->pos.x          = SDL_ReadLE32(src);//position.x in world
    dst->pos.y          = SDL_ReadLE32(src); //position.y in world
    dst->hitBox.x       = SDL_ReadLE32(src);//relative hitbox.x
    dst->hitBox.y       = SDL_ReadLE32(src);//relative hitbox.y
    dst->hitBox.w       = SDL_ReadLE32(src);//relative hitbox.w
    dst->hitBox.h       = SDL_ReadLE32(src);//relative hitbox.h
    dst->isReversible   = SDL_ReadU8(src);  //reversible

    //checking if animation embedded with animId <> 0
    if(!dst->animId)
    {
        dst->anim = CEV_aniMiniLoad_RW(src, false);

        if(IS_NULL(dst->anim))
        {
            fprintf(stderr, "Err at %s / %d : CEV_aniMiniLoad_RW returned failure.\n", __FUNCTION__, __LINE__ );
            return FUNC_ERR;
        }

        CEV_spriteMiniFrom(dst->anim, &dst->sprite);
		L_switchButtonPreset(dst);
    }

    if (freeSrc)
        SDL_RWclose(src);

    return FUNC_OK;
}


void CEV_switchButtonUpdate(CEV_SwitchButton* this, CEV_Camera* camera)
{//updates status and shows if into camera

    this->blit = (SDL_Rect){.x = this->pos.x,
                            .y = this->pos.y,
                            .w = this->sprite.clip.w,
                            .h = this->sprite.clip.h};

    CEV_switchButtonMove(this);
    L_switchButtonAnim(this);

    if(IS_NULL(camera) || SDL_HasIntersection(&camera->posFromWorld, &this->blit))
    {
        CEV_switchButtonDisplay(this, camera);
    }
}


SDL_Rect CEV_switchButtonHitBoxGet(CEV_SwitchButton* this)
{//hitBox position in world

    return (SDL_Rect){.x = this->pos.x + this->hitBox.x,
                      .y = this->pos.y + this->hitBox.y,
                      .w = this->hitBox.w,
                      .h = this->hitBox.h};

}


void CEV_switchButtonMove(CEV_SwitchButton* this)
{//updates status

    this->reActive.value = this->isActivated;

    CEV_edgeUpdate(&this->reActive);

    //numerical command as plateform call
    if(this->nDst && this->reActive.re)
        *(this->nDst) = this->value;

    //logical command
    if(this->bDst)
    {
        *(this->bDst) = this->isReversible ?
                        this->isActivated :
                        this->reActive.re || *(this->bDst);
    }
}


void CEV_switchButtonDisplay(CEV_SwitchButton* this, CEV_Camera* camera)
{//displays button

    //SDL_Renderer* render = CEV_videoSystemGet()->render;
    int relativeX = 0,
        relativeY = 0;

    if(NOT_NULL(camera))
    {
        relativeX = camera->posFromWorld.x;
        relativeY = camera->posFromWorld.y;
    }

    SDL_Rect blitPos = {this->pos.x - relativeX, this->pos.y - relativeY, this->blit.w, this->blit.h};
    CEV_spriteMiniBlit(&this->sprite, blitPos, SDL_GetTicks());
}


int CEV_switchButtonToCapsule(CEV_SwitchButton* src, CEV_Capsule *dst)
{//converts into capsule


    return 0;
}


int CEV_switchButtonConvertToData(const char* srcName, const char* dstName)
{//converts txt editing file into file

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
        goto err;
    }

    funcSts = CEV_switchButtonConvertTxtToDataFile(src, dst, srcName);

    fclose(dst);

err:
    CEV_textDestroy(src);

    return funcSts;
}


int CEV_switchButtonConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName)
{//Writes to data file from CEV_Text.

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    //id
    uint32_t valu32 = CEV_txtParseValueFrom(src, "id");
    valu32 = (valu32 & 0xFFFF0000) | SWITCHBTN_TYPE_ID;
    write_u32le(valu32, dst);

    //destination id
    write_u32le(CEV_txtParseValueFrom(src, "dstId"), dst);

    //animini id
    uint32_t animId = CEV_txtParseValueFrom(src, "animId");
    write_u32le(animId, dst);

    //destination id
    write_u32le(CEV_txtParseValueFrom(src, "value"), dst);

    write_u32le(L_switchButtonModeStrToNum(CEV_txtParseTxtFrom(src, "mode")), dst);

    double dArray[4];

    //pos x,y
    CEV_txtParseValueArrayFrom(src, "pos", dArray, 2);
    write_u32le((uint32_t)dArray[0], dst);
    write_u32le((uint32_t)dArray[1], dst);

    //hitbox x,y,w,h
    CEV_txtParseValueArrayFrom(src, "hit", dArray, 4);

    for(int i=0; i<4; i++)
        write_u32le((uint32_t)dArray[i], dst);

    //reversible
    write_u8((uint8_t)CEV_txtParseValueFrom(src, "reversible"), dst);

    char *aniMiniName = CEV_txtParseTxtFrom(src, "aniMini");

    if(!animId && NOT_NULL(aniMiniName))
    {
        char folderName[FILENAME_MAX] = "\0";
        CEV_fileFolderNameGet(folderName, srcName);
        strcat(folderName, aniMiniName);
        CEV_fileInsert(folderName, dst);
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


    /****  Local functions  ****/

static void L_switchButtonPreset(CEV_SwitchButton* this)
{//sets values to preset // starts animation

    this->blit = this->anim->clip;
    //CEV_rectDimCopy(this->clip, &this->pos);
    //CEV_spriteMiniPlay(&(this->anim->sprite), true);//playing at start up
    this->aniCtrl.preset = this->anim->numOfFrame[0] * this->anim->delay;
    this->aniCtrl.run = true;
}


static void L_switchButtonAnim(CEV_SwitchButton* this)
{
    if(this->anim->numOfAnim > 1)
    {
        if(this->bDst)
            this->sprite.switchAnim = *this->bDst;
        else if(this->nDst)
            this->sprite.switchAnim = *this->nDst == this->value;
    }

    if(this->reActive.any)
    {
        this->aniCtrl.preset = this->anim->numOfFrame[this->sprite.switchAnim] * this->anim->delay;
    }
    //bool needsUpdate = (this->sprite.picAct != this->anim->numOfPic[this->sprite.toggleAnim]);

    CEV_timerRepeat(&this->aniCtrl);
    //if(needsUpdate)
    if(!this->aniCtrl.cmd)
        CEV_spriteMiniUpdate(&this->sprite, this->aniCtrl.preset - this->aniCtrl.accu);

}


static uint32_t L_switchButtonModeStrToNum(const char* str)
{//mode as text to value conversion


    if(IS_NULL(str))
        return 0;

    char* src[SWITCHBTN_NUMOF_CMD] = SWITCHBTN_CMD_MODES;

    for(int i=0; i<SWITCHBTN_NUMOF_CMD; i++)
    {
        if (!strcmp(str, src[i]))
        {
            return (uint32_t)(i+1);
        }
    }

    return 0;
}
