//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  20-05-2016   |   1.0    |    SDL2 rev    **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**********************************************************/


// TODO (drx#1#): modifier les typeread pour lire et remplir sans allouer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "project_def.h"
#include "CEV_mixSystem.h"
#include "CEV_scroll.h"
#include "CEV_display.h"
#include "CEV_dataFile.h"
#include "CEV_file.h"
#include "CEV_api.h"
#include "CEV_texts.h"
#include "CEV_txtParser.h"
#include "rwtypes.h"

/*LOCAL FUNCTIONS DECLARATION*/


/** \brief updates upward scrolling.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return int : 1 while scrolling, 0 when done.
 */
static int L_scrollUp(CEV_ScrollText *in);


/** \brief updates downward scrolling.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return int : 1 while scrolling, 0 when done.
 */
static int L_scrollDown(CEV_ScrollText *in);


/** \brief updates leftward scrolling.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return int : 1 while scrolling, 0 when done.
 */
static int L_scrollLeft(CEV_ScrollText *in);


/** \brief updates rightward scrolling.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return int : 1 while scrolling, 0 when done.
 */
static int L_scrollRight(CEV_ScrollText *in);


/** \brief displays all lines.
 *
 * \param in : CEV_ScrollText* to display.
 *
 * \return uint8_t : 0 if display failed.
 */
static uint8_t L_scrollDisplay(CEV_ScrollText *in);


/** \brief displays single line.
 *
 * \param ligne : L_ScrollTextLine* to display.
 *
 * \return uint8_t : 0 if display failed.
 *
 */
static uint8_t L_scrollDispThisLine(L_ScrollTextLine *ligne);


static uint8_t L_stwaDisplay(CEV_ScrollText *in);


/** \brief convert scroll mode as string (from file) into numeric.
 *
 * \param mode : char* as string holding parameter.
 *
 * \return uint32_t as mode's numeric value.
 */
static uint32_t L_scrollModeStringToValue(char* mode);


/** \brief inserts text capsule into data file.
 *
 * \param src : FILE* to read from.
 * \param dst : FILE* to write into.
 *
 * \return int of standard function status.
 *
 */
static int L_scrollInsertText(FILE *src, FILE* dst);


/** \brief reads scroll header from data file.
 *
 * \param src : FILE* to read from.
 * \param dst : CEV_ScrollText* to fill.
 *
 * \return void
 */
static void L_scrollTypeHeaderRead(FILE* src, CEV_ScrollText *dst);


/** \brief reads scroll header from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_ScrollText* to fill.
 *
 * \return void
 */
static void L_scrollTypeHeaderRead_RW(SDL_RWops* src, CEV_ScrollText *dst);


void TEST_scroll(void)
{
    int result;
    CEV_ScrollText *scroll = NULL;
    SDL_Renderer * render = CEV_videoSystemGet()->render;
    //CEV_Font *font = CEV_fontFetch(1, "compiled.dat");

    puts("convertion");
    result = CEV_scrollConvertTxtToData("scroll/scrollEditable.txt", "scroll/scroll.scl");

    if (result != FUNC_OK)
    {
        puts("erreur de convertion de fichier");
        return;
    }
    else
        puts("convertion sans faute...");


    puts("testing scrollLoad");
/*
    CEV_Capsule caps;

    if(CEV_capsuleFromFile(&caps, "scroll.scl") != FUNC_OK)
    {
        fprintf(stderr, "Err at %s / %d : capsule load foire.\n", __FUNCTION__, __LINE__ );
    }*/

    scroll = /*CEV_capsuleExtract(&caps, 1);//*/CEV_scrollLoad("scroll/scroll.scl");

    if(!scroll)
    {
        puts("foirage sur scroll");
        return;
    }
    //else
        //CEV_scrollDump(scroll);

    //scroll->fontSize = 200;
    //scroll->speed = 2;
    //CEV_scrollSpaceSet(scroll, 50);
    CEV_scrollPosSet(scroll, SCREEN_WIDTH/2);

    CEV_scrollDump(scroll);

    SDL_SetRenderDrawColor(render, 0x00, 0x00, 0x00, 0x00); /*background color*/

    /**UP**/
   char run = 1;/*
    while(run)
    {
        SDL_RenderClear(render);
        run = CEV_scrollUpdate(scroll);
        SDL_RenderPresent(render); //shows it all
        SDL_Delay(10);
    }*/

    /**DOWN**/
/*    run = 1;
    CEV_scrollModeSet(scroll, SCROLL_DOWN);
    CEV_scrollRestart(scroll);

    CEV_scrollDump(scroll);

    while(run)
    {
        SDL_RenderClear(render);
        run = CEV_scrollUpdate(scroll);
        SDL_RenderPresent(render); //shows it all
        SDL_Delay(10);
    }*/

    /**RIGHT**/
 /*   run = 1;
    CEV_scrollModeSet(scroll, SCROLL_RIGHT);
    CEV_scrollPosSet(scroll, SCREEN_HEIGHT/2);
    CEV_scrollRestart(scroll);

    CEV_scrollDump(scroll);

    while(run)
    {
        SDL_RenderClear(render);
        run = CEV_scrollUpdate(scroll);
        SDL_RenderPresent(render); //shows it all
        SDL_Delay(10);
    }
*/
    /**LEFT**/
/*    run = 1;
    CEV_scrollModeSet(scroll, SCROLL_LEFT);
    CEV_scrollRestart(scroll);

    CEV_scrollDump(scroll);

    while(run)
    {
        SDL_RenderClear(render);
        run = CEV_scrollUpdate(scroll);
        SDL_RenderPresent(render); //shows it all
        SDL_Delay(10);
    }
*/
    /**STWA**/
    run = 1;
    //CEV_scrollModeSet(scroll, SCROLL_STWA);
    CEV_scrollPosSet(scroll, SCREEN_WIDTH/2);
    CEV_scrollRestart(scroll);

    CEV_scrollDump(scroll);

    while(run)
    {
        SDL_RenderClear(render);
        run = CEV_scrollUpdate(scroll);
        SDL_RenderPresent(render); /*shows it all*/
        SDL_Delay(10);
    }



    CEV_scrollDestroy(scroll);
    //CEV_fontClose(font);
}

/*USER END FUNCTIONS*/

CEV_ScrollText* CEV_scrollCreate(char** texts, unsigned int num, TTF_Font* font, SDL_Color colour)
{//create a text scroll structure

    CEV_ScrollText *result = NULL;

    result = malloc(sizeof(*result));

    if (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__,  strerror(errno));
        goto err_1;
    }

    result->texts = malloc(num * sizeof(L_ScrollTextLine));

    if (result->texts == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_2;
    }
    else
    {
        for(int i=0; i<num; i++)
            result->texts[i].img = NULL;
    }

        printf("reached line %d.\n", __LINE__);

    for (int i=0; i<num; i++)
    {
        result->texts[i].blitPos = (SDL_Rect){0, 0, 0, 0};

        result->texts[i].img = CEV_createTTFTexture(texts[i], font, colour);

        if (result->texts[i].img != NULL)
            SDL_QueryTexture(result->texts[i].img, NULL, NULL,
                             &result->texts[i].blitPos.w, &result->texts[i].blitPos.h);
        else
        {
            fprintf(stderr, "Err at %s / %d : creating texture.\n", __FUNCTION__, __LINE__);
            goto err_3;
        }
    }

    result->lineFrom    = 0;
    result->lineTo      = 1;
    result->lineNb      = num;
    result->color       = colour;
    result->render      = CEV_videoSystemGet()->render;
    result->renderDim.x = 0;
    result->renderDim.y = 0;
    result->renderDim.w = SCREEN_WIDTH;
    result->renderDim.h = SCREEN_HEIGHT;

    return result;


err_3:
    for (int i=0; i< num; i++)
    {
        if (result->texts[i].img != NULL)
            SDL_DestroyTexture(result->texts[i].img);
    }

    free(result->texts);

err_2:
    free(result);

err_1:
    return NULL;
}


void CEV_scrollDestroy(CEV_ScrollText *in)
{//libération d'une structure CEV_ScrollText

    for (int i=0; i<in->lineNb; i++)
        SDL_DestroyTexture(in->texts[i].img);

    free(in->texts);

    free(in);
}


void CEV_scrollClear(CEV_ScrollText *in)
{//libération du contenu + mise à 0/NULL

    for (int i=0; i<in->lineNb; i++)
        SDL_DestroyTexture(in->texts[i].img);

    free(in->texts);

    in->texts       = NULL;
    in->color       = (SDL_Color){0, 0, 0, 0};
    in->fontSize    =
    in->lineAct     =
    in->lineNb      =
    in->lineFrom    =
    in->lineTo      =
    in->space       =
    in->mode        = 0;
}


void CEV_scrollDump(CEV_ScrollText *in)
{//dumps struct content into stdout

    printf("dumping scroll at %p\n", in);
    printf("holds %u lines\n", in->lineNb);

    for(int i=0; i< in->lineNb; i++)
    {
        printf("speed = %d\n", in->speed);
        printf("space = %d\n", in->space);
        printf("index %d blit pos x=%d, y = %d\n", i, in->texts[i].blitPos.x, in->texts[i].blitPos.y);
        printf("x = %u, y = %u, w = %u, h = %u\n", in->texts[i].blitPos.x,
                                                    in->texts[i].blitPos.y,
                                                    in->texts[i].blitPos.w,
                                                    in->texts[i].blitPos.h);
        printf("texture at %p\n", in->texts[i].img);
    }
}

    /*CONTROL FUNCTIONS*/

void CEV_scrollModeSet(CEV_ScrollText *in, int mode)
{//sets scroll mode
    in->mode = mode;
}


void CEV_scrollSpaceSet(CEV_ScrollText *in, unsigned int pxl)
{//sets text spacing
    in->space = pxl;
}


void CEV_scrollRestart(CEV_ScrollText* in)
{//restarts scrolltext
    in->lineFrom = 0;
    in->lineTo  = 1;
}


void CEV_scrollPosSet(CEV_ScrollText* in, int pos)
{//sets scroll text position

    in->pos = pos;

    for(int i=0; i<in->lineNb; i++)
    {
        int wt = in->texts[i].blitPos.w,
            ht = in->texts[i].blitPos.h;

        switch (in->mode)
        {//sets up start positions
            case SCROLL_STWA:
            case SCROLL_UP:
                CEV_rectPosCopy((SDL_Rect){.x = pos - wt/2, .y = in->renderDim.h},
                                &in->texts[i].blitPos);
            break;

            case SCROLL_DOWN:
                CEV_rectPosCopy((SDL_Rect){.x = pos - wt/2, .y = -ht},
                                &in->texts[i].blitPos);
            break;

            case SCROLL_LEFT:
                CEV_rectPosCopy((SDL_Rect){.x = in->renderDim.w, .y = pos - ht/2},
                                &in->texts[i].blitPos);
            break;

            case SCROLL_RIGHT:
                CEV_rectPosCopy((SDL_Rect){.x = -wt, .y = pos - ht/2},
                                &in->texts[i].blitPos);
            break;
        }
    }
}


int CEV_scrollUpdate(CEV_ScrollText *in)
{//automatic scroll text update & display

    //int(*L_scrollShow[4])(CEV_ScrollText*) = {L_scrollUp, L_scrollDown, L_scrollLeft, L_scrollRight};

    int sts = 1;//on activity && no error = 1, else 0

    switch (in->mode)
    {
        case SCROLL_STWA :
        case SCROLL_UP :
            sts = L_scrollUp(in);
        break;

        case SCROLL_DOWN :
            sts = L_scrollDown(in);
        break;

        case SCROLL_RIGHT :
            sts = L_scrollRight(in);
        break;

        case SCROLL_LEFT :
            sts = L_scrollLeft(in);
        break;


        default:
            sts = 0;
            fprintf(stderr, "Warn at %s / %d : scroll mode error.\n", __FUNCTION__, __LINE__);
            goto exit;
        break;
    }

    if(in->mode == SCROLL_STWA)
        L_stwaDisplay(in);
    else if(!L_scrollDisplay(in))
        sts = 0; //stops scrolling on error

exit:

    return sts;
}



    /*FILES FUNCTIONS*/


int CEV_scrollConvertTxtToData(const char* srcName, const char* dstName)
{//convert natural .txt file into scroll.srl file


    if((srcName == NULL) || (dstName == NULL))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    CEV_Text *src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        funcSts = FUNC_ERR;
        goto end;
    }

    CEV_textDump(src);

    readWriteErr = 0;

    FILE* dst = fopen(dstName, "wb");

    if (IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    uint32_t valu32   = 0;
    double temp[4] ={0.0};

    char *fontName,
         folder[FILENAME_MAX],
         fileName[FILENAME_MAX],
         hasFolder = CEV_fileFolderNameGet(srcName, folder);

    //id
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "id");
    valu32 = (valu32 & 0x00FFFFFF) | SCROLL_TYPE_ID;
    write_u32le(valu32, dst);

    //font size (pxl)
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "sizeFont");
    write_u32le(valu32, dst);

    //text color rgba
    CEV_txtParseValueArrayFrom(src, "color", temp, 4);
    for(int i=0; i<4; i++)
        write_u8((uint8_t)temp[i], dst);

    //scroll mode
    fontName = CEV_txtParseTxtFrom(src, "mode");
    valu32 = L_scrollModeStringToValue(fontName);
    write_u32le(valu32, dst);
    //spacing
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "space");
    write_u32le(valu32, dst);

    //speed
    valu32 = (uint32_t)CEV_txtParseValueFrom(src, "vel");
    write_u32le(valu32, dst);


    //inserting CEV_Text
    valu32 = CEV_txtParseIndexGetFrom(src, "text");
    //inserting Text capsule
    CEV_Text *srcTxt = CEV_textCreate(0, 10);

    for(int i= valu32+1; i< src->linesNum; i++)
        CEV_textAppend(srcTxt, CEV_textRead(src, i));

    CEV_textTypeWrite(srcTxt, dst);
    CEV_textDestroy(srcTxt);

    //fetching font file
    fontName = CEV_txtParseTxtFrom(src, "font");
    sprintf(fileName, "%s%s", folder, fontName);
    printf("at line %d fileName is %s\n", __LINE__, fileName);

    CEV_Capsule caps;

    //loading font file into buffer
    if(CEV_capsuleFromFile(&caps, fileName))
    {
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, fileName);
        funcSts = FUNC_ERR;
        goto err_2;
    }

    CEV_capsuleTypeWrite(&caps, dst);


    if(readWriteErr)
        funcSts = FUNC_ERR;

err_2:
    CEV_capsuleClear(&caps);

err_1:

    if(NOT_NULL(dst))
        fclose(dst);

    CEV_textDestroy(src);

end:
    return funcSts;
}


CEV_ScrollText* CEV_scrollLoad(const char* fileName)
{//loads scroll.sdat file

    if(fileName == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    //openning file
    FILE* file = fopen(fileName, "rb");

    if(file == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        goto exit;
    }

    CEV_ScrollText* result = CEV_scrollTypeRead(file);

    if(result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

exit :

    if(file != NULL)
        fclose(file);

    return result;
}


CEV_ScrollText* CEV_scrollLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads scroll from virtual file

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : src arg is NULL.\n", __FUNCTION__, __LINE__ );
        return NULL;
    }

    CEV_ScrollText *result = CEV_scrollTypeRead_RW(src);

    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


CEV_ScrollText* CEV_scrollTypeRead(FILE* file)
{//loads scroll.sdat file

    CEV_ScrollText  *result = NULL,
                    scrollTemp;

    CEV_Text    text    = {.line = NULL};
    CEV_Capsule caps    = {.data = NULL};
    CEV_Font    *font   = NULL;

    if(IS_NULL(file))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    L_scrollTypeHeaderRead(file, &scrollTemp);  //scroll parameters
    CEV_textTypeRead(file, &text);              //associated texts
    CEV_textDump(&text);
    CEV_capsuleTypeRead(file, &caps);        //font capsule


    if(caps.type != IS_FONT)
    {//bad file type
        fprintf(stderr, "Err at %s / %d : embedded file is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    //closes ops
    font = CEV_capsuleExtract(&caps, true);

    if(font == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load font : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
        goto exit;
    }

    result = CEV_scrollCreate(text.line, text.linesNum, font->font, scrollTemp.color);

    CEV_fontClose(font);

    if(result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    result->mode     = scrollTemp.mode;
    result->fontSize = scrollTemp.fontSize;
    result->space    = scrollTemp.space;
    result->speed    = scrollTemp.speed;

exit :

    CEV_textClear(&text);

    return result;
}


CEV_ScrollText* CEV_scrollTypeRead_RW(SDL_RWops* src)
{

    CEV_ScrollText  *result = NULL,
                    scrollTemp;
    CEV_Capsule    caps  = {0};
    TTF_Font        *font   = NULL;
    SDL_RWops       *ops    = NULL;


    if(src == NULL)
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    L_scrollTypeHeaderRead_RW(src, &scrollTemp);    //scroll parameters

    CEV_Text* text = CEV_textLoad_RW(src, 0);   //associated texts

    CEV_textDump(text);

    if(IS_NULL(text))
    {
        fprintf(stderr, "Err at %s / %d : unable to create CEV_Text.\n", __FUNCTION__, __LINE__ );
        goto exit;
    }

    CEV_capsuleTypeRead_RW(src, &caps);              //font capsule


    if(caps.type != IS_FONT)
    {//bad file type
        fprintf(stderr, "Err at %s / %d : embedded file is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    ops = SDL_RWFromMem(caps.data, caps.size);

    if(ops == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto exit;
    }

    //closes ops
    font = TTF_OpenFontRW(ops, 1, scrollTemp.fontSize);

    if(font == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to load font : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
        goto exit;
    }

    result = CEV_scrollCreate(text->line, text->linesNum, font, scrollTemp.color);

    if(result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    result->mode     = scrollTemp.mode;
    result->fontSize = scrollTemp.fontSize;
    result->space    = scrollTemp.space;
    result->speed    = scrollTemp.speed;

exit :

    //if(ops != NULL)
        //SDL_RWclose(ops);

    CEV_textDestroy(text);

    TTF_CloseFont(font);

    CEV_capsuleClear(&caps);

    return result;
}

    /*LOCAL FUNCTIONS IMPLEMENTATION*/


static uint8_t L_scrollDisplay(CEV_ScrollText *in)
{//blit tout les texts valides

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {//pour chaque text présent

        if(!L_scrollDispThisLine(&in->texts[i]))
        {//si erreur de blit
            fprintf(stderr,"Err at %s / %d : impossible de blitter l'image de la ligne %d\n", __FUNCTION__, __LINE__, i+1);
            return(0);
        }
    }

    return 1;
}


static uint8_t L_stwaDisplay(CEV_ScrollText *in)
{//blit tout les texts valides

    SDL_Rect clip = {0, 0, 0, 1},
             blit = {0, 0, 0, 1};

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {//pour chaque text présent

        L_ScrollTextLine *this = &in->texts[i];

        clip.w = this->blitPos.w;
        blit = this->blitPos;
        blit.h = 1;

        for(int j=0; j<this->blitPos.h; j++)
        {
            clip.y = j;
            blit.y = this->blitPos.y+j;
            blit.w = CEV_map(blit.y, 0, in->renderDim.h, in->renderDim.w/10, in->renderDim.w);
            blit.x =(in->renderDim.w - blit.w)/2;

            SDL_RenderCopy(in->render, this->img, &clip, &blit);
        }
    }

    return 0;
}


static int L_scrollUp(CEV_ScrollText *in)
{//procède au déplacement de tous les images de texte

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.y + in->texts[i].blitPos.h) <= (in->renderDim.h - in->space);

        //moving pic
        in->texts[i].blitPos.y -= in->speed;

        if (in->texts[i].blitPos.y < -(in->texts[i].blitPos.h))
        {//if pic off the screen
            //start scan further
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && ((in->texts[i].blitPos.y + in->texts[i].blitPos.h) <= (in->renderDim.h - in->space))
            && !lFront )
        {//if space reached, launch next pic
                in->lineTo += (in->lineTo+1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollDown(CEV_ScrollText *in)
{//procède au déplacement de tous les images de texte

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.y >= in->space);

        //moving pic
        in->texts[i].blitPos.y += in->speed;

        if (in->texts[i].blitPos.y > in->renderDim.h)
        {//if pic off the screen
            //start scan further
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo - 1)
            && (in->texts[i].blitPos.y >= in->space)
            && !lFront)
        {//if space reached, launch next pic
                in->lineTo += (in->lineTo + 1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollLeft(CEV_ScrollText *in)
{//procède au déplacement de tous les images de texte

    /*---EXECUTION---*/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = ((in->texts[i].blitPos.x + in->texts[i].blitPos.w) <= (in->renderDim.w - in->space));

        //moving pic
        in->texts[i].blitPos.x -= in->speed;

        if (in->texts[i].blitPos.x < -(in->texts[i].blitPos.w))
        {//if pic off the screen
            //start scan further
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && ((in->texts[i].blitPos.x + in->texts[i].blitPos.w) <= (in->renderDim.w - in->space))
            && !lFront)
        {//if space reached, launch next pic
                in->lineTo += (in->lineTo + 1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollRight(CEV_ScrollText *in)
{//procède au déplacement de tous les images de texte

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.x >= in->space);
        //moving pic
        in->texts[i].blitPos.x += in->speed;

        if ((in->texts[i].blitPos.x) > in->renderDim.w)
        {//if pic off the screen
            //start scan further
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && (in->texts[i].blitPos.x >= in->space)
            && !lFront)
        {//if space reached, launch next pic
                in->lineTo += in->lineTo+1 <= in->lineNb;
        }
    }

    return 1;
}


static uint8_t L_scrollDispThisLine(L_ScrollTextLine *ligne)
{//blit une image de text

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if (SDL_RenderCopy(render, ligne->img, NULL, &ligne->blitPos)<0)
        return 0;

    return(1);
}


static void L_scrollTypeHeaderRead(FILE* file, CEV_ScrollText *in)
{//reads scroll header in file

    if ((in == NULL) || (file == NULL))
    {
        readWriteErr++;
        return;
    }

    in->id       = read_u32le(file);
    in->fontSize = read_u32le(file);
    in->color.r  = read_u8(file);
    in->color.g  = read_u8(file);
    in->color.b  = read_u8(file);
    in->color.a  = read_u8(file);
    in->mode     = read_u32le(file);
    in->space    = read_u32le(file);
    in->speed    = read_u32le(file);

}


static void L_scrollTypeHeaderRead_RW(SDL_RWops* src, CEV_ScrollText *dst)
{//reads scroll header in virtual file

    if ((dst == NULL) || (src == NULL))
    {
        readWriteErr++;
        return;
    }

    dst->id        = SDL_ReadLE32(src);
    dst->fontSize = SDL_ReadLE32(src);
    dst->color.r  = SDL_ReadU8(src);
    dst->color.g  = SDL_ReadU8(src);
    dst->color.b  = SDL_ReadU8(src);
    dst->color.a  = SDL_ReadU8(src);
    dst->mode     = SDL_ReadLE32(src);
    dst->space    = SDL_ReadLE32(src);
    dst->speed    = SDL_ReadLE32(src);

}


static uint32_t L_scrollModeStringToValue(char* mode)
{//converts string to value for scroll mode

   /** SCROLL_UP = 0, SCROLL_DOWN = 1, SCROLL_LEFT = 2, SCROLL_RIGHT = 3 */

   char* ref[SCROLL_LAST] = SCROLL_MODE_LIST;

    for (int i = 0; i < SCROLL_LAST; i++)
    {
        if (!strcmp(mode, ref[i]))
            return i;
    }

    return SCROLL_UP;//default value
}


static int L_scrollInsertText(FILE *src, FILE* dst)
{//inserts CEV_Text capsule

    int         funcSts = FUNC_OK;
    CEV_Text    *text   = CEV_textTxtLoadf(src);

    if(text != NULL)
        CEV_textTypeWrite(text, dst);
    else
    {
        fprintf(stderr, "Err at %s / %d : failed creating text capsule.\n", __FUNCTION__, __LINE__);
        funcSts = FUNC_ERR;
        goto exit;
    }

    CEV_textDestroy(text);

exit:

    if(readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}



