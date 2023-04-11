//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2023    |   1.0    | CEV_lib std    **/
//**   CEV    |    04-2023    |   1.0.1  | txt conversion **/
//**********************************************************/


//** CEV-04-2023 txt file to data now with parsing - CEV_txtParser

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
#include "CEV_texts.h"
#include "CEV_txtParser.h"

// TODO (drx#1#03/05/17): vérifier la sécurité des fonctions de relecture / ecriture fichier


// LOCAL FUNCTIONS DECLARATION

    // MENU

//converts string to value for button type
static int L_menuButtonTypeToValue(char* string);


    // SUB OBJECTS

        // TEXT BUTTON

//displays text button
static void L_mTextDisp(CEV_MText *text);
//reads text button from file
static void L_mTextTypeRead(FILE* src, CEV_MText* dst, TTF_Font* font);
//reads text button from RWops
static void L_mTextTypeRead_RW(SDL_RWops* src, CEV_MText* dst, TTF_Font* font);
//clears text button content
static void L_mTextClear(CEV_MText *this);
//reads an convert M_IS_TEXT type
static void L_mTextConvertTxtToData(CEV_Text* src, FILE* dst, int index);
//dumps text button content
static void L_menuTxtDump(CEV_MText* this);

        // PIC BUTTON

//displays picture
static void L_mPicDisp(CEV_MPic* pic);
//when clicked
static void L_mPicClic(CEV_MPic* pic);
//reads pic button from file
static void L_mPicTypeRead(FILE* src, CEV_MPic* dst);
//reads pic button from RWops
static void L_mPicTypeRead_RW(SDL_RWops* src, CEV_MPic* dst);
//clears pic button content
static void L_mPicClear(CEV_MPic *this);
//reads an convert M_IS_PIC type
static void L_mPicConvertTxtToData(CEV_Text* src, FILE* dst, char* folder, int index);
//dumps pic button content
static void L_menuPicDump(CEV_MPic* this);


        // SLIDER

//displays slider
static void L_mSlideDisp(CEV_MSlide* slider);
//when clicked
static void L_mSlideMove(CEV_MSlide* slider, int x);
//reads slide from file
static void L_mSlideTypeRead(FILE* src, CEV_MSlide* dst);
//reads slide from RWops
static void L_mSlideTypeRead_RW(SDL_RWops* src, CEV_MSlide* dst);
//clears slider content
static void L_mSlideClear(CEV_MSlide *this);
//reads an convert M_IS_SLIDE type
static void L_mSlideConvertTxtToData(CEV_Text *src, FILE *dst, char *folder, int index);
//dumps slider content
static void L_menuSlideDump(CEV_MSlide* this);


int TEST_menu(void)
{

    CEV_Input* input = CEV_inputGet();
    SDL_Renderer* render = CEV_videoSystemGet()->render;

    CEV_menuConvertTxtToData("menu/menu_option.txt", "menu/menu.mdat");

    CEV_Menu* menu = CEV_menuLoad("menu/menu.mdat");

    CEV_menuDump(menu);

    bool quit = false;
    int select = 0;

    unsigned char bt[2] = {0};

    CEV_menuButtonLink(menu, 2, &bt[0]);
    //CEV_menuButtonLink(menu, 4, &bt[1]);

    while(!quit)
    {

        CEV_inputUpdate();

        if(input->key[SDL_SCANCODE_RIGHT])
        {
            CEV_addModulo(INC, &select, 4);
            input->key[SDL_SCANCODE_RIGHT] = false;
        }

        if(input->key[SDL_SCANCODE_SPACE])
        {
            CEV_menuDump(menu);
            input->key[SDL_SCANCODE_SPACE] = false;
        }

        select = CEV_mouseBoxPtr(menu->buttonPos, menu->numOfButton);

        if(input->key[SDL_SCANCODE_ESCAPE])
            quit = true;

        CEV_menuUpdate(menu, select, input->mouse.button[SDL_BUTTON_LEFT], input->mouse.pos.x);

        SDL_RenderPresent(render);
        SDL_RenderClear(render);

        SDL_Delay(20);
    }

    CEV_menuDestroy(menu);
}


        // USER END FUNCTION

void CEV_menuDump(CEV_Menu *this)
{//dumps structure content
    puts(" - BEGIN - DUMPING CEV_Menu ****");
    printf("Has %u buttons.\n",
            this->numOfButton);


    for(int i=0; i<this->numOfButton; i++)
    {
        CEV_menuButtonDump(&this->button[i]);
    }

    puts(" - END - DUMPING CEV_Menu ****");
}


void CEV_menuButtonDump(CEV_MSelector* this)
{//dumps button structure content
    switch(this->type)
    {
        case M_IS_SLIDE:
            L_menuSlideDump((CEV_MSlide*)this);
        break;

        case M_IS_PIC:
            L_menuPicDump((CEV_MPic*)this);
        break;

        case M_IS_TEXT:
            L_menuTxtDump((CEV_MText*)this);
        break;

        default:
            puts("Unknown button type\n");
        break;
    }

}


CEV_Menu* CEV_menuLoad(const char* fileName)
{//loads CEV_Menu from file

    SDL_RWops* file = SDL_RWFromFile(fileName, "rb");

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : unable to open %s : %s.\n", __FUNCTION__, __LINE__, fileName, SDL_GetError());
        return NULL;
    }

    return CEV_menuLoad_RW(file, true);
}


CEV_Menu* CEV_menuLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads CEV_Menu from RWops

    CEV_Menu *result = calloc(1, sizeof(CEV_Menu));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    CEV_menuTypeRead_RW(src, result);

end:
    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


int CEV_menuTypeRead(FILE* src, CEV_Menu* dst)
{//reads from src and fills dst

    int funcSts = FUNC_OK;

    readWriteErr = 0;

    dst->id = read_u32le(src);

    dst->numOfButton = read_u32le(src);

    dst->button = calloc(dst->numOfButton, sizeof(CEV_MSelector));

    if(IS_NULL(dst->button))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate buttons array : %s.\n",
                __FUNCTION__, __LINE__, strerror(errno));

        funcSts = FUNC_ERR;
    }

    uint32_t fontSize = read_u32le(src);

    CEV_Font* font      = NULL;
    CEV_Capsule lBuffer = {0};

    if (fontSize)
    {//extracting font file

        CEV_capsuleTypeRead(src, &lBuffer);//reading capsule (font)

        if(IS_NULL(lBuffer.data))
        {
            fprintf(stderr, "Err at %s / %d : unable to load embedded file.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
            goto end;
        }
        else if (lBuffer.type != IS_FONT)
        {
            fprintf(stderr, "Warn at %s / %d : embedded file is not font.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
        }

        font = CEV_capsuleExtract(&lBuffer, 1);
    }

    dst->buttonPos = calloc(dst->numOfButton, sizeof(SDL_Rect*));

    if(IS_NULL(dst->buttonPos))
    {
        // TODO (drx#1#03/06/17): renforcer sécurité ici
        fprintf(stderr, "Err at %s / %d : unable to allocate buttonPos : %s.\n",
                __FUNCTION__, __LINE__, strerror(errno));

        funcSts = FUNC_ERR;
    }

    for(int i=0; i< dst->numOfButton; i++)
    {
        uint32_t lType = read_u32le(src);

        switch (lType)
        {
            case M_IS_SLIDE :
                //printf("found slide\n");//debug
                L_mSlideTypeRead(src, &dst->button[i].slide);
                dst->buttonPos[i] = &dst->button[i].slide.blitPos[0];
            break;

            case M_IS_PIC :
                //printf("found pic\n");//debug
                L_mPicTypeRead(src, &dst->button[i].pic);
                dst->buttonPos[i] = &dst->button[i].pic.blitPos;
            break;

            case M_IS_TEXT :
                //printf("found text\n");//debug
                L_mTextTypeRead(src, &dst->button[i].text, font->font);
                dst->buttonPos[i] = &dst->button[i].text.blitPos;

            break;

            default:
                fprintf(stderr, "Err at %s / %d : button type unknown : %s\n", __FUNCTION__, __LINE__, lType);
                funcSts = FUNC_ERR;
            break;

        }
    }

end:
    CEV_fontClose(font);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}


int CEV_menuTypeRead_RW(SDL_RWops* src, CEV_Menu* dst)
{//reads from RWops and fills dst

    int funcSts = FUNC_OK;

    dst->id             = SDL_ReadLE32(src);
    dst->numOfButton    = SDL_ReadLE32(src);
    dst->button         = calloc(dst->numOfButton, sizeof(CEV_MSelector));

    if(IS_NULL(dst->button))
    {
        fprintf(stderr,
                "Err at %s / %d : unable to allocate buttons array : %s.\n",
                __FUNCTION__, __LINE__, strerror(errno));

        funcSts = FUNC_ERR;
    }

    uint32_t fontSize = SDL_ReadLE32(src);

    CEV_Font* font      = NULL;
    CEV_Capsule lBuffer = {0};

    if (fontSize)
    {//extracting font file

        CEV_capsuleTypeRead_RW(src, &lBuffer);//read raw font

        if(IS_NULL(lBuffer.data))
        {
            fprintf(stderr, "Err at %s / %d : unable to load embedded file.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
            goto end;
        }
        else if (lBuffer.type != IS_FONT)
        {
            fprintf(stderr, "Warn at %s / %d : embedded file is not font.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
        }

        font = CEV_capsuleExtract(&lBuffer, 1);
    }

    dst->buttonPos = calloc(dst->numOfButton, sizeof(SDL_Rect*));

    if(IS_NULL(dst->buttonPos))
    {
        // TODO (drx#1#03/06/17): renforcer sécurité ici
        fprintf(stderr, "Err at %s / %d : unable to allocate buttonPos : %s.\n",
                __FUNCTION__, __LINE__, strerror(errno));

        funcSts = FUNC_ERR;
    }

    for(int i=0; i< dst->numOfButton; i++)
    {
        uint32_t lType = SDL_ReadLE32(src);

        switch (lType)
        {
            case M_IS_SLIDE :
                //printf("found slide\n");//debug
                L_mSlideTypeRead_RW(src, &dst->button[i].slide);
                dst->buttonPos[i] = &dst->button[i].slide.blitPos[0];
            break;

            case M_IS_PIC :
                //printf("found pic\n");//debug
                L_mPicTypeRead_RW(src, &dst->button[i].pic);
                dst->buttonPos[i] = &dst->button[i].pic.blitPos;
            break;

            case M_IS_TEXT :
                //printf("found text\n");//debug
                L_mTextTypeRead_RW(src, &dst->button[i].text, font->font);
                dst->buttonPos[i] = &dst->button[i].text.blitPos;

            break;

            default:
                fprintf(stderr, "Err at %s / %d : button type unknown : %d\n", __FUNCTION__, __LINE__, lType);
                funcSts = FUNC_ERR;
            break;

        }
    }

end:
    CEV_fontClose(font);

    return funcSts;
}


void CEV_menuDestroy(CEV_Menu* menu)
{//frees menu content and itself

    if(IS_NULL(menu))
        return;

    for(int i=0; i<menu->numOfButton; i++)
    {
        switch (menu->button[i].type)
        {
            case M_IS_SLIDE :
                L_mSlideClear(&menu->button[i].slide);
            break;

            case M_IS_PIC :
                L_mPicClear(&menu->button[i].pic);
            break;

            case M_IS_TEXT :
                L_mTextClear(&menu->button[i].text);
            break;

            default:
            break;
        }
    }

    free(menu->button);
    free(menu->buttonPos);

    free(menu);
}


int CEV_menuUpdate(CEV_Menu* menu, int selected, char clic, int x)
{//updates menu

    int result = -1;
    menu->edge.value    = clic;
    uint8_t front       = CEV_edgeRise(&menu->edge);

    for(int i = 0; i<menu->numOfButton; i++)
    {
        CEV_MSelector *lSelect = &menu->button[i];

        switch (lSelect->type)
        {
            case M_IS_SLIDE :
                if(clic && (i == selected))
                {
                    L_mSlideMove(&lSelect->slide, x);
                    result = i;
                }

                L_mSlideDisp(&lSelect->slide);
            break;

            case M_IS_PIC :

                if(front && (i == selected))
                {
                    L_mPicClic(&lSelect->pic);
                    result = i;
                }

                L_mPicDisp(&lSelect->pic);
            break;

            case M_IS_TEXT :

                if(front && (i == selected))       //if clicked
                {
                    if(lSelect->text.valuePtr)
                        *lSelect->text.valuePtr ^=1;

                    result = i;
                }

                lSelect->text.value = (i == selected);//hover
                L_mTextDisp(&lSelect->text);
            break;

            default:
            break;
        }
    }

    return result;
}


CEV_MSelector* CEV_menuButtonGet(CEV_Menu* menu, unsigned int index)
{//fetches button

    if(index>=menu->numOfButton)
        return NULL;
    else
        return &menu->button[index];
}


void CEV_menuButtonValueLink(CEV_Menu* menu, unsigned int mastIndex, unsigned int slaveIndex)
{//links 2 buttons' values



    if((mastIndex>=menu->numOfButton)
       || (slaveIndex>=menu->numOfButton))
        return;

    else
        menu->button[mastIndex].comn.valuePtr = &menu->button[slaveIndex].comn.value;

}


void CEV_menuButtonLink(CEV_Menu* menu, unsigned int mastIndex, unsigned char* slave)
{//links button to external

    if((!slave) || (mastIndex>=menu->numOfButton))
        return;

    else
        menu->button[mastIndex].comn.valuePtr = slave;
}


int CEV_menuConvertTxtToData(const char* srcName, const char* dstName)
{//converts parameters file into program friendly data

    int funcSts = FUNC_OK;

    if((srcName == NULL) || (dstName) == NULL)
    {//checking args
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    CEV_Text *src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : cannot create CEV_Text.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    CEV_textDump(src);

    uint32_t    id,
                buttonNum,
                fontSize;

    char lString[FILENAME_MAX],
         fileFolder[FILENAME_MAX],
         hasFolder = CEV_fileFolderNameGet(srcName, fileFolder);

    CEV_Capsule caps = {0};

    FILE *dst = fopen(dstName, "wb");

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    //id
    id = (uint32_t)CEV_txtParseValueFrom(src, "id");
    id = (id & 0x00FFFFFF) | (M_TYPE_ID);
    write_u32le(id, dst);

    //number of button R/W
    buttonNum = (uint32_t)CEV_txtParseValueFrom(src, "btNum");
    write_u32le(buttonNum, dst);

    //font size, 0 means no font
    fontSize = (uint32_t)CEV_txtParseValueFrom(src, "sizeFont");
    write_u32le(fontSize, dst);

    if(fontSize)
    {//if font, insert font into file

        char *fontFileName = CEV_txtParseTxtFrom(src, "font");

        sprintf(lString, "%s%s", fileFolder, fontFileName);

        printf("font is at %s\n", lString);
        CEV_capsuleFromFile(&caps, lString); //loads font file into buffer

        if(caps.type != IS_FONT)
        {
            fprintf(stderr,"Warn at %s / %d : file is not ttf extension.\n", __FUNCTION__, __LINE__);
            funcSts = FUNC_ERR;
        }

        CEV_capsuleTypeWrite(&caps, dst);
        CEV_capsuleClear(&caps);
    }

    for(int i =0; i<buttonNum; i++)
    {
        sprintf(lString, "[%d]type", i);

        uint32_t type = L_menuButtonTypeToValue(CEV_txtParseTxtFrom(src, lString));
        write_u32le(type, dst);

        switch (type)
        {
            case M_IS_PIC :
                L_mPicConvertTxtToData(src, dst, hasFolder? fileFolder: NULL, i);
            break;

            case M_IS_SLIDE :
                L_mSlideConvertTxtToData(src, dst, hasFolder? fileFolder: NULL, i);
            break;

            case M_IS_TEXT :
                L_mTextConvertTxtToData(src, dst, i);
            break;

            default:
                fprintf(stderr, "Err at %s / %d : button %d is of unknown type enum.\n", __FUNCTION__, __LINE__, i);
                funcSts = FUNC_ERR;
            break;
        }
    }

    fclose(dst);

err_1:

    CEV_textDestroy(src);

    return funcSts;
}



//-----Locals functions -------

    //TEXT BUTTON

static void L_mTextDisp(CEV_MText* text)
{//displays text from menu

    SDL_RenderCopy(CEV_videoSystemGet()->render, text->pic[text->value? 1:0], NULL, &text->blitPos);
}


static void L_mTextTypeRead(FILE* src, CEV_MText* dst, TTF_Font* font)
{//reads single text button from file

    char textTemp[256] = ""; //text

    SDL_Color color[2];//colors off/hover

    for(int i=0; i<2; i++)
    {
        color[i].r = read_u8(src);
        color[i].g = read_u8(src);
        color[i].b = read_u8(src);
        color[i].a = read_u8(src);
    }

    dst->pos.x  = read_u32le(src);
    dst->pos.y  = read_u32le(src);
    dst->justif = read_u32le(src);
    dst->scale  = (float)read_u32le(src)/100.0;

    int index = 0;
    //reading text line
    while(textTemp[index++] = (char)read_u8(src));

    for(int i=0; i<2; i++)
    {
        dst->pic[i] = CEV_createTTFTexture(textTemp, font, color[i]);

        if(IS_NULL(dst->pic[i]))
        {
            fprintf(stderr, "Err at %s / %d : unable to create texture from text.\n", __FUNCTION__, __LINE__);
            readWriteErr++;
            return;
        }
    }

    dst->blitPos = CEV_textureDimGet(dst->pic[0]);
    CEV_rectDimScale(&dst->blitPos, dst->scale);
    CEV_dispBlitPos(&dst->blitPos, dst->pos, dst->justif, dst->scale);

    dst->type = M_IS_TEXT;
}


static void L_mTextTypeRead_RW(SDL_RWops* src, CEV_MText* dst, TTF_Font* font)
{//reads single text button from RWops

    char textTemp[256] = ""; //text

    SDL_Color color[2];//colors off/hover

    for(int i=0; i<2; i++)
    {
        color[i].r = SDL_ReadU8(src);
        color[i].g = SDL_ReadU8(src);
        color[i].b = SDL_ReadU8(src);
        color[i].a = SDL_ReadU8(src);
    }

    dst->pos.x  = SDL_ReadLE32(src);
    dst->pos.y  = SDL_ReadLE32(src);
    dst->justif = SDL_ReadLE32(src);
    dst->scale  = (float)SDL_ReadLE32(src)/100.0;

    char index = 0;
    //reading text line
    while((textTemp[index++]) = SDL_ReadU8(src));

    for(int i=0; i<2; i++)
    {
        dst->pic[i] = CEV_createTTFTexture(textTemp, font, color[i]);

        if(IS_NULL(dst->pic[i]))
        {
            fprintf(stderr, "Err at %s / %d : unable to create texture from text.\n", __FUNCTION__, __LINE__);
            readWriteErr++;
            return;
        }
    }

    dst->blitPos = CEV_textureDimGet(dst->pic[0]);
    CEV_rectDimScale(&dst->blitPos, dst->scale);
    CEV_dispBlitPos(&dst->blitPos, dst->pos, dst->justif, dst->scale);

    dst->type = M_IS_TEXT;
}


static void L_mTextConvertTxtToData(CEV_Text* src, FILE* dst, int index)
{//reads and converts M_IS_TEXT type

    double  values[4];

    char parName[50]; //parameter name

    //colors off
    sprintf(parName, "[%d]colorOff", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 4);

    for(int i=0; i<4; i++)
        write_u8((uint8_t)values[i], dst);

    //colors hover
    sprintf(parName, "[%d]colorOn", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 4);

    for(int i=0; i<4; i++)
        write_u8((uint8_t)values[i], dst);

    //position x,y
    sprintf(parName, "[%d]pos", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 2);

    for(int i=0; i<2; i++)
        write_u32le((uint32_t)values[i], dst);

    //justification
    sprintf(parName, "[%d]justif", index);
    write_u32le((uint32_t)CEV_txtParseValueFrom(src, parName), dst);

    //scale
    sprintf(parName, "[%d]scale", index);
    write_u32le((uint32_t)CEV_txtParseValueFrom(src, parName), dst);

    //text line
    sprintf(parName, "[%d]text", index);
    char *lTxt = CEV_txtParseTxtFrom(src, parName);

    size_t length = strlen(lTxt);

    for(int i=0; i<=length; i++)
        write_u8(lTxt[i], dst);
}


static void L_mTextClear(CEV_MText *this)
{//clears content

    for(int i =0; i<2; i++)
        SDL_DestroyTexture(this->pic[i]);

    *this = (CEV_MText){0};
}


static void L_menuTxtDump(CEV_MText* this)
{//dumps text button content

    puts(" - BEGIN - DUMPING CEV_MText ****");

    printf("Value is %u and points to %p\n text is : %s\n",
            this->value, this->valuePtr, this->text);

    printf("Scale is %f with justification %u.\n", this->scale, this->justif);
    printf("Position on screen is : %d, %d \n", this->pos.x, this->pos.y);
    puts("Blit position is :");
    CEV_rectDump(this->blitPos);

    for(int i=0; i<2; i++)
        printf("Texture %d is at %p\n", i, this->pic[i]);

    puts(" - END - DUMPING CEV_MText ****");

}

    //PIC BUTTON

static void L_mPicDisp(CEV_MPic * pic)
{//display

    if(pic->valuePtr != NULL)
        pic->value = *pic->valuePtr;

    pic->clip.x = pic->value * pic->clip.w;
    SDL_Renderer *render = CEV_videoSystemGet()->render;
    SDL_RenderCopy(render, pic->pic, &pic->clip, &pic->blitPos);
}


static void L_mPicClic(CEV_MPic * pic)
{//when clicked

    int lVal = pic->value;

    CEV_addModulo(INC, &lVal, pic->stateNum);

    pic->value = lVal;

    if(pic->valuePtr != NULL)
        *pic->valuePtr = pic->value;

}


static void L_mPicTypeRead(FILE* src, CEV_MPic* dst)
{//single pic button read from file

    dst->stateNum   = read_u32le(src);  // num of button state.
    dst->blitPos.x  = read_u32le(src);  // blit x
    dst->blitPos.y  = read_u32le(src);  // blit y
    dst->type       = M_IS_PIC;

    CEV_Capsule lBuffer = {0};
    CEV_capsuleTypeRead(src, &lBuffer);// fetch pic raw

    dst->pic = CEV_capsuleExtract(&lBuffer, 1);//turns into Texture..hopefully

    if(IS_NULL(dst->pic))
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture : %s\n", __FUNCTION__, __LINE__, IMG_GetError());
        readWriteErr++;
        goto end;
    }

    dst->clip = CEV_textureDimGet(dst->pic);    //texture size is clip
    dst->clip.w /= dst->stateNum;               //clip width is num of state
    CEV_rectDimCopy(dst->clip, &dst->blitPos);  //blit at clip pos


end:
    CEV_capsuleClear(&lBuffer);
    return;
}


static void L_mPicTypeRead_RW(SDL_RWops* src, CEV_MPic* dst)
{//single pic button read from RWops

    dst->stateNum   = SDL_ReadLE32(src);  // num of button state.
    dst->blitPos.x  = SDL_ReadLE32(src);  // blit x
    dst->blitPos.y  = SDL_ReadLE32(src);  // blit y
    dst->type       = M_IS_PIC;

    CEV_Capsule lBuffer = {0};
    CEV_capsuleTypeRead_RW(src, &lBuffer);// fetch pic raw

    dst->pic = CEV_capsuleExtract(&lBuffer, 1);//turns into Texture..hopefully

    if(IS_NULL(dst->pic))
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        goto end;
    }

    dst->clip = CEV_textureDimGet(dst->pic);    //texture size is clip
    dst->clip.w /= dst->stateNum;               //clip width is num of state
    CEV_rectDimCopy(dst->clip, &dst->blitPos);  //blit at clip pos


end:
    CEV_capsuleClear(&lBuffer);
    return;
}


static void L_mPicClear(CEV_MPic *this)
{//clears content

    SDL_DestroyTexture(this->pic);

    *this = (CEV_MPic){0};
}


static void L_mPicConvertTxtToData(CEV_Text *src, FILE *dst, char* folder, int index)
{//extracts an convert M_IS_PIC type

    uint32_t valu32;

    char fileName[FILENAME_MAX],
         parName[50]; //parameter name

    double storage[2];

    //num of state for this button
    sprintf(parName, "[%d]stateNum", index);
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, parName);
    write_u32le(valu32, dst);

    //position
    sprintf(parName, "[%d]pos", index);
    CEV_txtParseValueArrayFrom(src, parName, storage, 2);

    for(int i=0; i<2; i++)
    {
        valu32 = (uint32_t)storage[i];
        write_u32le(valu32, dst);//pos x,y
    }

    //picture
    sprintf(parName, "[%d]picture", index);
    char* picName = CEV_txtParseTxtFrom(src, parName);

    sprintf(fileName,"%s%s", folder, picName);

    CEV_Capsule caps = {0};

    if(CEV_capsuleFromFile(&caps, picName))
    {//on error
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, fileName);
        readWriteErr++;
        goto err_1;
    }

    if(!IS_PIC(caps.type))
    {
        fprintf(stderr,"Warn at %s / %d : file is not picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    CEV_capsuleTypeWrite(&caps, dst);//inserts picture

err_1:

    CEV_capsuleClear(&caps);

    return;
}


static void L_menuPicDump(CEV_MPic* this)
{//dumps pic button content

    puts(" - BEGIN - DUMPING CEV_MPic ****");

    printf("Has %d states\n", this->stateNum);

    printf("Value is %u and points to %p.\n",
            this->value, this->valuePtr);

    puts("Blit position is :");
    CEV_rectDump(this->blitPos);

    puts("Clip position is :");
    CEV_rectDump(this->clip);

    printf("Texture is at %p\n",this->pic);

    puts(" - END - DUMPING CEV_MPic ****");
}

    //SLIDER

static void L_mSlideDisp(CEV_MSlide *slider)
{//display

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if(NOT_NULL(slider->valuePtr))
        slider->blitPos[1].x = CEV_map(*slider->valuePtr, 0, 255, slider->blitPos[0].x, slider->blitPos[0].x + slider->blitPos[0].w - slider->blitPos[1].w);

    for(int i=0; i<2; i++)
        SDL_RenderCopy(render, slider->pic, &slider->clip[i], &slider->blitPos[i]);
}


static void L_mSlideMove(CEV_MSlide *slider, int x)
{//when clicked

    int leftLim  = slider->blitPos[0].x,
        rightLim = slider->blitPos[0].x + slider->blitPos[0].w-1;

    CEV_constraint(leftLim, &x, rightLim);

    //slider->blitPos[1].x = x - slider->blitPos[1].w/2;

    slider->value = (unsigned char)CEV_map(x, leftLim, rightLim, 0, 255);

    if(NOT_NULL(slider->valuePtr))
        *slider->valuePtr = slider->value;
}


static void L_mSlideTypeRead(FILE *src, CEV_MSlide *dst)
{//single slider read from file

    for(int i=0; i<2; i++)//back / fore
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

    CEV_Capsule lBuffer = {0};

    CEV_capsuleTypeRead(src, &lBuffer);// fetching pic capsule

    dst->pic = CEV_capsuleExtract(&lBuffer, 1);

    if(IS_NULL(dst->pic))
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    CEV_capsuleClear(&lBuffer);

    return;
}


static void L_mSlideTypeRead_RW(SDL_RWops *src, CEV_MSlide *dst)
{//single slider read from RWops

    for(int i=0; i<2; i++)//back / fore
    {
        dst->clip[i].x = SDL_ReadLE32(src);
        dst->clip[i].y = SDL_ReadLE32(src);
        dst->clip[i].w = SDL_ReadLE32(src);
        dst->clip[i].h = SDL_ReadLE32(src);

        dst->blitPos[i].w = dst->clip[i].w;
        dst->blitPos[i].h = dst->clip[i].h;
    }

    dst->blitPos[0].x = SDL_ReadLE32(src);
    dst->blitPos[0].y = SDL_ReadLE32(src);

    dst->blitPos[1].x = dst->blitPos[0].x;
    dst->blitPos[1].y = dst->blitPos[0].y + (dst->blitPos[0].h - dst->blitPos[1].h)/2;
    dst->type = M_IS_SLIDE;

    CEV_Capsule lBuffer = {NULL};

    CEV_capsuleTypeRead_RW(src, &lBuffer);// fetching pic capsule

    dst->pic = CEV_capsuleExtract(&lBuffer, 1);

    if(dst->pic == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    CEV_capsuleClear(&lBuffer);

    return;
}


static void L_mSlideClear(CEV_MSlide *this)
{//clears content

    SDL_DestroyTexture(this->pic);

    *this = (CEV_MSlide){0};
}


static void L_mSlideConvertTxtToData(CEV_Text *src, FILE *dst, char *folder, int index)
{//reads an convert M_IS_SLIDE type


    double values[4];

    uint32_t valu32;

    char parName[50],  //parameter name
         fileName[FILENAME_MAX];

    //reading M_SLIDE infos

    //background clip
    sprintf(parName, "[%d]bclip", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 4);
    for(int i=0; i<4; i++)
    {
        valu32 = (uint32_t)values[i];
        write_u32le(valu32, dst);
    }

    //slider clip
    sprintf(parName, "[%d]fclip", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 4);
    for(int i=0; i<4; i++)
    {
        valu32 = (uint32_t)values[i];
        write_u32le(valu32, dst);
    }

    //display pos
    sprintf(parName, "[%d]pos", index);
    CEV_txtParseValueArrayFrom(src, parName, values, 2);
    for(int i=0; i<2; i++)
    {
        valu32 = (uint32_t)values[i];
        write_u32le(valu32, dst);
    }

    //picture file name
    sprintf(parName, "[%d]picture", index);
    char *picName = CEV_txtParseTxtFrom(src, parName);

    CEV_Capsule caps = {0};

    sprintf(fileName, "%s%s", folder, picName);

    //loading picture file into capsule
    if(CEV_capsuleFromFile(&caps, fileName))
    {
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, parName);
        readWriteErr++;
        goto end;
    }

    if(!IS_PIC(caps.type))
    {
        fprintf(stderr,"Warn at %s / %d : file is not picture.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
    }

    CEV_capsuleTypeWrite(&caps, dst);//inserting picture

end:
    CEV_capsuleClear(&caps);

    return;
}


static void L_menuSlideDump(CEV_MSlide* this)
{//dumps pic button content

    puts(" - BEGIN - DUMPING CEV_MSlide ****");

    printf("Value is %u and points to %p.\n",
            this->value, this->valuePtr);

    for(int i=0; i<2; i++)
    {
        printf("%s rects :\n", i? "fore pic" : "back pic");

        puts("Blit position is :");
        CEV_rectDump(this->blitPos[i]);

        puts("Clip position is :");
        CEV_rectDump(this->clip[i]);
    }

    printf("Texture is at %p\n",this->pic);

    puts(" - END - DUMPING CEV_MSlide ****");

}
        //MENU

static int L_menuButtonTypeToValue(char *string)
{//converts string to value for button type
    if(IS_NULL(string))
        return -1;

    char* ref[M_TYPE_LAST] = M_TYPE_NAMES;

    for (int i = 0; i < M_TYPE_LAST; i++)
    {
        if (!strcmp(string, ref[i]))
            return i;
    }

    return -1;//default value
}


