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

    int sts = 1;//on activity && no error = 1, else 0

    switch (in->mode)
    {
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

    if(!L_scrollDisplay(in))
        sts = 0; //stops scrolling on error

exit:

    return sts;
}



    /*FILES FUNCTIONS*/


int CEV_scrollConvertTxtToData(const char* srcName, const char* dstName)
{//convert natural .txt file into scroll.srl file


    int funcSts     = FUNC_OK;

    FILE *src       = NULL,
         *dst       = NULL;

    CEV_Capsule lBuffer;

    uint32_t fontSize   = 0,
             speed      = 0,
             space      = 0;

    uint8_t  colors[4];

    int temp[4] ={0, 0, 0, 0};

    char lString[FILENAME_MAX],
         folder[FILENAME_MAX],
         fileName[FILENAME_MAX],
         hasFolder = CEV_fileFolderNameGet(srcName, folder);

    if((srcName == NULL) || (dstName == NULL))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        funcSts = ARG_ERR;
        goto exit;
    }

    readWriteErr = 0;

    src = fopen(srcName, "r");
    dst = fopen(dstName, "wb");

    if ((src == NULL) || (dst == NULL))
    {
        fprintf(stderr, "Err at %s / %d : cannot open file.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    rewind(src);

    fgets(lString, sizeof(lString)-1, src);//gets font filename
    CEV_stringEndFormat(lString);

    if(hasFolder)
    {
        strcpy(fileName, folder);
        strcat(fileName, lString);
    }
    else
        strcpy(fileName, lString);

    CEV_capsuleFromFile(&lBuffer, fileName);//loads font file into buffer

    if(lBuffer.type != IS_FONT)
    {
        fprintf(stderr,"Warn at %s / %d : file is not ttf extension.\n", __FUNCTION__, __LINE__);
        funcSts = FUNC_ERR;
    }

    if(lBuffer.data == NULL)
    {
        fprintf(stderr,"Err at %s / %d : failed loading %s.\n", __FUNCTION__, __LINE__, fileName);
        funcSts = FUNC_ERR;
        goto err_1;
    }

    //reads font size
    fscanf(src, "%u", &fontSize);

    //reads RGBA
    fscanf(src, "%d %d %d %d\n", &temp[0], &temp[1], &temp[2], &temp[3]);

    for(int i = 0; i<4; i++)
        colors[i] = temp[i];

    //reads mode
    fgets(lString, sizeof(lString)-1, src);
    CEV_stringEndFormat(lString);

    //reads speed & space
    fscanf(src, "%u %u\n", &space, &speed);

    //starts writing data fileFormat

    write_u32le(fontSize, dst); //font size

    for(int i=0; i< sizeof(colors); i++) //rgba
        write_u8(colors[i], dst);

    write_u32le(L_scrollModeStringToValue(lString), dst);//read mode

    write_u32le(space, dst);//scroll space
    write_u32le(speed, dst);//scroll speed


    //inserts text capsule
    funcSts = L_scrollInsertText(src, dst);
    if(funcSts != FUNC_OK)
    {
        fprintf(stderr,"Err at %s / %d : unable to insert text.\n", __FUNCTION__, __LINE__);
        goto err_2;
    }

    //inserts raw font file
    CEV_capsuleTypeWrite(&lBuffer, dst);

    if(readWriteErr)
        funcSts = FUNC_ERR;

err_2:
    CEV_capsuleClear(&lBuffer);

err_1:
    if(src != NULL)
        fclose(src);

    if(dst != NULL)
        fclose(dst);

exit :
    if(readWriteErr)
        funcSts = FUNC_ERR;

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

    CEV_Text        text    = {.line = NULL};
    CEV_Capsule    lBuffer  = {.data = NULL};
    TTF_Font        *font   = NULL;
    SDL_RWops       *ops    = NULL;


    if(IS_NULL(file))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    L_scrollTypeHeaderRead(file, &scrollTemp);  //scroll parameters
    CEV_textTypeRead(file, &text);              //associated texts
    CEV_capsuleTypeRead(file, &lBuffer);            //font capsule


    if(lBuffer.type != IS_FONT)
    {//bad file type
        fprintf(stderr, "Err at %s / %d : embedded file is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    ops = SDL_RWFromMem(lBuffer.data, lBuffer.size);

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

    result = CEV_scrollCreate(text.line, text.linesNum, font, scrollTemp.color);

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

    CEV_textClear(&text);

    TTF_CloseFont(font);

    CEV_capsuleClear(&lBuffer);

    return result;
}


CEV_ScrollText* CEV_scrollTypeRead_RW(SDL_RWops* src)
{

    CEV_ScrollText  *result = NULL,
                    scrollTemp;
    //CEV_Text        text    = {0};
    CEV_Capsule    lBuffer  = {0};
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

    CEV_capsuleTypeRead_RW(src, &lBuffer);              //font capsule


    if(lBuffer.type != IS_FONT)
    {//bad file type
        fprintf(stderr, "Err at %s / %d : embedded file is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    ops = SDL_RWFromMem(lBuffer.data, lBuffer.size);

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

    CEV_capsuleClear(&lBuffer);

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
{/*convert string to value for scroll mode*/

   /** SCROLL_UP = 0, SCROLL_DOWN = 1, SCROLL_LEFT = 2, SCROLL_RIGHT = 3 */

   char* ref[SCROLL_MODE_NUM] = SCROLL_MODE_LIST;

    for (int i = 0; i < SCROLL_MODE_NUM; i++)
    {
        if (!strcmp(mode, ref[i]))
            return i;
    }

    return SCROLL_UP;//default value
}


static int L_scrollInsertText(FILE *src, FILE* dst)
{/*insert CEV_Text capsule*/

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



