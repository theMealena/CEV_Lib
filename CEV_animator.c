//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2016      |   0.0    |    creation    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  |  diag improved **/
//**   CEV    |  09-2018      |   1.0.2  |animAuto public **/
//**   CEV    |  04-2023      |   1.1.0  |CEV_lib format  **/
//**********************************************************/

/** log :
CEV /04-2023 /1.1.0
    - blit / blitEx does not update sprite status anymore, needs spriteUpdate()
    - convert function now uses CEV_Text / parsing from txt file.
    - functions renamed to fit CEV_lib std model.
    - Deprecated functions removed due to structures gone public.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>


#include <SDL.h>
#include <SDL_image.h>
#include "project_def.h"
#include "CEV_dataFile.h"
#include "CEV_api.h"
#include "CEV_animator.h"
#include "CEV_mixSystem.h"
#include "CEV_file.h"
#include "CEV_texts.h"
#include "CEV_txtParser.h"
#include "rwtypes.h"


/*---LOCAL FUNCTIONS DECLARATION---*/


//resets sprite instance
static void L_spriteInit(SP_Sprite* sprite, int viewIndex);

//selects next picture
static void L_spritePicureNxt(SP_Sprite* sprite, int viewIndex);

//scaling / display position correction
static void L_spriteBlitPosScale(double scale, SDL_Rect clip, const SDL_Point display, SDL_Rect* result);

//clip update
static void L_spriteClipUpdate(SP_Sprite* sprite, int viewType, SDL_Rect* clip);

//limit
static bool L_lim(int limInf, int val, int limSup);

//reads view from file
static void L_animViewTypeRead_RW(SDL_RWops* src, SP_View* dst);

//fetch view request
static int32_t L_viewReqNxtGet(int32_t *req);

//stores request
static char L_viewReqAppend(int32_t *req, int32_t view);

//clears requests
static void L_viewReqClear(int32_t *req);


/*-- file format dedicated locals --*/

//writes view type into file
static void L_viewTypeWrite(SP_View *src, FILE *dst);

//Converts CEV_Text as text file as view written into dst
static void L_viewConvertTxtToData(CEV_Text *src, FILE *dst, int index);

//SP_LOOP_MODE from text into enum's value
static uint32_t L_animModeStringToValue(char* mode);


void TEST_sprite(void)
{/**testing sprite sheets lib*/

#include <CEV_display.h>

    /*CEV_Texture image = CEV_textureLoad("serenity.png");*/
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    CEV_Input *input = CEV_inputGet();

    printf("convert = %d\n", SP_animConvertTxtToData("sprite/spriteSave.txt", "sprite/save.sps"));
    //SP_AnimList* animTab = SP_animListLoad("bonhommelarite.dat");

    //CEV_Font *font = CEV_fontFetchById(1, "compiled.dat");

    //printf("animTab contient %d animations.\n", animTab->num);
    SP_Anim* anim = SP_animLoad("sprite/save.sps");



    if(IS_NULL(anim))
    {
        fprintf(stderr, "Err at %s / %d : anim is NULL.\n", __FUNCTION__, __LINE__ );
        return;
    }

    SDL_Rect blitClip = CEV_textureDimGet(anim->sheet);

    SP_Sprite *bonhomme = SP_spriteCreateFromAnim(anim);
    SP_spriteStart(bonhomme);
    bonhomme->scale = 5.0;

    SP_viewDisplay(bonhomme,SP_NVIEW, SP_SHOW);
    //SP_spriteStart(bonhomme);
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SP_viewRequest(bonhomme, SP_NVIEW, 0);

    //SDL_Rect temp;
    //SDL_Point rot = {50,50};

    SDL_SetRenderDrawColor(render, 0, 0, 0, 0);
    SDL_RenderClear(render);
    //SDL_RenderCopy(render, bonhomme->anim->sheet, NULL, &blitClip);
    //SDL_RenderPresent(render);
    //SDL_Delay(2000);
    bool quit = false;
    int viewReq = 0;
    double angle = 0.0;

    while(!quit)
    {
        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_ESCAPE] || input->window.quitApp)
            quit = true;

        if(input->mouse.button[SDL_BUTTON_WHEELDOWN])
        {
            CEV_addModulo(INC, &viewReq, anim->viewNum[0]);
            SP_viewRequest(bonhomme, SP_NVIEW, viewReq);
        }
        if(input->mouse.button[SDL_BUTTON_WHEELUP])
        {
            CEV_addModulo(DEC, &viewReq, anim->viewNum[0]);
            SP_viewRequest(bonhomme, SP_NVIEW, viewReq);
        }

        if(input->key[SDL_SCANCODE_RIGHT])
        {
            angle = CEV_fModulo(angle +2.0, 360.0);
        }

        if(input->key[SDL_SCANCODE_LEFT])
        {
            angle = CEV_fModulo(angle -2.0, 360.0);
        }

        if(input->key[SDL_SCANCODE_DOWN])
        {
            angle = 0.0;
            input->key[SDL_SCANCODE_DOWN] = false;
        }


        SDL_Point pos = {700, 300};

        SDL_Rect clip = SP_spriteClipGet(bonhomme, SP_NVIEW);
        CEV_renderColorSet(render, (SDL_Color){255, 0, 0, SDL_ALPHA_OPAQUE});
        SDL_RenderDrawRect(render, &clip);
        CEV_renderColorSet(render, (SDL_Color){0, 0, 0, SDL_ALPHA_OPAQUE});
        SDL_RenderCopy(render, anim->sheet, NULL, &blitClip);
        SP_spriteUpdate(bonhomme);
        SP_spriteBlitEx(bonhomme, &pos, angle, NULL, flip);


        //SDL_Rect box = SP_spriteHBoxGet(bonhomme, pos, flip);
        //SDL_RenderDrawRect(render, &box);
        //CEV_dispValue(SP_spriteIsLocked(bonhomme)? 1:0, font->font, (SDL_Color){.r=100,.g=45,.b=195, .a=255}, (SDL_Point){.x=20, .y=510}, CEV_LEFT | CEV_TOP, 1.0);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);
        SDL_Delay(10);
    }


    SP_spriteDestroy(bonhomme);
    SP_animDestroy(anim);

    //SP_animListFree(animTab, 1);

}


/*-------- DEBUG  ---------*/

void SP_spriteDump(SP_Sprite* this)
{//dumps srpite structure content

    puts("*** BEGIN SP_Sprite ***");

    if(IS_NULL(this))
	{
		puts("this SP_Sprite is NULL");
        goto end;
	}

    printf("Sprite is at %p\n", this);
    printf("Animation at %p\n", this->anim);
    printf("Scale is %f\n", this->scale);
    printf("Animation is %s\n", this->run ? "Running" : "Stopped");

    for(int i=0; i<2; i++)
    {
        printf("*Dumping %s :*\n", i? "NVIEW" : "XVIEW");
        printf(" Is locked : %d\n", this->control[i].isLocked);
        printf(" Show view : %d\n", this->control[i].viewShow);
        printf(" Direction : %d\n", this->control[i].direction);
        printf(" Act View : %d\n", this->control[i].viewAct);
        printf(" Act Pic : %d\n", this->control[i].picAct);

        printf(" View requests are :");
        for(int j=0; j<SP_NUM_OF_REQ_MAX; j++)
        {
            printf(" %d;", this->control[i].viewReq[j]);

        }
        puts("\n");
    }

end:
    puts("***END SP_Sprite ***");

}


void SP_viewDump(SP_View* this)
{//dumps view content

    puts("*** BEGIN SP_View ***");

    if(IS_NULL(this))
	{
		puts("this SP_View is NULL");
        goto end;
	}

    printf("picNum = %d\n delay = %d\n restart = %d\n stop = %d\n mode = %d\n",
        this->picNum, this->delay, this->restart, this->stop, this->mode);

end:
    puts("*** END SP_View ***");
}


void SP_animDump(SP_Anim* anim)
{//dumps anim content

    puts("*** BEGIN SP_Anim ***");

    printf("spritesheet at %p\n", anim->sheet);

    for(int view = 0; view<SP_VIEW_LAST; view++)
    {
        printf("For %s : %d views\n", view? "XVIEW" : "NVIEW", anim->viewNum[view]);
        for(unsigned i=0; i<anim->viewNum[view]; i++)
        {
            printf("View num %d\n", i);
            SP_viewDump(&anim->view[view][i]);

        }
    }
    puts("*** END SP_Anim ***");
}


/*------- USER END FUNCTIONS  ---------*/


/*-- animation functions --*/

SP_Anim* SP_animCreate(uint32_t nview, uint32_t xview, SDL_Texture* sheet)
{//creation and alloc of animation

    SP_Anim* result = NULL;

    if IS_NULL(sheet)
    {//in case of in-built function argument
        fprintf(stderr, "Warn at %s / %d : texture provided is NULL.\n", __FUNCTION__, __LINE__);
    }

    result = calloc(1, sizeof(SP_Anim)); //allocating memory

    if IS_NULL(result)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to allocate main struct : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    if(nview)
    {
        result->viewNum[SP_NVIEW] = nview;
        result->view[SP_NVIEW]    = calloc ((size_t)nview, sizeof(SP_View));

        if IS_NULL(result->view[SP_NVIEW])
        {//on error
            fprintf(stderr, "Err at %s / %d : unable to allocate nview : %s\n", __FUNCTION__, __LINE__, strerror(errno));
            goto err_1;
        }
    }
    else
        result->view[SP_NVIEW] = NULL;


    if(xview)
    {
        result->viewNum[SP_XVIEW] = xview;
        result->view[SP_XVIEW]    = calloc ((size_t)xview, sizeof(SP_View));

        if(IS_NULL(result->view[SP_XVIEW]))
        {//on error
            fprintf(stderr, "Err at %s / %d : unable to allocate xview : %s\n", __FUNCTION__, __LINE__, strerror(errno));
            goto err_2;
        }
    }
    else
        result->view[SP_XVIEW] = NULL;

    /*---POST---*/

    result->sheet = sheet;

end :
    return result;

err_2 :
    free(result->view[SP_NVIEW]);

err_1 :
    free(result);
    return NULL;
}


SP_Anim* SP_animLoad(const char* fileName)
{//load animation from file

    SDL_RWops *src = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return NULL;
    }

    SP_Anim* result = SP_animLoad_RW(src, true);

    return result;
}


SP_Anim* SP_animLoad_RW(SDL_RWops* src, bool freeSrc)
{//load animation set from RWops


    //reading header
    uint32_t    id      = SDL_ReadLE32(src), //id
                nView   = SDL_ReadLE32(src), //Nview num
                xView   = SDL_ReadLE32(src), //Xview num
                picId   = SDL_ReadLE32(src); //picture ID


    SP_Anim *result = SP_animCreate(nView, xView, NULL);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : SP_anim creation failed.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    result->id      = id;
    result->picId   = picId;

    for (int i = 0; i<SP_VIEW_LAST; i++)
        for(unsigned j = 0; j<result->viewNum[i]; j++)
            L_animViewTypeRead_RW(src, &result->view[i][j]);

    //reading // extracting pic if embedded
    if(!picId)
    {
        CEV_Capsule caps;

        CEV_capsuleTypeRead_RW(src, &caps);

        if(!IS_PIC(caps.type))
        {
            fprintf(stderr, "Warn at %s / %d : Embedded file is not picture.\n", __FUNCTION__, __LINE__ );
        }

        result->sheet = CEV_capsuleExtract(&caps, true);
    }

end:

    if (freeSrc)
        SDL_RWclose(src);

    return result;
}


int SP_animSave(SP_Anim *src, char* fileName)
{//saves animation into file

    //readWriteErr = 0;

    FILE *dst = fopen(fileName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    SP_animTypeWrite(src, dst);

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int SP_animTypeWrite(SP_Anim *src, FILE *dst)
{//writes src into dst

    write_u32le(src->id, dst);          //writting id
    write_u32le(src->viewNum[0], dst);  //writting num of nview
    write_u32le(src->viewNum[1], dst);  //writting num of wview
    write_u32le(src->picId, dst);       //writting pic id

    //writting views parameters
    for(int i=0; i<SP_VIEW_LAST; i++)
        for(unsigned j = 0; j<src->viewNum[i]; j++)
            L_viewTypeWrite(&src->view[i][j], dst);

    //inserting picture
    if(NOT_NULL(src->sheet))
    {
        CEV_Capsule caps;
        CEV_textureToCapsule(src->sheet, &caps);
        CEV_capsuleTypeWrite(&caps, dst);
        CEV_capsuleClear(&caps);
    }

    return readWriteErr? FUNC_ERR : FUNC_OK;
}


int SP_animConvertTxtToData(const char* srcName, const char* dstName)
{//converts txt file into data format.

    CEV_Text* src = CEV_textTxtLoad(srcName);

    if (IS_NULL(src))
    {
        printf("Err at %s / %d : unable to load %s : %s\n", __FUNCTION__, __LINE__, dstName, strerror(errno));
        goto err_1;
    }

    FILE* dst = NULL;

    dst = fopen(dstName, "wb");

    if (IS_NULL(dst))
    {
        printf("Err at %s / %d : unable to open %s : %s\n", __FUNCTION__, __LINE__, dstName, strerror(errno));
        goto err_1;
    }

    //id
    uint32_t id = (uint32_t)CEV_txtParseValueFrom(src, "id");
    id = (id & 0x00FFFFFF) | SP_TYPE_ID;
    write_u32le(id, dst);

    //num of n view
    uint32_t vNum = (uint32_t)CEV_txtParseValueFrom(src, "vNum");
    write_u32le(vNum, dst);

    //num of x view
    uint32_t xNum = (uint32_t)CEV_txtParseValueFrom(src, "xNum");
    write_u32le(xNum, dst);

    //picId
    uint32_t picId = (uint32_t)CEV_txtParseValueFrom(src, "picId");
    write_u32le(picId, dst);

    //each view
    for(unsigned i= 0; i< vNum + xNum; i++)
        L_viewConvertTxtToData(src, dst, i);

    //inserting picture if not referenced
    if(!picId)
    {
        char folderName[FILENAME_MAX];

        CEV_fileFolderNameGet(srcName, folderName);

        char *picName = CEV_txtParseTxtFrom(src, "picture");

        if(picName)
        {
            strcat(folderName, picName);

            CEV_Capsule caps = {0};

            CEV_capsuleFromFile(&caps, folderName);

            if(!IS_PIC(caps.type))
            {
                fprintf(stderr, "Warn at %s / %d : file is not picture.\n", __FUNCTION__, __LINE__ );
            }

            CEV_capsuleTypeWrite(&caps, dst);
        }
    }

    if(fclose(dst))
    {
        readWriteErr++;
    }

err_1 :

    CEV_textDestroy(src);

    return (readWriteErr)? FUNC_ERR : FUNC_OK;
}


void SP_animDestroy(SP_Anim* anim)
{//frees it all

    if(IS_NULL(anim))
        return;

    SP_animClear(anim);

    free(anim);
}


void SP_animClear(SP_Anim* anim)
{//clears content

    if(IS_NULL(anim))
        return;

    if (!anim->picId && NOT_NULL(anim->sheet))
    {
        SDL_DestroyTexture(anim->sheet);
    }

    free(anim->view[SP_NVIEW]);
    free(anim->view[SP_XVIEW]);

    *anim = (SP_Anim){0};
}



/*-- Sprite based related functions --*/

SP_Sprite* SP_spriteCreateFromAnim(SP_Anim *anim)
{//creates new sprite based on anim

    SP_Sprite *result = calloc(1, sizeof(SP_Sprite));

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    SP_spriteLinkToAnim(result, anim);

    return result;
}


void SP_spriteLinkToAnim(SP_Sprite* sprite, SP_Anim *anim)
{//links existing sprite to anim

    if( IS_NULL(sprite) || IS_NULL(anim) )
        return;

    sprite->anim = anim;

    SP_spriteReset(sprite);
}


void SP_spriteDestroy(SP_Sprite *src)
{//destroys content and itself
    free(src);
}


void SP_spriteClear(SP_Sprite *src)
{//clear structure content

    *src = (SP_Sprite){0};
}


void SP_spriteReset(SP_Sprite *sprite)
{//resets sprite struct to default

    if IS_NULL(sprite)
        return;

    sprite->scale   = 1.0;
    sprite->run     = false;

    for (int i =0; i<SP_VIEW_LAST; i++)
    {
        sprite->control[i].timePrev  = SDL_GetTicks();
        sprite->control[i].viewShow  = SP_HIDE;
        sprite->control[i].direction = 1;

        if(sprite->anim != NULL)
            SP_viewForce(sprite, i, 0, 0);

        L_viewReqClear(sprite->control[i].viewReq);
    }
}


void SP_spriteUpdate(SP_Sprite* sprite)
{//animation management

    SP_Anim *anim = (IS_NULL(sprite))? NULL : sprite->anim;

    if (IS_NULL(anim))
        return;


    for (int i=0; i<SP_VIEW_LAST && sprite->anim->viewNum[i]; i++)
    {
        //ez coding shortcuts

        uint32_t    *actViewNum  = &sprite->control[i].viewAct,
                    delay        = anim->view[i][*actViewNum].delay,
                    timeElapsed  = SDL_GetTicks() - sprite->control[i].timePrev;

//       if (!sprite->control[i].viewShow)//if view inactive
//            continue; //skip code

        if (timeElapsed >= delay)
        {//if time elapsed

            uint32_t viewReq    = sprite->control[i].viewReq[0] != SP_NONE,
                    viewLocked  = sprite->control[i].isLocked;

            if (viewReq)
            {//if view request
                if (!viewLocked)
                {//if view unlocked
                    *actViewNum = L_viewReqNxtGet(sprite->control[i].viewReq);  //changing actual view
                    L_spriteInit(sprite, i);                        //reset selected view
                }
                else
                {
                    if (anim->view[i][*actViewNum].mode == SP_FOR_REV_LOCK)
                        sprite->control[i].direction = -1;  //change direction

                    L_spritePicureNxt(sprite, i);   //select next picture
                }
            }
            else
                L_spritePicureNxt(sprite, i);
        }
    }
}



void SP_spriteBlitEx(SP_Sprite* sprite, const SDL_Point* pos,const double angle, const SDL_Point* center, const SDL_RendererFlip flip)
{//blits sprite with ex // no update

    if (IS_NULL(sprite) || IS_NULL(pos))
    {
        fprintf(stderr, "Warn at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }

    SDL_Rect scaledPos,
             clipPos;

    SDL_Renderer *dst = CEV_videoSystemGet()->render;

    //if(sprite->run)
        //SP_spriteUpdate(sprite);

    for(int i=0; i<SP_VIEW_LAST; i++)
    {
        if (sprite->control[i].viewShow)
        {
            L_spriteClipUpdate(sprite, i, &clipPos);
            L_spriteBlitPosScale(sprite->scale, clipPos, *pos, &scaledPos);
            SDL_RenderCopyEx(dst, sprite->anim->sheet, &clipPos, &scaledPos, angle, center, flip);
        }
    }
}


void SP_spriteBlit(SP_Sprite* sprite, const SDL_Point* pos)
{//blits sprite /: no update


    if (IS_NULL(sprite) || IS_NULL(pos))
    {
        fprintf(stderr, "Warn at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }

    SDL_Rect blit,
             clip;

    SDL_Renderer *dst = CEV_videoSystemGet()->render;

    //if(sprite->run)
        //SP_spriteUpdate(sprite);


    for(int i=0; i<SP_VIEW_LAST; i++)
    {
        if (sprite->control[i].viewShow)
        {
            L_spriteClipUpdate(sprite, i, &clip);
            L_spriteBlitPosScale(sprite->scale, clip, *pos, &blit);
            SDL_RenderCopy(dst, sprite->anim->sheet, &clip, &blit);
        }
    }
}


int SP_spriteModeGet(SP_Sprite *sprite)
{//fetches playing mode

    return sprite->anim->view[SP_NVIEW][1].mode;
}


SDL_Rect SP_spriteClipGet(SP_Sprite* sprite, uint32_t viewType)
{//returns clip pos in sprite sheet

    SDL_Rect result;

    if((sprite != NULL) && (viewType<SP_VIEW_LAST))
    {
        result = sprite->anim->view[viewType][sprite->control[viewType].viewAct].rect[SP_CLIP];
        result.x = result.w * sprite->control[viewType].picAct;
    }
    else
        result = CLEAR_RECT;

    return result;
}


SDL_Rect SP_spriteHBoxClipGet(SP_Sprite* sprite)
{//return hbox position on spritesheet

    SDL_Rect box  = sprite->anim->view[SP_NVIEW][sprite->control[SP_NVIEW].viewAct].rect[SP_HBOX],
             clip = sprite->anim->view[SP_NVIEW][sprite->control[SP_NVIEW].viewAct].rect[SP_CLIP],
             result = box;


    result.x = clip.x + box.x + ((unsigned)sprite->control[SP_NVIEW].picAct * clip.w);
    result.y = clip.y + box.y;

    return result;
}


SDL_Rect SP_spriteHBoxGet(SP_Sprite* sprite, SDL_Point pos, SDL_RendererFlip flip)
{//returns hitBox pos in sprite's world

    SDL_Rect box  = sprite->anim->view[SP_NVIEW][sprite->control[SP_NVIEW].viewAct].rect[SP_HBOX],
             clip = sprite->anim->view[SP_NVIEW][sprite->control[SP_NVIEW].viewAct].rect[SP_CLIP],
             result;


    box.x *= sprite->scale;
    box.y *= sprite->scale;
    result.w = box.w *= sprite->scale;
    result.h = box.h *= sprite->scale;

    int leftX,
        topY;

    L_spriteBlitPosScale(sprite->scale, clip, pos, &clip);

    if(flip & SDL_FLIP_HORIZONTAL)
        leftX = clip.w - box.x - box.w;
    else
        leftX = box.x;

    if(flip & SDL_FLIP_VERTICAL)
        topY = clip.h - box.y - box.h;
    else
        topY = box.y;

    result.x = clip.x + leftX;
    result.y = clip.y + topY;

    return result;
}


void SP_viewForce(SP_Sprite* sprite, uint32_t viewType, uint32_t viewIndex, uint32_t frameIndex)
{//asynchroneous view change

    if ((viewType > SP_XVIEW)
        || (viewIndex >= sprite->anim->viewNum[viewType])
        || (frameIndex >= sprite->anim->view[viewType][viewIndex].picNum))
        return;

    sprite->control[viewType].viewAct   = viewIndex; //View change

    for (int i =0; i<SP_XVIEW; i++)
        L_viewReqClear(sprite->control[i].viewReq);

    L_spriteInit(sprite, viewType);
    sprite->control[viewType].picAct = frameIndex;
// TODO (drx#1#): comment débloquer le forcage, et obtenir le lock depuis SP_spriteIsLocked() dès le forçage.
//last modif 14/05/2019
/*    if((sprite->control[viewType].direction == SP_FOR_ONCE) || (sprite->control[viewType].direction==SP_FOR_REV_LOCK))
        sprite->control[viewType].isLocked = 1;*/

}


char SP_viewRequest(SP_Sprite* sprite, uint32_t viewType, uint32_t viewIndex)
{//synchroneous view request

    if (sprite == NULL
        || (viewType > SP_XVIEW)
        || (viewIndex >= sprite->anim->viewNum[viewType]))
        return -1;

    return L_viewReqAppend(sprite->control[viewType].viewReq, viewIndex);
}


void SP_viewDisplay(SP_Sprite* sprite, uint32_t viewType, bool display)
{//enable / disable the display of a view

    if (viewType > SP_XVIEW)
        return;

    sprite->control[viewType].viewShow = display;
}


void SP_spriteStart(SP_Sprite* sprite)
{//starts playing sprite

    if(sprite != NULL)
        sprite->run = true;
}


void SP_spriteStop(SP_Sprite* sprite)
{//stops playing sprite

    if(sprite != NULL)
        sprite->run = false;
}


char SP_spriteIsLocked(SP_Sprite* sprite)
{//is locked anim still playing

    return sprite->control[SP_NVIEW].isLocked;
}



 /*--LOCAL FUNCTIONS---*/


static void L_viewConvertTxtToData(CEV_Text *src, FILE *dst, int index)
{//copies One spritesheet parameters

    uint32_t valu32;
    double buffer[4];

    char parName[50] = "\0",
         prefix [10] = "\0";

    sprintf(prefix, "[%d]", index);

    //num of pics
    sprintf(parName, "%spicNum", prefix);
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, parName);
    write_u32le(valu32, dst);

    //pics delay
    sprintf(parName, "%sdelay", prefix);
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, parName);
    write_u32le(valu32, dst);

    //restart pic index
    sprintf(parName, "%spicStart", prefix);
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, parName);
    write_u32le(valu32, dst);

    //play mode
    sprintf(parName, "%smode", prefix);
    valu32 = L_animModeStringToValue(CEV_txtParseTxtFrom(src, parName));
    write_u32le(valu32, dst);

    //picture clip
    sprintf(parName, "%sclip", prefix);
    valu32 = (uint32_t)CEV_txtParseValueArrayFrom(src, parName, buffer, 4);
    for(int i=0; i<4; i++)
        write_u32le((uint32_t)buffer[i], dst);

    //picture hit box
    sprintf(parName, "%shbox", prefix);
    valu32 = (uint32_t)CEV_txtParseValueArrayFrom(src, parName, buffer, 4);
    for(int i=0; i<4; i++)
        write_u32le((uint32_t)buffer[i], dst);


}


static void L_viewTypeWrite(SP_View *src, FILE *dst)
{//writes view structure content

    write_u32le(src->picNum, dst);
    write_u32le(src->delay, dst);
    write_u32le(src->restart, dst);
    write_u32le(src->mode, dst);

    for(int i=0; i<=SP_HBOX; i++)
    {
        write_u32le(src->rect[i].x, dst);
        write_u32le(src->rect[i].y, dst);
        write_u32le(src->rect[i].w, dst);
        write_u32le(src->rect[i].h, dst);
    }
}


static uint32_t L_animModeStringToValue(char* mode)
{//string to view mode enum

   char* enumList[SP_MODE_LAST] = SP_MODE_LIST;

    for (int i=0; i<SP_MODE_LAST; i++)
    {
        if (!strcmp(mode, enumList[i]))
            return i;
    }

    return SP_LOOP_FOR;
}


static void L_spriteInit(SP_Sprite* sprite, int viewIndex)
{//newly selected sprite parameters

    SP_View *view = &(sprite->anim->view[viewIndex][sprite->control[viewIndex].viewAct]);

    sprite->control[viewIndex].direction    = 1;
    sprite->control[viewIndex].picAct       = 0;
    sprite->control[viewIndex].timePrev     = SDL_GetTicks();
    sprite->control[viewIndex].isLocked     = ((view->mode == SP_FOR_REV_LOCK) || (view->mode == SP_FOR_ONCE));
}


static void L_spritePicureNxt(SP_Sprite* sprite, int viewIndex)
{//selects next picture to be shown

     SP_View *view = &(sprite->anim->view[viewIndex][sprite->control[viewIndex].viewAct]);

     switch (view->mode)
     {
        case SP_LOOP_FOR :

            if(sprite->control[viewIndex].picAct >= view->picNum-1)
                sprite->control[viewIndex].picAct = view->restart;
            else
                sprite->control[viewIndex].picAct++;
        break;

        case SP_FOR_REV :

            if(!L_lim(0, sprite->control[viewIndex].picAct + sprite->control[viewIndex].direction, view->picNum-1))
                sprite->control[viewIndex].direction *= -1;

            sprite->control[viewIndex].picAct += sprite->control[viewIndex].direction;
        break;

        case SP_FOR_ONCE :

            if(sprite->control[viewIndex].picAct < view->picNum-1)
            //{
                sprite->control[viewIndex].picAct++;
                //sprite->isLocked[viewIndex] = false;//
            //}//
            else//
                //sprite->isLocked[viewIndex] = true;//
                sprite->control[viewIndex].isLocked = sprite->control[viewIndex].picAct < view->picNum-1;

        break;

        case SP_FOR_REV_LOCK :

            switch (sprite->control[viewIndex].direction)
            {
                case 1 :
                    if(sprite->control[viewIndex].picAct >= view->picNum-1)
                        sprite->control[viewIndex].picAct = view->restart;
                    else
                        sprite->control[viewIndex].picAct++;
                break;

                case -1 :
                    if(sprite->control[viewIndex].picAct > 0)
                        sprite->control[viewIndex].picAct--;

                break;
            }

            sprite->control[viewIndex].isLocked = (sprite->control[viewIndex].picAct != 0);

        break;
     }

     sprite->control[viewIndex].timePrev = SDL_GetTicks(); //memo time
}


static void L_spriteBlitPosScale(double scale, SDL_Rect clip, const SDL_Point display, SDL_Rect* result)
{//creates display rect according to scale

     result->w = clip.w * scale;
     result->h = clip.h * scale;
     result->x = display.x - (result->w/2);
     result->y = display.y - (result->h/2);
}


static void L_spriteClipUpdate(SP_Sprite* sprite, int viewType, SDL_Rect* clip)
{//selects clip in sprite sheet

    *clip = sprite->anim->view[viewType][sprite->control[viewType].viewAct].rect[SP_CLIP];

    clip->x = sprite->anim->view[viewType][sprite->control[viewType].viewAct].rect[SP_CLIP].w * sprite->control[viewType].picAct;
    //clip->y = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].y;
    //clip->w = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].w;
    //clip->h = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].h;
}


static bool L_lim(int limInf, int val, int limSup)
{//interval checked within limits included

    if (limInf<=limSup)
       return (val>=limInf && val<=limSup);
    //else
    return (val>=limInf || val<=limSup);
}


static void L_animViewTypeRead_RW(SDL_RWops* src, SP_View* dst)
{//fills animation parameters

    //picNum, delay
    dst->picNum = SDL_ReadLE32(src);
    dst->delay  = SDL_ReadLE32(src);

    //restart, mode
    dst->restart    = SDL_ReadLE32(src);
    dst->mode       = SDL_ReadLE32(src);

    //clip x ;	clip y ; clip w ; clip h ; hb x ; hb y ; hb w ; hb h
    dst->rect[SP_CLIP].x = SDL_ReadLE32(src);
    dst->rect[SP_CLIP].y = SDL_ReadLE32(src);
    dst->rect[SP_CLIP].w = SDL_ReadLE32(src);
    dst->rect[SP_CLIP].h = SDL_ReadLE32(src);
    dst->rect[SP_HBOX].x = SDL_ReadLE32(src);
    dst->rect[SP_HBOX].y = SDL_ReadLE32(src);
    dst->rect[SP_HBOX].w = SDL_ReadLE32(src);
    dst->rect[SP_HBOX].h = SDL_ReadLE32(src);
}


static int32_t L_viewReqNxtGet(int32_t *req)
{//gets request from stack

    int32_t result = req[0];

    for (int i=0; i<SP_NUM_OF_REQ_MAX-1; i++)
        req[i] = req[i+1];

    req[SP_NUM_OF_REQ_MAX-1] = SP_NONE;

    return result;
}


static char L_viewReqAppend(int32_t *req, int32_t view)
{//fills request stack

    int i = 0;

    while ( (req[i] != SP_NONE) && (i < SP_NUM_OF_REQ_MAX) )
        i++;

    if (i >= SP_NUM_OF_REQ_MAX)
        return SP_NONE;
    else
        req[i] = view;

    return i;
}


static void L_viewReqClear(int32_t *req)
{//empties request stack

    for(int i=0; i<SP_NUM_OF_REQ_MAX; i++)
        req[i] = SP_NONE;
}
