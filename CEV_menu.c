#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "project_def.h"
#include "CEV_api.h"
#include "CEV_menu.h"
#include "CEV_mixSystem.h"
#include "CEV_display.h"
#include "CEV_dataFile.h"
#include "CEV_file.h"
#include "rwtypes.h"
// TODO (drx#1#03/05/17): vérifier la sécurité des fonctions de relecture / ecriture fichier
// TODO (drx#1#03/05/21): Créer chargement depuis RWops




/*---LOCAL FUNCTIONS DECLARATION---*/

/*--CONTROL--*/

/*display text from menu*/
static void L_mtextDisp(CEV_MText *text, TTF_Font *font);

/*when clicked*/
static void L_mpicClic(CEV_MPic * pic);

/*display picture*/
static void L_mpicDisp(CEV_MPic * pic);

/*when clicked*/
static void L_mslideMove(CEV_MSlider *slider, int x);

/*display slider*/
static void L_mslideDisp(CEV_MSlider *slider);


/*--FILE SYSTEM--*/

/*reading*/

/*extracts slide from file*/
static void L_mSlideTypeRead(FILE* src, CEV_MSlider *dst);

/*extracts pic button from file*/
static void L_mPicTypeRead(FILE* src, CEV_MPic* dst);

/*extracts text button from file*/
static void L_mTextTypeRead(FILE* src, CEV_MText* dst);

/*conversion*/

/*reads an convert M_IS_PIC type*/
static void L_menuCopyPicToDat(FILE *src, FILE *dst, char *folder);

/*reads an convert M_IS_SLIDE type*/
static void L_menuCopySlideToDat(FILE *src, FILE *dst, char *folder);

/*reads an convert M_IS_TEXT type*/
static void L_menuCopyTextToDat(FILE *src, FILE *dst);

/*convert string to value for button type*/
static int L_menuButtonTypeToValue(char *string);



/*---menu---*/


int CEV_menuUpdate(CEV_Menu * menu, int selected, char clic, int x)
{/*updating menu*/

    int result = -1;
    menu->edge.value    = clic;
    uint8_t front       = CEV_edgeRise(&menu->edge);

    for(int i = 0; i<menu->numOfButton; i++)
    {
        CEV_Selector *lSelect   = &menu->button[i];

        switch (lSelect->type)
        {
            case M_IS_SLIDE :
                if(clic && (i == selected))
                {
                    L_mslideMove(&lSelect->slide, x);
                    result = i;
                }

                L_mslideDisp(&lSelect->slide);
            break;

            case M_IS_PIC :
                if(front && (i == selected))
                {
                    L_mpicClic(&lSelect->pic);
                    result = i;
                }

                L_mpicDisp(&lSelect->pic);
            break;

            case M_IS_TEXT :
                if(front && (i == selected))       //if clicked
                {
                    if(lSelect->text.valuePtr)
                        *lSelect->text.valuePtr ^=1;

                    result = i;
                }

                lSelect->text.value = (i == selected);//hover
                L_mtextDisp(&lSelect->text, menu->font->font);
            break;
        }
    }

    return result;
}


void CEV_menuFree(CEV_Menu *menu)
{/*frees menu*/

    if(IS_NULL(menu))
        return;

    for(int i=0; i<menu->numOfButton; i++)
    {
        switch (menu->button[i].type)
        {
            case M_IS_SLIDE :
                SDL_DestroyTexture(menu->button[i].slide.pic);
            break;

            case M_IS_PIC :
                SDL_DestroyTexture(menu->button[i].pic.pic);
            break;

            case M_IS_TEXT :
                free(menu->button[i].text.text);
            break;
        }
    }

    free(menu->button);
    free(menu->buttonPos);

    if(menu->font != NULL)
        CEV_fontClose(menu->font);
    free(menu);
}


CEV_Selector* CEV_menuButtonGet(CEV_Menu *menu, unsigned int index)
{/*fetch button*/

    if(index>=menu->numOfButton)
        return NULL;
    else
        return &menu->button[index];
}


void CEV_menuButtonValueLink(CEV_Menu * menu, unsigned int mastIndex, unsigned int slaveIndex)
{/*link 2buttons' values*/



    if((mastIndex>=menu->numOfButton)
       || (slaveIndex>=menu->numOfButton))
        return;

    else
        menu->button[mastIndex].comn.valuePtr = &menu->button[slaveIndex].comn.value;

}


void CEV_menuButtonLink(CEV_Menu *menu, unsigned int mastIndex, unsigned char* slave)
{

    if((!slave) || (mastIndex>=menu->numOfButton))
        return;

    else
        menu->button[mastIndex].comn.valuePtr = slave;
}



/*---FILE SYSTEM FUNCTION IMPLEMENTATION---*/

int CEV_convertMenuTxtToData(const char* srcName, const char* dstName)
{/*convert parameters file into programm friendly data.*/

    /*---DECLARATIONS---*/
    int funcSts = FUNC_OK;

    FILE *src   = NULL,
         *dst   = NULL;

    uint32_t    buttonNum,
                fontSize;

    char lString[FILENAME_MAX],
         lFileFolder[FILENAME_MAX],
         hasFolder = CEV_fileFolderNameGet(srcName, lFileFolder);

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};


        /*---PRL---*/

    if((srcName == NULL) || (dstName) == NULL)
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        funcSts = ARG_ERR;
        goto err_1;
    }

    readWriteErr = 0;

    /*EXE*/

    src = fopen(srcName, "r");
    dst = fopen(dstName, "wb");

    if ((src == NULL) || (dst == NULL))
    {
        fprintf(stderr, "Err at %s / %d : cannot open file.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    rewind(src);

    /*number of button R/W*/
    fscanf(src, "%u\n", &buttonNum);
    write_u32le(buttonNum, dst);

    /*font size, 0 means no font*/
    fscanf(src, "%u\n", &fontSize);
    write_u32le(fontSize, dst);

    if (fontSize)
    {/*if font, insert it in file*/
        fgets(lString, sizeof(lString)-1, src);/*gets font filename*/
        CEV_stringEndFormat(lString);

        if (hasFolder)
        {
            strcat(lFileFolder, lString);
            strcpy(lString, lFileFolder);
        }

        CEV_capsuleLoad(&lBuffer, lString);/*loads font file into buffer*/

        if(lBuffer.type != IS_FONT)
        {
            fprintf(stderr,"Warn at %s / %d : file is not ttf extension.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
        }

        if(lBuffer.data == NULL)
        {
            fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, lString);
            funcSts = FUNC_ERR;
            goto err_1;
        }

        CEV_capsuleWrite(&lBuffer, dst);
    }

    for(int i =0; i<buttonNum; i++)
    {
        uint32_t type;

        CEV_fileFolderNameGet(srcName, lFileFolder);

        fgets(lString, sizeof(lString)-1, src);/*gets button type*/
        CEV_stringEndFormat(lString);

        type = L_menuButtonTypeToValue(lString);
        write_u32le(type, dst);

        switch (type)
        {
            case M_IS_PIC :
                L_menuCopyPicToDat(src, dst, hasFolder? lFileFolder: NULL);
            break;

            case M_IS_SLIDE :
                L_menuCopySlideToDat(src, dst, hasFolder? lFileFolder: NULL);
            break;

            case M_IS_TEXT :
                L_menuCopyTextToDat(src, dst);
            break;

            default:
                readWriteErr++;
                goto err_1;
            break;
        }
    }



    if(readWriteErr)
        funcSts = FUNC_ERR;
err_1:

    if(lBuffer.data != NULL)
        CEV_capsuleClear(&lBuffer);

    if(src != NULL)
        fclose(src);

    if(dst != NULL)
        fclose(dst);

    return funcSts;
}


CEV_Menu * CEV_menuLoad(const char* fileName)
{/*loads menu.mdat file*/

/*---DECLARATIONS---*/

    CEV_Menu        *result = NULL;
    FILE            *file   = NULL;

    /*---PRL---*/

    if(fileName == NULL)
    {/*bad arg*/
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*openning file*/
    file = fopen(fileName, "rb");

    if(file == NULL)
    {/*bad arg*/
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto exit;
    }

    /*---EXECUTION---*/

    result = CEV_menuLoadf(file);

    if(result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    /*---POST---*/

exit :

    if(file != NULL)
        fclose(file);

    return result;
}


CEV_Menu * CEV_menuLoadf(FILE *src)
{/*loads from opened data file*/
// TODO (drx#1#03/04/17): voir pour les ratios de texte à ajouter

    /*---DECLARATIONS---*/

    uint32_t    fontSize;

    CEV_Menu*result = NULL;

    SDL_RWops * ops = NULL;

    TTF_Font *font = NULL;

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};

    readWriteErr = 0;

    result = calloc(1, sizeof(CEV_Menu));

    if(result == NULL)
    {/*error*/
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto exit;
    }

    result->numOfButton = read_u32le(src);

    result->button = calloc(result->numOfButton, sizeof(CEV_Selector));

    fontSize = read_u32le(src);

    if (fontSize)
    {/*extracting font file*/

        CEV_capsuleRead(src, &lBuffer);//read raw font

        if(lBuffer.data == NULL)
        {
            fprintf(stderr, "Err at %s / %d : unable to load embedded file.\n", __FUNCTION__, __LINE__);
            goto exit;
        }
        else if (lBuffer.type != IS_FONT)
            fprintf(stderr, "Warn at %s / %d : embedded file is not font.\n", __FUNCTION__, __LINE__);

        ops = SDL_RWFromMem(lBuffer.data,lBuffer.size);//turn into virtual file

        if(ops == NULL)
        {
            fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
            goto err_1;
        }

        font = TTF_OpenFontRW(ops, 1, fontSize);//load font from virtual

        if(font == NULL)
        {
            fprintf(stderr, "Err at %s / %d : unable to load font : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
            goto err_2;
        }

        result->font = calloc(1, sizeof(CEV_Font));

        if(result->font == NULL)
        {
            fprintf(stderr, "Err at %s / %d : unable to allocate font memory : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
            goto err_3;
        }

        result->font->font        = font;
        result->font->virtualFile = lBuffer.data;
    }

    result->buttonPos = calloc(result->numOfButton, sizeof(SDL_Rect*));

    if(result->buttonPos == NULL)
    {
// TODO (drx#1#03/06/17): renforcer sécurité ici
        fprintf(stderr, "Err at %s / %d : unable to allocate button memory : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
    }


    for(int i=0; i< result->numOfButton; i++)
    {
        uint32_t lType = read_u32le(src);

        switch (lType)
        {
            case M_IS_SLIDE :
                //printf("found slide\n");/*debug*/
                L_mSlideTypeRead(src, &result->button[i].slide);
                result->buttonPos[i] = &result->button[i].slide.blitPos[0];
            break;

            case M_IS_PIC :
                //printf("found pic\n");/*debug*/
                L_mPicTypeRead(src, &result->button[i].pic);
                result->buttonPos[i] = &result->button[i].pic.blitPos;
            break;

            case M_IS_TEXT :
                //printf("found text\n");/*debug*/
                L_mTextTypeRead(src, &result->button[i].text);
                TTF_SizeText (result->font->font, result->button[i].text.text, &result->button[i].text.blitPos.w, &result->button[i].text.blitPos.h);
                CEV_rectAroundPoint(result->button[i].text.pos, &result->button[i].text.blitPos);
                result->buttonPos[i] = &result->button[i].text.blitPos;

            break;

            default:
                fprintf(stderr, "Err at %s / %d : button type unknown : %s\n", __FUNCTION__, __LINE__, lType);
            break;

        }
    }

    fclose(src);
    //SDL_RWclose(ops);

    return result;

err_3:
    TTF_CloseFont(font);

err_2:
    SDL_RWclose(ops);

err_1:
    //CEV_capsuleClear(&lBuffer);

exit:
    return NULL;

}

/*-----Local loading functions -------*/

void L_mSlideTypeRead(FILE *src, CEV_MSlider *dst)
{

    /*---DECLARATIONS---*/

    SDL_RWops* ops = NULL;

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};

    /*---EXECUTION---*/

    CEV_capsuleRead(src, &lBuffer);// fetch pic raw

    if(lBuffer.data == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load embedded file.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else if (!IS_PIC(lBuffer.type))
        fprintf(stderr, "Err at %s / %d : embedded file is not picture.\n", __FUNCTION__, __LINE__);

    ops = SDL_RWFromMem(lBuffer.data,lBuffer.size);//turn into virtual file

    if(ops == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        readWriteErr++;
        goto err_1;
    }

    dst->pic = IMG_LoadTexture_RW(CEV_videoSystemGet()->render, ops, 1);//turn into Texture

    if(dst->pic == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        readWriteErr++;
        goto err_2;
    }

    for(int i=0; i<2; i++)
    {
        dst->clip[i].x = read_u32le(src);
        dst->clip[i].y = read_u32le(src);
        dst->clip[i].w = read_u32le(src);
        dst->clip[i].h = read_u32le(src);

        dst->blitPos[i].w = dst->clip[i].w;
        dst->blitPos[i].h = dst->clip[i].h;
    }

    dst->blitPos[0].x = read_u32le(src);
    dst->blitPos[0].y = read_u32le(src);

    dst->blitPos[1].y = dst->blitPos[0].y + (dst->blitPos[0].h - dst->blitPos[1].h)/2;
    dst->type = M_IS_SLIDE;
    /*---POST---*/

err_2:
    //SDL_RWclose(ops);

err_1:
    CEV_capsuleClear(&lBuffer);

exit:
    return;
}


void L_mPicTypeRead(FILE* src, CEV_MPic* dst)
{

    /*---DECLARATIONS---*/

    SDL_RWops* ops = NULL;

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};

    /*---EXECUTION---*/

    CEV_capsuleRead(src, &lBuffer);// fetch pic raw

    if(lBuffer.data == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load embedded file.\n", __FUNCTION__, __LINE__);
        goto exit;
    }
    else if (!IS_PIC(lBuffer.type))
        fprintf(stderr, "Warn at %s / %d : embedded file is not picture.\n", __FUNCTION__, __LINE__);

    ops = SDL_RWFromMem(lBuffer.data,lBuffer.size);//turn into virtual file

    if(ops == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s\n", __FUNCTION__, __LINE__, SDL_GetError());
        readWriteErr++;
        goto err_1;
    }

    dst->pic = IMG_LoadTexture_RW(CEV_videoSystemGet()->render, ops, 1);//turn into Texture

    if(dst->pic == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture : %s\n", __FUNCTION__, __LINE__, IMG_GetError());
        readWriteErr++;
        goto err_2;
    }

    dst->picNum = read_u32le(src);
    dst->clip.x = read_u32le(src);
    dst->clip.y = read_u32le(src);
    dst->clip.w = read_u32le(src);
    dst->clip.h = read_u32le(src);

    dst->blitPos.w = dst->clip.w;
    dst->blitPos.h = dst->clip.h;

    dst->blitPos.x = read_u32le(src);
    dst->blitPos.y = read_u32le(src);

    dst->type = M_IS_PIC;


    /*---POST---*/

err_2:
    //SDL_RWclose(ops);

err_1:
    CEV_capsuleClear(&lBuffer);

exit:
    return;
}


void L_mTextTypeRead(FILE* src, CEV_MText *dst)
{

        /*---DECLARATIONS---*/

    uint32_t textSize = 0;

        /*---EXECUTION---*/

    dst->color[0].r = read_u8(src);
    dst->color[0].g = read_u8(src);
    dst->color[0].b = read_u8(src);
    dst->color[0].a = read_u8(src);
    dst->color[1].r = read_u8(src);
    dst->color[1].g = read_u8(src);
    dst->color[1].b = read_u8(src);
    dst->color[1].a = read_u8(src);

    dst->pos.x = read_u32le(src);
    dst->pos.y = read_u32le(src);
    dst->justif = read_u32le(src);
    dst->scale = (float)read_u32le(src)/100.0;

    textSize = read_u32le(src);

    dst->text = calloc(textSize, sizeof(char));

    if(dst->text)
    {
        for (int i=0; i<textSize; i++)
            dst->text[i] = read_u8(src);
    }
    else
        readWriteErr++;

    dst->type = M_IS_TEXT;

}

/*----LOCAL CONTROL FUNCTIONS----*/

/*---text---*/

static void L_mtextDisp(CEV_MText *text, TTF_Font *font)
{/*display text from menu*/

    float lScale = (text->value)? text->scale : 1.0;

    TTF_SizeText(font, text->text, &text->blitPos.w, &text->blitPos.h);

    CEV_dispBlitPos(&text->blitPos, text->pos, text->justif, lScale);

    CEV_dispText(text->text, font, text->color[text->value], text->pos, text->justif, lScale);
}

/*---picture---*/

static void L_mpicClic(CEV_MPic * pic)
{/*when clicked*/

    int lVal = pic->value;

    CEV_addModulo(INC, &lVal, pic->picNum);

    pic->value = lVal;

    if(pic->valuePtr != NULL)
        *pic->valuePtr = pic->value;

}


static void L_mpicDisp(CEV_MPic * pic)
{/*display*/

    if(pic->valuePtr != NULL)
        pic->value = *pic->valuePtr;

    pic->clip.x = pic->value * pic->clip.w;
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    SDL_RenderCopy(render, pic->pic, &pic->clip, &pic->blitPos);
}


/*----slider----*/

static void L_mslideMove(CEV_MSlider *slider, int x)
{/*when clicked*/

    int leftLim  = slider->blitPos[0].x,
        rightLim = slider->blitPos[0].x + slider->blitPos[0].w-1;

    CEV_constraint(leftLim, &x, rightLim);

    //slider->blitPos[1].x = x - slider->blitPos[1].w/2;

    slider->value = (unsigned char)CEV_map(x, leftLim, rightLim, 0, 255);

    if(slider->valuePtr != NULL)
        *slider->valuePtr = slider->value;
}


static void L_mslideDisp(CEV_MSlider *slider)
{/*display*/

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    slider->blitPos[1].x = CEV_map(*slider->valuePtr, 0, 255, slider->blitPos[0].x, slider->blitPos[0].x + slider->blitPos[0].w - slider->blitPos[1].w);

    for(int i=0; i<2; i++)
        SDL_RenderCopy(render, slider->pic, &slider->clip[i], &slider->blitPos[i]);
}


/*----LOCALS FILE FUNCTIONS----*/

static void L_menuCopyPicToDat(FILE *src, FILE *dst, char* folder)
{/*reads an convert M_IS_PIC type*/

    /*---DECLARATIONS---*/

    uint32_t picNum,
             clip[4],
             pos[2];

    char lString[FILENAME_MAX],
         lFileName[FILENAME_MAX];

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};

        /*---EXECUTION---*/

    /*reading M_PIC infos*/

    fgets(lString, sizeof(lString)-1, src);/*gets image file name*/
    CEV_stringEndFormat(lString);

    if(folder != NULL)
    {
        strcpy(lFileName, folder);
        strcat(lFileName, lString);
    }
    else
        strcpy(lFileName, lString);

    CEV_capsuleLoad(&lBuffer, lFileName);/*loads picture file into buffer*/

    fscanf(src, "%u\n", &picNum);
    fscanf(src, "%u %u %u %u\n", &clip[0], &clip[1], &clip[2], &clip[3]);
    fscanf(src, "%u %u\n", &pos[0], &pos[1]);

    if( !IS_PIC(lBuffer.type))
    {
        fprintf(stderr,"Warn at %s / %d : file is not picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    if(lBuffer.data == NULL)
    {
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, lString);
        readWriteErr++;
        goto err_1;
    }

    /*writting M_PIC data*/

    CEV_capsuleWrite(&lBuffer, dst);//inserts picture

    write_u32le(picNum, dst); //number of clips

    for(int i = 0; i<4; i++)
        write_u32le(clip[i], dst);//clip

    for(int i = 0; i<2; i++)
        write_u32le(pos[i], dst);//pos

    /*---POST---*/

    CEV_capsuleClear(&lBuffer);

err_1 :

    return;
}


static void L_menuCopySlideToDat(FILE *src, FILE *dst, char *folder)
{/*reads an convert M_IS_SLIDE type*/

    /*---DECLARATIONS---*/

    int clip[2][4],
             pos[2];

    char lString[FILENAME_MAX],
         lFileName[FILENAME_MAX];

    CEV_Capsule lBuffer = {.type = 0,
                            .size = 0,
                            .data = NULL};

        /*---EXECUTION---*/

    /*reading M_PIC infos*/

    fgets(lString, sizeof(lString)-1, src);/*gets image file name*/
    CEV_stringEndFormat(lString);

    if(folder != NULL)
    {
        strcpy(lFileName, folder);
        strcat(lFileName, lString);
    }
    else
        strcpy(lFileName, lString);

    CEV_capsuleLoad(&lBuffer, lFileName);/*loads picture file into buffer*/

    for(int i = 0; i<2; i++)
        fscanf(src, "%d %d %d %d\n", &clip[i][0], &clip[i][1], &clip[i][2], &clip[i][3]);

    fscanf(src, "%d %d\n", &pos[0], &pos[1]);

    if( !IS_PIC(lBuffer.type))
    {
        fprintf(stderr,"Warn at %s / %d : file is not picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    if(lBuffer.data == NULL)
    {
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, lString);
        readWriteErr++;
        goto err_1;
    }

    /*writting M_PIC data*/

    CEV_capsuleWrite(&lBuffer, dst);//inserts picture

    for(int i = 0; i<2; i++)
        for(int j=0; j<4; j++)
            write_u32le(clip[i][j], dst);//clip

    for(int i = 0; i<2; i++)
        write_u32le(pos[i], dst);//pos

    /*---POST---*/

    CEV_capsuleClear(&lBuffer);

err_1 :

    return;
}


static void L_menuCopyTextToDat(FILE *src, FILE *dst)
{/*reads an convert M_IS_TEXT type*/

        /*---DECLARATIONS---*/

    uint8_t     color[2][4];
    int    temp[4],
                xPos, yPos, scale, justif;

    char lString[50];

        /*---EXECUTION---*/

    /*reading infos*/
    for(int i=0; i<2; i++)
    {
        fscanf(src, "%d %d %d %d\n", &temp[0], &temp[1], &temp[2], &temp[3]);

        for(int j=0; j<4; j++)
            color[i][j] = temp[j];
    }

    fscanf(src, "%d %d %d %d\n", &xPos, &yPos, &justif, &scale);

    /*writting dat infos*/
    for(int i=0; i<2; i++)
        for(int j=0; j<4; j++)
            write_u8(color[i][j], dst);

    write_u32le(xPos, dst);
    write_u32le(yPos, dst);
    write_u32le(justif, dst);
    write_u32le(scale, dst);

    fgets(lString, sizeof(lString)-1, src);
    CEV_stringEndFormat(lString);

    int length = strlen(lString);

    write_s32le(length+1, dst);

        for(int j=0; j<=length; j++)
            write_u8(lString[j], dst);
}


static int L_menuButtonTypeToValue(char *string)
{/*convert string to value for button type*/

   char* ref[M_TYPE_NUM] = M_TYPE_LIST;

    for (int i = 0; i < M_TYPE_NUM; i++)
    {
        if (!strcmp(string, ref[i]))
            return i;
    }

    return -1;//default value
}


