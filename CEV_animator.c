//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2016      |   0.0    |    creation    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  |  diag improved **/
//**   CEV    |  09-2018      |   1.0.2  |animAuto made public**/
//**********************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include <SDL_image.h>
#include "project_def.h"
#include "CEV_dataFile.h"
#include "CEV_api.h"
#include "CEV_animator.h"
#include "CEV_mixSystem.h"
#include "CEV_file.h"
#include "rwtypes.h"

// TODO (drx#1#03/20/23): Supprimer la liste d'animation, chaque animation est indépendante désormais. Mettre au standard CEV_lib

/*---LOCAL FUNCTIONS DECLARATION---*/

/*selecting next view*/
static void L_animAuto(SP_Sprite* sprite);

/*reset sprite instance*/
static void L_animSpriteReset(SP_Sprite* sprite, int viewIndex);

/*select next picture*/
static void L_animNextPicture(SP_Sprite* sprite, int viewIndex);

/*scaling*/
static void L_animScalePos(double scale, SDL_Rect clip, const SDL_Point display, SDL_Rect* result);

/*clip update*/
static void L_animUpdateClip(SP_Sprite* sprite, int viewType, SDL_Rect* clip);

/*limit*/
static char L_animLim(int limInf, int val, int limSup);

/*extract SDL_Texture from ops animSet file*/
static SDL_Texture *L_animTextureGet_RW(SDL_RWops* ops);

/*read anim file*/
static void L_animFillRW(SP_Anim* anim, SDL_RWops* ops);

/*fetch view request*/
static int16_t L_animReqGetNxt(int16_t *req);

/*stores request*/
static char L_animReqAppend(int16_t *req, int16_t view);

/*dumps requests*/
static void L_animReqDump(int16_t *req);


/*-- file format dedicated locals --*/
static void L_animSpriteSheetTypeWrite(FILE *src, FILE *dst, char* folder);
static void L_animPictureTypeWrite(char* fileName, FILE* dst);
static void L_animViewTypeWrite(FILE *src, FILE *dst);
static uint16_t L_animSPViewStringToValue(char* mode);


void SP_viewDump(SP_View* view)
{//dumps view content

    puts("- BEGIN - DUMPING SP_VIEW ****");
    printf("picNum = %d\n picTime = %d\n restart = %d\n stop = %d\n mode = %d\n",
        view->picNum, view->picTime, view->restart, view->stop, view->mode);
    puts("- END - DUMPING SP_VIEW ****");
}


void SP_animDump(SP_Anim* anim)
{//dumps animp content

    puts("- BEGIN - DUMPING SP_ANIM ****");

    printf("spritesheet at %p\n", anim->sheet);

    for(int view = 0; view<SP_VIEW_NUM; view++)
    {
        printf("For %s : %d views\n", view? "XVIEW" : "NVIEW", anim->viewNum[view]);
        for(int i=0; i<anim->viewNum[view]; i++)
        {
            printf("View num %d\n", i);
            SP_viewDump(&anim->view[view][i]);

        }
    }
    puts("- END - DUMPING SP_ANIM ****");
}


/*------- USER END FUNCTIONS  ---------*/


/*-- sprite sheet base related functions --*/

SP_Anim* SP_animCreate(uint16_t nview, uint16_t xview, SDL_Texture* sheet)
{//creation and alloc of animation

    /*---DECLARATIONS---*/

    SP_Anim* result = NULL;

    /*---PRL---*/

    if IS_NULL(sheet)
    {//in case of in-built function argument
        fprintf(stderr, "Err at %s / %d : texture parameter is NULL.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---EXECUTION---*/

    result = calloc(1, sizeof(SP_Anim)); //allocating memory

    if IS_NULL(result)
    {//on error
        fprintf(stderr, "Err at %s / %d : unable to allocate main struct : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    if(nview)
    {
        result->viewNum[SP_NVIEW] = nview;
        result->view[SP_NVIEW]    = calloc ((size_t)nview, sizeof(SP_View));

        if IS_NULL(result->view[SP_NVIEW])
        {//on error
            fprintf(stderr, "Err at %s / %d : unable to allocate nview : %s\n", __FUNCTION__, __LINE__, strerror(errno));
            goto err_2;
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
            goto err_3;
        }
    }
    else
        result->view[SP_XVIEW] = NULL;

    /*---POST---*/

    result->sheet = sheet;

exit :
    return result;

err_3 :
    free(result->view[SP_NVIEW]);

err_2 :
    free(result);

err_1 :
    SDL_DestroyTexture(sheet);

    return NULL;
}


void SP_animDestroy(SP_Anim* anim, char freePic)
{//free it all

    if(IS_NULL(anim))
        return;

    if (freePic && !IS_NULL(anim->sheet))
        SDL_DestroyTexture(anim->sheet);

    free(anim->view[SP_NVIEW]);
    free(anim->view[SP_XVIEW]);
    free(anim);
}


SDL_Texture* SP_animTextureGet(SP_Anim *anim)
{//gets texture

    return anim->sheet;
}


void SP_animQuery(SP_Anim *anim, uint16_t* nView, uint16_t* xView)
{//query animation

    if(nView != NULL)
        *nView = anim->viewNum[SP_NVIEW];

    if(xView != NULL)
        *xView = anim->viewNum[SP_XVIEW];
}


/*-- animation set functions--*/

SP_Anim* SP_AnimLoad(const char* fileName)
{

    SP_AnimList* srcList = SP_animListLoad(fileName);

    if(srcList->num > 1)
    {
        fprintf(stderr, "Err at %s / %d : fil contains multiple animations.\n", __FUNCTION__, __LINE__ );
        SP_animListFree(srcList, 1);
        return NULL;
    }

    SP_Anim *dst = malloc(sizeof(SP_Anim));

    if(IS_NULL(srcList))
    {
        fprintf(stderr, "Err at %s / %d : SrcList load failed.\n", __FUNCTION__, __LINE__ );
        exit(-1);
    }

    dst = SP_animListGetIndex(srcList ,0);
/*
    puts("dumping src");
    SP_animDump(SP_animListGetIndex(srcList ,0));
    puts("dumping dst");
    SP_animDump(dst);*/

    free(srcList);
/*
    puts("dumping dst after free");
    SP_animDump(dst);*/

    //exit(0);

    return dst;
}


SP_AnimList* SP_animListLoad(const char* fileName)
{//loads animation set

    /*---DECLARATIONS---*/

    SP_AnimList *set = NULL;
    SDL_RWops   *ops = NULL;

    /*---PRL---*/

    if IS_NULL(fileName)
    {
        fprintf(stderr,"Err at %s / %d : fileName is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    ops = SDL_RWFromFile(fileName, "rb");

    if IS_NULL(ops)
    {
        fprintf(stderr, "Err at %s / %d : unable to open file %s: %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
        return NULL;
    }

    /*---POST---*/

    set = SP_animListLoad_RW(ops, 1);

    return set;
}


SP_AnimList* SP_animListLoad_RW(SDL_RWops* ops, uint8_t freeSrc)
{//load animation set from RWops

    /*---DECLARATIONS---*/

    SP_AnimList* result = NULL;

    /*---PRL---*/

    result = malloc(sizeof(SP_AnimList));

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate result : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    /*---EXECUTION---*/

    SDL_RWseek(ops, 0, RW_SEEK_SET);

    result->num = SDL_ReadLE16(ops); //number of spritesheet in file

    result->animSet = malloc(result->num * sizeof(SP_Anim*));

    if IS_NULL(result->animSet)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate animSet : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_2;
    }

    for (int i =0; i<result->num; i++)
    {
        uint16_t nview, xview;
        SDL_Texture* texture = NULL;

        nview   = SDL_ReadLE16(ops);
        xview   = SDL_ReadLE16(ops);

        texture = L_animTextureGet_RW(ops);

        result->animSet[i] = SP_animCreate(nview, xview, texture);

        if IS_NULL(result->animSet[i])
        {
            fprintf(stderr, "Err at %s / %d : animation creation failed.\n", __FUNCTION__, __LINE__);
            goto err_2;
        }

        L_animFillRW(result->animSet[i], ops);
    }

    /*---POST---*/

    if (freeSrc)
        SDL_RWclose(ops);

    return result;

err_2 :
    free(result);

err_1 :
    if (freeSrc)
        SDL_RWclose(ops);

    return NULL;
}


uint16_t SP_animListNum(SP_AnimList *set)
{/*number of animation instance helds*/

    return set->num;
}


SP_Anim* SP_animListGetIndex(SP_AnimList *set, unsigned int index)
{/*fetches an anim instance*/

    if(index >= set->num)
        index = 0;

    return set->animSet[index];
}


void SP_animListFree(SP_AnimList *set, char freePic)
{//frees animation set

    if(IS_NULL(set))
        return;

    for(int i = 0; i<set->num; i++)
        SP_animDestroy(set->animSet[i], freePic);

    free(set->animSet);

    free(set);
}


/*--animations and views settings--*/

void SP_viewSet(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t frameNum, SDL_Rect clip, SDL_Rect hitBox, uint16_t time, uint16_t mode, uint16_t restart, uint16_t stop)
{//view setting

    SP_View* Lview = &anim->view[viewType][viewIndex];

    Lview->restart  = restart;
    Lview->stop     = stop;
    Lview->mode     = mode;
    Lview->picTime  = time;
    Lview->picNum   = frameNum;

    //if (clip != NULL)
        Lview->rect[SP_CLIP] = clip;

    if (/*hitBox != NULL && */viewType == SP_NVIEW)
        Lview->rect[SP_HBOX] = hitBox;
}


void SP_viewTime(SP_Anim* anim, uint8_t viewType, uint8_t viewIndex, uint16_t time)
{//sets view time
    anim->view[viewType][viewIndex].picTime = time;
}


void SP_viewMode(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t mode)
{//sets view mode
    anim->view[viewType][viewIndex].mode = mode;
}


int SP_fetchMode(SP_Sprite *sprite)
{//fetches playing mode
    return sprite->anim->view[SP_NVIEW][1].mode;
}


void SP_viewClip(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, SDL_Rect* clip)
{//sets clip
    anim->view[viewType][viewIndex].rect[SP_CLIP] = *clip;
}


void SP_viewHitBox(SP_Anim* anim, uint16_t viewIndex, SDL_Rect* hbox)
{//sets hitbox
    anim->view[SP_NVIEW][viewIndex].rect[SP_HBOX] = *hbox;
}


void SP_viewRestart(SP_Anim* anim, uint8_t viewType, uint16_t viewIndex, uint16_t restart)
{//set restart index
    anim->view[viewType][viewIndex].restart = restart;
}



/*-- Sprite based related functions --*/

SP_Sprite* SP_spriteCreateFromAnim(SP_Anim *anim)
{//creates new sprite based on anim

    SP_Sprite *result = calloc(1, sizeof(SP_Sprite));

    if IS_NULL(result)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate new sprite : %s\n", __FUNCTION__, __LINE__, strerror(errno));
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


void SP_spriteReset(SP_Sprite *sprite)
{//resets sprite struct to default

    if IS_NULL(sprite)
        return;

    sprite->scale   = 1.0;
    sprite->run     = 0;

    for (int i =0; i<SP_XVIEW; i++)
    {
        sprite->timePrev[i]  = SDL_GetTicks();
        sprite->viewShow[i]  = SP_HIDE;
        sprite->direction[i] = 1;

        if(sprite->anim != NULL)
            SP_viewForce(sprite, i, 0, 0);

        L_animReqDump(sprite->viewReq[i]);
    }
}


void SP_spriteScale(SP_Sprite* sprite, double scale)
{//sets scale

    if(sprite != NULL)
        sprite->scale = scale;
}


double SP_spriteScaleGet(SP_Sprite* sprite)
{//fetch scale

    return sprite->scale;
}


void SP_spriteAnim(SP_Sprite* sprite)
{//animation management // Ex L_animAuto()

        L_animAuto(sprite);
}


void SP_spriteBlitEx(SP_Sprite* sprite, SDL_Renderer* dst, const SDL_Point* pos,const double angle, const SDL_Point* center, const SDL_RendererFlip flip)
{//blit animation with ex

    SDL_Rect scaledPos,
             clipPos;

    if (IS_NULL(sprite) || IS_NULL(dst))
        return;

    if(sprite->run)
        L_animAuto(sprite);

    for(int i=0; i<=SP_XVIEW; i++)
        if (sprite->viewShow[i])
        {
            L_animUpdateClip(sprite, i, &clipPos);
            L_animScalePos(sprite->scale, clipPos, *pos, &scaledPos);
            SDL_RenderCopyEx(dst, sprite->anim->sheet, &clipPos, &scaledPos, angle, center, flip);
        }
}


void SP_spriteBlit(SP_Sprite* sprite, SDL_Renderer* dst, const SDL_Point* pos)
{//blit animation & blit

    SDL_Rect scaledPos,
             clipPos;

    if (IS_NULL(sprite) || IS_NULL(dst))
        return;

    if(sprite->run)
        L_animAuto(sprite);


    for(int i=0; i<=SP_XVIEW; i++)
        if (sprite->viewShow[i])
        {
            L_animUpdateClip(sprite, i, &clipPos);
            L_animScalePos(sprite->scale, clipPos, *pos, &scaledPos);
            SDL_RenderCopy(dst, sprite->anim->sheet, &clipPos, &scaledPos);
        }
}



SDL_Rect* SP_clipGet(SP_Sprite* sprite, uint8_t viewType)
{//returns clip pos in sprite sheet

    if((sprite != NULL) && (viewType<=SP_XVIEW))
        return &sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP];
    else
        return NULL;
}


SDL_Rect SP_sheetHBoxGet(SP_Sprite* sprite, SDL_RendererFlip flip)
{//return hbox position on spritesheet

    SDL_Rect box  = sprite->anim->view[SP_NVIEW][sprite->viewAct[SP_NVIEW]].rect[SP_HBOX],
             clip = sprite->anim->view[SP_NVIEW][sprite->viewAct[SP_NVIEW]].rect[SP_CLIP],
             result = box;

/*
    box.x *= sprite->scale;
    box.y *= sprite->scale;
    result.w = box.w *= sprite->scale;
    result.h = box.h *= sprite->scale;
*/
    int leftX,
        topY;

    //L_animScalePos(sprite->scale, clip, pos, &clip);
/*
    if(flip & SDL_FLIP_HORIZONTAL)
        leftX = clip.w - box.x - box.w;
    else*/
        leftX = box.x;
/*
    if(flip & SDL_FLIP_VERTICAL)
        topY = clip.h - box.y - box.h;
    else*/
        topY = box.y;

    result.x = clip.x + leftX + ((unsigned)sprite->picAct[SP_NVIEW] * clip.w);
    result.y = clip.y + topY;

    return result;
}


SDL_Rect SP_hBoxGet(SP_Sprite* sprite, SDL_Point pos, SDL_RendererFlip flip)
{//returns absolute pos of hitBox

    SDL_Rect box  = sprite->anim->view[SP_NVIEW][sprite->viewAct[SP_NVIEW]].rect[SP_HBOX],
             clip = sprite->anim->view[SP_NVIEW][sprite->viewAct[SP_NVIEW]].rect[SP_CLIP],
             result;


    box.x *= sprite->scale;
    box.y *= sprite->scale;
    result.w = box.w *= sprite->scale;
    result.h = box.h *= sprite->scale;

    int leftX,
        topY;

    L_animScalePos(sprite->scale, clip, pos, &clip);

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


void SP_viewForce(SP_Sprite* sprite, uint8_t viewType, uint16_t viewIndex, uint16_t frameIndex)
{//asynchroneous view change

    if ((viewType > SP_XVIEW)
        || (viewIndex >= sprite->anim->viewNum[viewType])
        || (frameIndex >= sprite->anim->view[viewType][viewIndex].picNum))
        return;

    sprite->viewAct[viewType]   = viewIndex; //View change

    for (int i =0; i<SP_XVIEW; i++)
        L_animReqDump(sprite->viewReq[i]);

    L_animSpriteReset(sprite, viewType);
    sprite->picAct[viewType] = frameIndex;
// TODO (drx#1#): comment débloquer le forcage, et obtenir le lock depuis SP_spriteIsLocked() dès le forçage.
//last modif 14/05/2019
/*    if((sprite->direction[viewType] == SP_FOR_ONCE) || (sprite->direction[viewType]==SP_FOR_REV_LOCK))
        sprite->isLocked[viewType] = 1;*/

}


char SP_viewRequest(SP_Sprite* sprite, uint8_t viewType, uint16_t viewIndex)
{//synchroneous view request

    if (sprite == NULL
        || (viewType > SP_XVIEW)
        || (viewIndex >= sprite->anim->viewNum[viewType]))
        return -1;

    return L_animReqAppend(sprite->viewReq[viewType], viewIndex);
}


void SP_viewDisplay(SP_Sprite* sprite, uint8_t viewType, uint8_t display)
{//enable / disable the display of a view

    if (viewType > SP_XVIEW)
        return;

    sprite->viewShow[viewType] = display;
}


void SP_spriteStop(SP_Sprite* sprite)
{//stops playing sprite

    if(sprite != NULL)
        sprite->run = 0;
}


void SP_spriteStart(SP_Sprite* sprite)
{//starts playing sprite

    if(sprite != NULL)
        sprite->run = 1;
}


char SP_spriteIsLocked(SP_Sprite* sprite)
{//is locked anim still playing

    return sprite->isLocked[SP_NVIEW];
}


void SP_spriteQuery(SP_Sprite* sprite, int* actNview, int* actXview, int* actNpic, int* actXpic)
{//gets informations

    if IS_NULL(sprite)
        return;

    if(actNview != NULL)
        *actNview = sprite->viewAct[SP_NVIEW];

    if(actXview != NULL)
        *actXview = sprite->viewAct[SP_XVIEW];

    if(actNpic != NULL)
        *actNpic = sprite->picAct[SP_NVIEW];

    if(actXpic != NULL)
        *actXpic = sprite->picAct[SP_XVIEW];
}


/*--File Format function--*/


int SP_spriteConvertTxtToData(const char* srcName, const char* dstName)
{//converts CSV file into data format

    /*---DECLARATIONS---*/

    unsigned int SPnum;
    char    folderName[FILENAME_MAX],
            hasFolder = CEV_fileFolderNameGet(srcName, folderName);

    FILE* src = NULL,
          *dst = NULL;

    /*---PRL---*/
    puts("preparing to create animation...");

    readWriteErr = 0;

    printf("opening %s...", srcName);
    src = fopen(srcName, "r");

    if IS_NULL(src)
    {
        printf("Err at %s / %d : unable to open %s : %s\n", __FUNCTION__, __LINE__, srcName, strerror(errno));
        goto end;
    }
    puts("ok");

    printf("creating %s...", dstName);
    dst = fopen(dstName, "wb");

    if IS_NULL(dst)
    {
        printf("Err at %s / %d : unable to create %s. %s\n", __FUNCTION__, __LINE__, dstName, strerror(errno));
        goto err_1;
    }
    puts("ok");
    /*---EXECUTION---*/

    rewind(src);

    fscanf(src, "%u", &SPnum);//number of sprite sheet to create
    write_u16le(SPnum, dst);

    printf("preparing to pack %d animation%s\n", SPnum, (SPnum>1)? "s." : ".");

    for(int i =0; i<SPnum; i++)
        L_animSpriteSheetTypeWrite(src, dst, hasFolder? folderName : NULL);

    if(readWriteErr)
        fprintf(stderr, "Err at %s / %d : read/write error.\n", __FUNCTION__,  __LINE__);


    /*---POST---*/

    fclose(dst);

err_1 :
    fclose(src);

end :
    return (readWriteErr)? FUNC_ERR : FUNC_OK;

}


 /*--LOCAL FUNCTIONS---*/

static void L_animSpriteSheetTypeWrite(FILE *src, FILE *dst, char *folder)
{//copies One spritesheet parameters

    unsigned int nView, xView;
    char fileName[FILENAME_MAX] = "\0",
         lString[FILENAME_MAX];

    fscanf(src, "%u %u %s", &nView, &xView, lString);//reading number of views and pic name

    write_u16le(nView, dst);     //writting num of nview
    write_u16le(xView, dst);     //writting num of wview

    if(folder != NULL)
    {
        strcpy(fileName, folder);
        strcat(fileName, lString);
    }
    else
        strcpy(fileName, lString);

    L_animPictureTypeWrite(fileName, dst);     //inserting pic into file

    for(int i = 0; i<(nView+xView); i++)
        L_animViewTypeWrite(src, dst);
}


static void L_animPictureTypeWrite(char* fileName, FILE* dst)
{//inserts pic

    printf("packing picture %s...", fileName);
    CEV_Capsule buffer;

    CEV_capsuleFromFile(&buffer, fileName);

    write_u32le(buffer.size, dst);

    if( fwrite(buffer.data, 1, buffer.size, dst) != buffer.size)
        readWriteErr++;

    printf("%s\n", readWriteErr? "nok" : "ok");

    free(buffer.data);
}


static void L_animViewTypeWrite(FILE *src, FILE *dst)
{//copies view parameters

    unsigned int temp, picNum, picTime;
    char mode[20];

    fscanf(src, "%u %u",&picNum, &picTime);
    write_u16le(picNum, dst);
    write_u16le(picTime, dst);

    for(int i=0; i<9; i++)
    {//clip x ;	clip y ; clip w ; clip h ; hb x ; hb y ; hb w ; hb h ; restart
        fscanf(src,"%u", &temp);
        write_u16le(temp, dst);
    }

    fscanf(src,"%s", mode);
    write_u16le(L_animSPViewStringToValue(mode), dst);
}


static uint16_t L_animSPViewStringToValue(char* mode)
{//string to view mode enum

   char* enumList[SP_MODE_NUM] = SP_MODE_LIST;

    for (int i=0; i<SP_MODE_NUM; i++)
    {
        if (!strcmp(mode, enumList[i]))
            return i;
    }

    return SP_LOOP_FOR;
}


// removed to be able to update before blit and enable position correction
static void L_animAuto(SP_Sprite* sprite)
{//animation management

        //---DECLARATIONS---

    int i;
    SP_Anim *anim = (sprite != NULL)? sprite->anim : NULL;

        //---PRL---

    if IS_NULL(anim)
        return;

    //---EXECUTION---

    for (i=0; i<=SP_XVIEW && sprite->anim->viewNum[i]; i++)
    {
        //ez coding shortcuts

        uint16_t    *actViewNum = &(sprite->viewAct[i]),
                    *picTime    = &(anim->view[i][*actViewNum].picTime);

        uint32_t timeElapsed = SDL_GetTicks() - sprite->timePrev[i];

//       if (!sprite->viewShow[i])//if view inactive
//            continue; //skip code

        if (timeElapsed >= *picTime)
        {//if time elapsed

            uint8_t viewReq     = sprite->viewReq[i][0] != SP_NONE,
                    viewLocked  = sprite->isLocked[i];

            if (viewReq)
            {//if view request
                if (!viewLocked)
                {//if view unlocked
                    *actViewNum = L_animReqGetNxt(sprite->viewReq[i]);  //changing actual view
                    L_animSpriteReset(sprite, i);                       //reset selected view
                }
                else
                {
                    if (anim->view[i][*actViewNum].mode == SP_FOR_REV_LOCK)
                        sprite->direction[i] = -1;//change direction

                    L_animNextPicture(sprite, i);
                }
            }
            else
                L_animNextPicture(sprite, i);
        }
    }
}


static void L_animSpriteReset(SP_Sprite* sprite, int viewIndex)
{//newly selected sprite parameters

    SP_View *view = &(sprite->anim->view[viewIndex][sprite->viewAct[viewIndex]]);

    sprite->direction[viewIndex] = 1;
    sprite->picAct[viewIndex]    = 0;
    sprite->timePrev[viewIndex]  = SDL_GetTicks();
    sprite->isLocked[viewIndex]  = ((view->mode == SP_FOR_REV_LOCK) || (view->mode == SP_FOR_ONCE));
}


static void L_animNextPicture(SP_Sprite* sprite, int viewIndex)
{//selects next picture to be shown

     SP_View *view = &(sprite->anim->view[viewIndex][sprite->viewAct[viewIndex]]);

     switch (view->mode)
     {
        case SP_LOOP_FOR :

            if(sprite->picAct[viewIndex] >= view->picNum-1)
                sprite->picAct[viewIndex] = view->restart;
            else
                sprite->picAct[viewIndex]++;
        break;

        case SP_FOR_REV :

            if(!L_animLim(0, sprite->picAct[viewIndex] + sprite->direction[viewIndex], view->picNum-1))
                sprite->direction[viewIndex] *= -1;

            sprite->picAct[viewIndex] += sprite->direction[viewIndex];
        break;

        case SP_FOR_ONCE :

            if(sprite->picAct[viewIndex] < view->picNum-1)
            //{
                sprite->picAct[viewIndex]++;
                //sprite->isLocked[viewIndex] = false;//
            //}//
            else//
                //sprite->isLocked[viewIndex] = true;//
            sprite->isLocked[viewIndex] = sprite->picAct[viewIndex] < view->picNum-1;

        break;

        case SP_FOR_REV_LOCK :

            switch (sprite->direction[viewIndex])
            {
                case 1 :
                    if(sprite->picAct[viewIndex] >= view->picNum-1)
                        sprite->picAct[viewIndex] = view->restart;
                    else
                        sprite->picAct[viewIndex]++;
                break;

                case -1 :
                    if(sprite->picAct[viewIndex] > 0)
                        sprite->picAct[viewIndex]--;

                break;
            }

            sprite->isLocked[viewIndex] = (sprite->picAct[viewIndex] != 0);

        break;
     }

     sprite->timePrev[viewIndex] = SDL_GetTicks(); //memo time
}


static void L_animScalePos(double scale, SDL_Rect clip, const SDL_Point display, SDL_Rect* result)
{//creates display rect according to scale

     result->w = clip.w * scale;
     result->h = clip.h * scale;
     result->x = display.x - (result->w/2);
     result->y = display.y - (result->h/2);
}


static void L_animUpdateClip(SP_Sprite* sprite, int viewType, SDL_Rect* clip)
{//selects clip in sprite sheet

    clip->x = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].w * sprite->picAct[viewType];
    clip->y = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].y;
    clip->w = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].w;
    clip->h = sprite->anim->view[viewType][sprite->viewAct[viewType]].rect[SP_CLIP].h;
}


static char L_animLim(int limInf, int val, int limSup)
{//interval checked within limits included

    switch (limInf<=limSup)
    {
        case true:
            if (val>=limInf && val<=limSup)
                return 1;
        break;

        case false:
            if (val>=limInf || val<=limSup)
                return 1;
        break;
    }

    return 0;
}


static SDL_Texture *L_animTextureGet_RW(SDL_RWops* ops)
{//extract SDL_Texture from ops animSet file

    /*---DECLARATIONS---*/

    uint32_t        textureSize = 0;
    SDL_Renderer    *render     = CEV_videoSystemGet()->render;
    void            *rawData    = NULL;
    SDL_Texture     *pic        = NULL;

    /*---PRL---*/

    if IS_NULL(ops)
        return NULL;

    /*---EXECUTION---*/

    textureSize = SDL_ReadLE32(ops);

    rawData = malloc(textureSize);

    if IS_NULL(rawData)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate rawData : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    SDL_RWread(ops, rawData, 1, textureSize);

    pic = IMG_LoadTexture_RW(render, SDL_RWFromConstMem(rawData, textureSize), 1);/*closes RWops*/

    if IS_NULL(pic)
    {
        fprintf(stderr,"Err at %s / %d : %s\n",__FUNCTION__, __LINE__, IMG_GetError());
        return NULL;
    }
    else
        SDL_SetTextureBlendMode(pic, SDL_BLENDMODE_BLEND);

    /*---POST---*/

    free(rawData);

    return pic;
}


static void L_animFillRW(SP_Anim* anim, SDL_RWops* ops)
{//fills animation parameters

    for (int i=0; i<SP_XVIEW; i++)
    {
        for (int j=0; j<anim->viewNum[i]; j++)
        {
            //picNum, picTime
            anim->view[i][j].picNum  = SDL_ReadLE16(ops);
            anim->view[i][j].picTime = SDL_ReadLE16(ops);
            //clip x ;	clip y ; clip w ; clip h ; hb x ; hb y ; hb w ; hb h ; restart, mode
            anim->view[i][j].rect[SP_CLIP].x = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_CLIP].y = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_CLIP].w = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_CLIP].h = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_HBOX].x = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_HBOX].y = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_HBOX].w = SDL_ReadLE16(ops);
            anim->view[i][j].rect[SP_HBOX].h = SDL_ReadLE16(ops);
            anim->view[i][j].restart         = SDL_ReadLE16(ops);
            anim->view[i][j].mode            = SDL_ReadLE16(ops);

        }
    }
}


static int16_t L_animReqGetNxt(int16_t *req)
{//gets request from stack

    int16_t result = req[0];

    for (int i=0; i<SP_NUM_OF_REQ_MAX-1; i++)
        req[i] = req[i+1];

    req[SP_NUM_OF_REQ_MAX-1] = SP_NONE;

    return result;
}


static char L_animReqAppend(int16_t *req, int16_t view)
{//fills request stack

    int i =0;

    while ( (req[i] != SP_NONE) && (i < SP_NUM_OF_REQ_MAX) )
        i++;

    if (i >= SP_NUM_OF_REQ_MAX)
        return SP_NONE;
    else
        req[i] = view;

    return i;
}


static void L_animReqDump(int16_t *req)
{//empties request stack

    for(int i=0; i<SP_NUM_OF_REQ_MAX; i++)
        req[i] = SP_NONE;
}
