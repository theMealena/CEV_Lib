#include <stdio.h>
#include <errno.h>
#include <wchar.h>
#include "CEV_logiLCD.h"
#include <LogitechLCDLib.h>
#include "CEV_dataFile.h"
#include "CEV_mixSystem.h"


/*---LOCAL FUNCTIONS DECLARATION---*/

//sortcut to main structure
REV_SystemLcd *L_systemLcdSet(REV_SystemLcd* sys);

//turns surface into LCD display pixel format
SDL_Surface *L_lcdPicformat(SDL_Surface *src);


/*---FUNCTIONS IMLEMENTATION---*/

bool REV_systemLcdInit(REV_SystemLcd *sys)
{//init logitech lcd lib

    //setting vars to default
    sys->result = sys->srcBckgd = NULL;

    //init lib
    sys->isInit = LogiLcdInit(L"RevolveR", LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR);

    if (sys->isInit)
    {//init ok

        //trying to find any compatible keyboard
        if(LogiLcdIsConnected(LOGI_LCD_TYPE_COLOR))//color
        {
            sys->lcdType = LOGI_LCD_TYPE_COLOR;
            sys->result = SDL_CreateRGBSurface(0, LOGI_LCD_COLOR_WIDTH, LOGI_LCD_COLOR_HEIGHT, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
            SDL_SetSurfaceBlendMode(sys->result, SDL_BLENDMODE_BLEND);

        }

        else if (LogiLcdIsConnected(LOGI_LCD_TYPE_MONO))//BW
        {
            sys->lcdType = LOGI_LCD_TYPE_MONO;
            sys->result = SDL_CreateRGBSurface(0, LOGI_LCD_MONO_WIDTH, LOGI_LCD_MONO_HEIGHT, 8, 0x0, 0x0, 0x0, 0x0);
        }

        else
        {//close lib if none found
            LogiLcdShutdown();
            sys->isInit = false;
        }
    }

    if(sys->result == NULL)
    {
        fprintf(stderr, "Err / REV_systemLcdInit : unable to create resulting surface : %s\n", SDL_GetError());
    }

    L_systemLcdSet(sys);

    return sys->isInit;
}


void REV_systemLcdClose(void)
{//closees logitech LCD lib

    REV_SystemLcd* sys = REV_systemLcdGet();

    if(sys != NULL && sys->isInit)
        LogiLcdShutdown();

    if(sys->result != NULL)
    {
        SDL_FreeSurface(sys->result);
        sys->result = NULL;
    }

    if(sys->srcBckgd != NULL)
    {
        SDL_FreeSurface(sys->srcBckgd);
        sys->srcBckgd = NULL;
    }
}


REV_SystemLcd* REV_systemLcdGet(void)
{//fetches lcd system

    return L_systemLcdSet(NULL);
}


void REV_lcdTitleSet(const char *src)
{//sts lcd title

    size_t srcSize = strlen(src);

    wchar_t text[200] = {0};

    REV_SystemLcd* sys = REV_systemLcdGet();

    mbsrtowcs(text, &src, srcSize+1, NULL);

    switch(sys->lcdType)
    {
        case LOGI_LCD_TYPE_COLOR:
            LogiLcdColorSetTitle(text, 255, 255, 255);
        break;

        case LOGI_LCD_TYPE_MONO:
            LogiLcdMonoSetText(0, text);
        break;
    }

    //LogiLcdUpdate();
}


void REV_lcdBackgroundSet(int index)
{//sets lcd background

    /*---DECLARATIONS---*/

    REV_SystemLcd* sys = REV_systemLcdGet();

    /*---PRL---*/

    if((!sys->isInit) || (sys->result == NULL))
        return;

    /*---EXECUTION---*/

    if(sys->srcBckgd != NULL)
    {
        SDL_FreeSurface(sys->srcBckgd);
        sys->srcBckgd = NULL;
    }

    switch (sys->lcdType)
    {
        case LOGI_LCD_TYPE_COLOR:
// TODO (drx#2#): résoudre problème du nom du fichier ici versus resource holder
            sys->srcBckgd = CEV_anyFetchByIndexFromFile(index, DATA_FILE);

        break;

        case LOGI_LCD_TYPE_MONO:

            sys->srcBckgd = CEV_anyFetchByIndexFromFile(index+1, DATA_FILE);

        break;
    }

    if(sys->srcBckgd == NULL)
    {
        fprintf(stderr, "Err / REV_lcdBackgroundSet : unable to load surface :%s\n", SDL_GetError());
    }
    else
    {
        SDL_BlitSurface(sys->srcBckgd, NULL, sys->result, NULL);
    }

    /*---POST---*/

    //LogiLcdUpdate();

    //SDL_FreeSurface(lSurface);
}


void REV_lcdShow(void)
{
    /*---DECLARATIONS---*/

    REV_SystemLcd* sys = REV_systemLcdGet();
    SDL_Surface *lSurface = NULL;

    /*---PRL---*/
    if(!sys->isInit)
        return;

    /*---EXECUTION---*/

    switch (sys->lcdType)
    {
        case LOGI_LCD_TYPE_COLOR:

            lSurface = L_lcdPicformat(sys->result);

            if(lSurface == NULL)
            {
                fprintf(stderr, "Err / REV_lcdShow : unable to convert surface :%s\n", SDL_GetError());
            }
            else
            {
                LogiLcdColorSetBackground((BYTE*)lSurface->pixels);
                SDL_FreeSurface(lSurface);
            }

        break;

        case LOGI_LCD_TYPE_MONO:

            LogiLcdMonoSetBackground((BYTE*)sys->result->pixels);

        break;
    }

    /*---POST---*/

    LogiLcdUpdate();
}


void REV_lcdLinesClear()
{
    REV_SystemLcd *sys = REV_systemLcdGet();

    if(sys->lcdType == LOGI_LCD_TYPE_COLOR)
    {
        REV_lcdTitleSet("");

        for (int i = 0; i<8; i++)
            LogiLcdColorSetText(i, L"", 0, 0, 0 );
    }
    else
    {
        for (int i = 0; i<4; i++)
            LogiLcdMonoSetText(i, L"");
    }
}


/*---LOCALS FUNCTIONS IMLEMENTATION---*/


REV_SystemLcd *L_systemLcdSet(REV_SystemLcd* sys)
{//memorize shortcut to lcd system

    static REV_SystemLcd* system = NULL;

    if(sys != NULL)
        system = sys;

    return system;
}


SDL_Surface *L_lcdPicformat(SDL_Surface *src)
{//format surface to display on lcd screen

    SDL_Surface *result = NULL;

    result = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_ARGB8888, 0);

    if(result == NULL)
    {
        fprintf(stderr, "Err / L_lcdPicformat : unable to convert surface.\n");
    }

    return result;
}
