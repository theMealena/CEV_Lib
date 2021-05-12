//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  20-05-2016   |   1.0    |    SDL2 rev    **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**********************************************************/


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

/*manage scrolling up*/
static int L_scrollUp(CEV_ScrollText *in);

/*manage scrolling down*/
static int L_scrollDown(CEV_ScrollText *in);

/*manage scrolling left*/
static int L_scrollLeft(CEV_ScrollText *in);

/*manage scrolling right*/
static int L_scrollRight(CEV_ScrollText *in);

/*display all lines*/
static uint8_t L_scrollDisplay(CEV_ScrollText *in);

/*display one line*/
static uint8_t L_scrollDispThisLine(L_ScrollTextLine *ligne);

/*convert string to value for scroll mode*/
static uint32_t L_scrollModeStringToValue(char* mode);

/*inserts text capsule into data file*/
static int L_scrollInsertText(FILE *src, FILE* dst);

/*reads scroll header in sdat file.*/
static void L_scrollTypeRead(FILE* file, CEV_ScrollText *in);


/*USER END FUNCTIONS*/

CEV_ScrollText* CEV_scrollCreate(char** texts, unsigned int num, TTF_Font* font, SDL_Color colour)
{/*create a text scroll structure*/

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



void CEV_scrollFree(CEV_ScrollText *in)
{/*lib�ration d'une structure CEV_ScrollText*/

    for (int i=0; i<in->lineNb; i++)
        SDL_DestroyTexture(in->texts[i].img);

    free(in->texts);

    free(in);
}


void CEV_scrollRaz(CEV_ScrollText *in)
{/*lib�ration du contenu + mise � 0/NULL*/

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


    /*CONTROL FUNCTIONS*/

void CEV_scrollModeSet(CEV_ScrollText *in, int mode)
{
    in->mode = mode;
}


void CEV_scrollSpaceSet(CEV_ScrollText *in, unsigned int pxl)
{
    in->space = pxl;
}


void CEV_scrollRestart(CEV_ScrollText* in)
{
    in->lineFrom = 0;
    in->lineTo  = 1;
}


void CEV_scrollPosSet(CEV_ScrollText* in, int pos)
{
    in->pos = pos;

    for(int i=0; i<in->lineNb; i++)
    {
        int wt, ht, wRatio, hRatio;

        SDL_QueryTexture(in->texts[i].img, NULL, NULL, &wt, &ht);

        wRatio = in->fontSize*wt/FONT_MAX;
        hRatio = in->fontSize*ht/FONT_MAX;

        switch (in->mode)
        {/*sets up start positions*/

            case SCROLL_UP:
                in->texts[i].blitPos = (SDL_Rect){.x = pos - wRatio/2,
                                                  .y = in->renderDim.h,
                                                  .w = wRatio,
                                                  .h = hRatio};
            break;

            case SCROLL_DOWN:
                in->texts[i].blitPos = (SDL_Rect){.x = pos - wRatio/2,
                                                  .y = -hRatio/2,
                                                  .w = wRatio,
                                                  .h = hRatio};
            break;

            case SCROLL_LEFT:
                in->texts[i].blitPos = (SDL_Rect){.x = in->renderDim.w,
                                                  .y = pos - hRatio/2,
                                                  .w = wRatio,
                                                  .h = hRatio};
            break;

            case SCROLL_RIGHT:
                in->texts[i].blitPos = (SDL_Rect){.x = -wRatio,
                                                  .y = pos - hRatio/2,
                                                  .w = wRatio,
                                                  .h = hRatio};
            break;
        }
    }
}


int CEV_scrollUpdate(CEV_ScrollText *in)
{
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

    L_scrollDisplay(in);

exit:

    return sts;
}



    /*FILES FUNCTIONS*/


int CEV_convertScrollTxtToData(const char* srcName, const char* dstName)
{/*convert natural .txt file into scroll.srl file*/

    /*DEC*/

    int funcSts     = FUNC_OK;
    FILE *src       = NULL,
         *dst       = NULL;

    CEV_FileInfo lBuffer;

    uint32_t fontSize   = 0,
             speed      = 0,
             space      = 0;

    uint8_t  colors[4];

    int temp[4] ={0, 0, 0, 0};

    char lString[FILENAME_MAX],
         folder[FILENAME_MAX],
         fileName[FILENAME_MAX],
         hasFolder = CEV_fileFolderNameGet(srcName, folder);

    /*PRL*/

    if((srcName == NULL) || (dstName == NULL))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        funcSts = ARG_ERR;
        goto exit;
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

    fgets(lString, sizeof(lString)-1, src);/*gets font filename*/
    CEV_stringEndFormat(lString);

    if(hasFolder)
    {
        strcpy(fileName, folder);
        strcat(fileName, lString);
    }
    else
        strcpy(fileName, lString);

    CEV_rawLoad(&lBuffer, fileName);/*loads font file into buffer*/

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

    /*reads font size*/
    fscanf(src, "%u", &fontSize);

    /*reads RGBA*/
    fscanf(src, "%d %d %d %d\n", &temp[0], &temp[1], &temp[2], &temp[3]);

    for(int i = 0; i<4; i++)
        colors[i] = temp[i];

    /*reads mode*/
    fgets(lString, sizeof(lString)-1, src);
    CEV_stringEndFormat(lString);

    /*reads speed & space*/
    fscanf(src, "%u %u\n", &space, &speed);

    /*starts writing data fileFormat**/

    write_u32le(fontSize, dst); /*font size*/

    for(int i=0; i< sizeof(colors); i++) /*rgba*/
        write_u8(colors[i], dst);

    write_u32le(L_scrollModeStringToValue(lString), dst);/*read mode*/

    write_u32le(space, dst);/*scroll space*/
    write_u32le(speed, dst);/*scroll speed*/


    /*inserts text capsule*/
    funcSts = L_scrollInsertText(src, dst);
    if(funcSts != FUNC_OK)
    {
        fprintf(stderr,"Err at %s / %d : unable to insert text.\n", __FUNCTION__, __LINE__);
        goto err_2;
    }

    /*inserts raw font file*/
    CEV_fileInfoTypeWrite(&lBuffer, dst);

    /*POST**/

    if(readWriteErr)
        funcSts = FUNC_ERR;

err_2:
    CEV_fileInfoClear(&lBuffer);

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
{/*loads scroll.sdat file*/

    /*---DECLARATIONS---*/

    CEV_ScrollText  *result = NULL;
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

    result = CEV_scrollLoadf(file);

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


CEV_ScrollText* CEV_scrollLoadf(FILE* file)
{/*loads scroll.sdat file*/

    /**DECLARATIONS***/

    CEV_ScrollText  *result = NULL,
                    scrollTemp;
    CEV_Text        text    = {.line = NULL};
    CEV_FileInfo    lBuffer = {.data = NULL};
    TTF_Font        *font   = NULL;
    SDL_RWops       *ops    = NULL;

    /**PRL**/


    if(file == NULL)
    {/*bad arg*/
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    L_scrollTypeRead(file, &scrollTemp);/*scroll parameters*/

    CEV_textTypeRead(file, &text);   /*associated texts*/

    CEV_fileInfoTypeRead(file, &lBuffer);/*font with header*/


    if(lBuffer.type != IS_FONT)
    {/*bad file type*/
        fprintf(stderr, "Err at %s / %d : embedded file is not scroll.\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    ops = SDL_RWFromMem(lBuffer.data, lBuffer.size);

    if(ops == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to create virtual file : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto exit;
    }

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

    if(text.line != NULL)
        CEV_textClear(&text);

    if(font != NULL)
        TTF_CloseFont(font);

    if(lBuffer.data != NULL)
        CEV_fileInfoClear(&lBuffer);

    return result;
}



    /*LOCAL FUNCTIONS IMPLEMENTATION*/


static uint8_t L_scrollDisplay(CEV_ScrollText *in)
{/*blit tout les texts valides*/

    /****EXECUTION****/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {/*pour chaque text pr�sent*/

        if(!L_scrollDispThisLine(&in->texts[i]))
        {/*si erreur de blit*/
            fprintf(stderr,"Err at %s / %d : impossible de blitter l'image de la ligne %d\n", __FUNCTION__, __LINE__, i+1);
            return(0);
        }
    }

    return 1;
}


static int L_scrollUp(CEV_ScrollText *in)
{/*proc�de au d�placement de tous les images de texte*/

    /****EXECUTION****/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.y + in->texts[i].blitPos.h) <= (in->renderDim.h - in->space);

        /*moving pic*/
        in->texts[i].blitPos.y -= in->speed;

        if (in->texts[i].blitPos.y < -(in->texts[i].blitPos.h))
        {/*if pic off the screen*/
            /*start scan further*/
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && ((in->texts[i].blitPos.y + in->texts[i].blitPos.h) <= (in->renderDim.h - in->space))
            && !lFront )
        {/*if space reached, launch next pic*/
                in->lineTo += (in->lineTo+1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollDown(CEV_ScrollText *in)
{/*proc�de au d�placement de tous les images de texte*/

    /*---EXECUTION---*/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.y >= in->space);

        /*moving pic*/
        in->texts[i].blitPos.y += in->speed;

        if (in->texts[i].blitPos.y > in->renderDim.h)
        {/*if pic off the screen*/
            /*start scan further*/
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo - 1)
            && (in->texts[i].blitPos.y >= in->space)
            && !lFront)
        {/*if space reached, launch next pic*/
                in->lineTo += (in->lineTo + 1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollLeft(CEV_ScrollText *in)
{/*proc�de au d�placement de tous les images de texte*/

    /*---EXECUTION---*/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = ((in->texts[i].blitPos.x + in->texts[i].blitPos.w) <= (in->renderDim.w - in->space));

        /*moving pic*/
        in->texts[i].blitPos.x -= in->speed;

        if (in->texts[i].blitPos.x < -(in->texts[i].blitPos.w))
        {/*if pic off the screen*/
            /*start scan further*/
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && ((in->texts[i].blitPos.x + in->texts[i].blitPos.w) <= (in->renderDim.w - in->space))
            && !lFront)
        {/*if space reached, launch next pic*/
                in->lineTo += (in->lineTo + 1 <= in->lineNb);
        }
    }

    return 1;
}


static int L_scrollRight(CEV_ScrollText *in)
{/*proc�de au d�placement de tous les images de texte*/

    /*---EXECUTION---*/

    for(int i = in->lineFrom; i < in->lineTo; i++)
    {
        char lFront = (in->texts[i].blitPos.x >= in->space);
        /*moving pic*/
        in->texts[i].blitPos.x += in->speed;

        if ((in->texts[i].blitPos.x) > in->renderDim.w)
        {/*if pic off the screen*/
            /*start scan further*/
            in->lineFrom += (in->lineFrom + 1 <= in->lineNb);

            if(in->lineFrom == in->lineTo)
                return 0;
        }

        if ((i == in->lineTo-1)
            && (in->texts[i].blitPos.x >= in->space)
            && !lFront)
        {/*if space reached, launch next pic*/
                in->lineTo += in->lineTo+1 <= in->lineNb;
        }
    }

    return 1;
}


static uint8_t L_scrollDispThisLine(L_ScrollTextLine *ligne)
{/*blit une image de text*/

    SDL_Renderer *render = CEV_videoSystemGet()->render;

    if (SDL_RenderCopy(render, ligne->img, NULL, &ligne->blitPos)<0)
        return 0;

    return(1);
}


static void L_scrollTypeRead(FILE* file, CEV_ScrollText *in)
{
    /*PRL*/

    if ((in == NULL) || (file == NULL))
    {
        readWriteErr++;
        return;
    }

    /*EXECUTION*/

    in->fontSize = read_u32le(file);
    in->color.r  = read_u8(file);
    in->color.g  = read_u8(file);
    in->color.b  = read_u8(file);
    in->color.a  = read_u8(file);
    in->mode     = read_u32le(file);
    in->space    = read_u32le(file);
    in->speed    = read_u32le(file);

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
    CEV_Text    *text   = CEV_textLoadf(src);

    if(text != NULL)
        CEV_textTypeWrite(text, dst);
    else
    {
        fprintf(stderr, "Err at %s / %d : failed creating text capsule.\n", __FUNCTION__, __LINE__);
        funcSts = FUNC_ERR;
        goto exit;
    }

    CEV_textFree(text);

exit:

    if(readWriteErr)
        funcSts = FUNC_ERR;

    return funcSts;
}



