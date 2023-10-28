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



void TEST_switchButton(void)
{//module test and stress

#include <CEV_dataFile.h>
    CEV_Input *input = CEV_inputGet();
    CEV_inputClear();
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    bool quit = false;

    int test = 0;

    bool createBtn = 0;

    CEV_SwitchButton *button = NULL;

    if(createBtn)
    {
        button = calloc(1, sizeof(CEV_SwitchButton));
        button->ID = SWITCHBTN_OBJECT;
        button->animID = 0;
        button->anim = CEV_aniMiniLoad("animini/testshortanim.ani");
        CEV_spriteMiniFrom(button->anim, &button->sprite );
        button->anim->delay = 1000;
        button->pos = (SDL_Rect){20, 20, 50, 50};
        L_switchButtonPreset(button);
        button->value = 5;
    }
    else
    {
        CEV_switchButtonConvertTxtToData("bouton/boutonEditable.txt","bouton/testswitchbutton.obj");

        CEV_Capsule capTest = {0};
        CEV_capsuleFromFile(&capTest, "bouton/testswitchbutton.obj");

        button = CEV_capsuleExtract(&capTest, true);//CEV_switchButtonLoad("bouton/testswitchbutton.obj");
        CEV_capsuleClear(&capTest);
    }

    CEV_switchButtonAttachValue(&test, button);

    //CEV_switchButtonDump(button);

    while(!quit)
    {
        CEV_inputUpdate();
        quit = input->window.quitApp;

        if(input->key[SDL_SCANCODE_SPACE])
            button->isActivated = true;

        if(input->key[SDL_SCANCODE_RETURN])
        {
            test = 0;
            button->isActivated = false;
        }

        CEV_switchButtonUpdate(button, (SDL_Rect){0, 0, 1280, 720});
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

    puts("*** BEGIN CEV_SwitchButton ***");

    if(IS_NULL(this))
    {
        puts("This switch button is NULL");
        goto end;
    }

    printf("ID is : %08X\ndst_Id is: %08X\nvalue is: %d\n",
            this->ID,
            this->dstID,
            this->value);

    printf("linked bool at: %p\nlinked num at: %p\n is activated: %u\n is reversible: %u\n",
            this->bDst,
            this->nDst,
            this->isActivated,
            this->isReversible);

    printf("anim is at: %p\n", this->anim);
    puts("anim contains :");
    CEV_aniMiniDump(this->anim);

    puts("pos is");
    CEV_rectDump(this->pos);
    puts("clip is");
    CEV_rectDump(this->clip);

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

    if(!this->animID)
    {
        CEV_aniMiniDestroy(this->anim);
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

    if(!dst->animID)//was embedded, otherwise is held somewhere else
        CEV_aniMiniDestroy(dst->anim);//NULL compliant

    CEV_rectDimCopy(src->clip, &dst->clip);
    CEV_spriteMiniFrom(src, &dst->sprite);
    dst->anim 	= src;
	dst->animID = src->id;
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

    write_u32le(src->ID, dst);      //own ID
    write_u32le(src->dstID, dst);   //its dst ID
    write_u32le(src->animID, dst);  //its animation ID
    write_u32le(src->value, dst);   //its value
    write_u32le(src->pos.x, dst);   //x pos in world
    write_u32le(src->pos.y, dst);   //y pos in world
    write_u32le(src->hitBox.x, dst);//relative hitbox.x
    write_u32le(src->hitBox.y, dst);//relative hitbox.y
    write_u32le(src->hitBox.w, dst);//relative hitbox.w
    write_u32le(src->hitBox.h, dst);//relative hitbox.h

    write_u8(src->isReversible, dst); //is reversible

    if(!src->animID)//means to embed animini
    {
        CEV_aniMiniTypeWrite(src->anim, dst);
    }

    if (readWriteErr)
    {
        fprintf(stderr, "Err at %s / %d : R/W error occured.\n", __FUNCTION__, __LINE__ );
        funcSts = FUNC_ERR;
    }

    return funcSts;
}


int CEV_switchButtonTypeRead_RW(SDL_RWops* src, CEV_SwitchButton* dst, bool freeSrc)
{//reads structure from RWops

    //CEV_switchButtonClear(dst);

    dst->ID = SDL_ReadLE32(src); //its ID

    if (SWITCHBTN_OBJECT != (dst->ID & 0xFFFF0000))
    {
        fprintf(stderr, "Err at %s / %d : File does not contain switchButton.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    dst->dstID          = SDL_ReadLE32(src);//destination ID
    dst->animID         = SDL_ReadLE32(src);//its animini ID
    dst->value          = SDL_ReadLE32(src);//value
    dst->pos.x          = SDL_ReadLE32(src);//position.x in world
    dst->pos.y          = SDL_ReadLE32(src); //position.x in world
    dst->hitBox.x       = SDL_ReadLE32(src);//relative hitbox.x
    dst->hitBox.y       = SDL_ReadLE32(src);//relative hitbox.y
    dst->hitBox.w       = SDL_ReadLE32(src);//relative hitbox.w
    dst->hitBox.h       = SDL_ReadLE32(src);//relative hitbox.h
    dst->isReversible   = SDL_ReadU8(src);  //reversible

    //checking if animation embedded with animID <> 0
    if(!dst->animID)
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


void CEV_switchButtonUpdate(CEV_SwitchButton* this, SDL_Rect camera)
{//updates status and shows if into camera

    CEV_switchButtonMove(this);

    if(SDL_HasIntersection(&camera, &this->pos))
        CEV_switchButtonDisplay(this, camera);

}


void CEV_switchButtonMove(CEV_SwitchButton* this)
{//updates status

    this->reActive.value = this->isActivated;

    CEV_edgeUpdate(&this->reActive);

    //numerical command as plateform call
    if(this->nDst && this->reActive.any)
        *(this->nDst) = this->value;

    //logical command
    if(this->bDst)
    {
        *(this->bDst) = this->isReversible ?
                        this->isActivated :
                        this->reActive.re || *(this->bDst);
    }

    L_switchButtonAnim(this);
}


void CEV_switchButtonDisplay(CEV_SwitchButton* this, SDL_Rect camera)
{//displays button

    SDL_Renderer* render = CEV_videoSystemGet()->render;

    SDL_Rect blitPos = {this->pos.x - camera.x, this->pos.y - camera.y, this->clip.w, this->clip.h};
    SDL_RenderCopy(render, this->anim->pic, &this->clip, &blitPos);
}


int CEV_switchButtonToCapsule(CEV_SwitchButton* src, CEV_Capsule *dst)
{//converts into capsule


    return 0;
}


int CEV_switchButtonConvertTxtToData(const char* srcName, const char* dstName)
{//converts txt editing file into file

    int funcSts = FUNC_OK;

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

    //id
    uint32_t valu32 = CEV_txtParseValueFrom(src, "id");
    valu32 = (valu32 & 0xFFFF0000) | SWITCHBTN_OBJECT;
    write_u32le(valu32, dst);

    //destination id
    write_u32le(CEV_txtParseValueFrom(src, "dstId"), dst);

    //animini id
    uint32_t animId = CEV_txtParseValueFrom(src, "animId");
    write_u32le(animId, dst);

    //destination id
    write_u32le(CEV_txtParseValueFrom(src, "value"), dst);


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

        CEV_AniMini *ani = CEV_aniMiniLoad(aniMiniName);
        CEV_aniMiniTypeWrite(ani, dst);
        CEV_aniMiniDestroy(ani);
    }

err_1:
    CEV_textDestroy(src);

    fclose(dst);

    return 0;
}


static void L_switchButtonPreset(CEV_SwitchButton* this)
{//sets values to preset // starts animation

    this->clip = this->anim->clip;
    CEV_rectDimCopy(this->clip, &this->pos);
    //CEV_spriteMiniPlay(&(this->anim->sprite), true);//playing at start up
    this->aniCtrl.preset = this->anim->numOfPic[0] * this->anim->delay;
    this->aniCtrl.run = true;
}


static void L_switchButtonAnim(CEV_SwitchButton* this)
{
    if(this->anim->numOfAnim > 1)
        this->sprite.switchAnim = this->isActivated;

    if(this->reActive.any)
    {
        this->aniCtrl.preset = this->anim->numOfPic[this->sprite.switchAnim] * this->anim->delay;
    }
    //bool needsUpdate = (this->sprite.picAct != this->anim->numOfPic[this->sprite.toggleAnim]);

    CEV_timerRepeat(&this->aniCtrl);
    //if(needsUpdate)
    if(!this->aniCtrl.cmd)
        this->clip = CEV_spriteMiniUpdate(&this->sprite, this->aniCtrl.preset - this->aniCtrl.accu);

}
