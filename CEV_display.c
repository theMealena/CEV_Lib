//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  20-05-2016   |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**********************************************************/

// TODO (drxvd#1#02/15/17): virer les zooms et menus de là


#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "CEV_mixSystem.h"

#ifndef CEV_MIXSYSTEM_H_INCLUDED
 #include <SDL_mixer.h>
#elif CEV_FMODSYSTEM_H_INCLUDED
 #include <FMOD.h>
#endif // CEV_MIXSYSTEM_H_INCLUDED

#include "CEV_display.h"
#include "CEV_dataFile.h"
#include "project_def.h"


    /*LOCAL FUNCTIONS DECLARATIONS*/



/*DIRECT DISPLAY***/



int CEV_dispValue(const int val, TTF_Font *font, SDL_Color colour, SDL_Point point, int mode, float ratio)
{

        /*DECLARATIONS*****/

    char text[NUM_MAX_DIGIT]    = {0};   /*le texte*/
    int funcSts                 = FUNC_OK;

    if(!font)
    {
        fprintf(stderr, "Err at %s / %d : font arg is NULL.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    /*on formate le texte à afficher*/
    sprintf(text, "%d", val);

    /*on crée la surface texte*/
    funcSts = CEV_dispText(text, font, colour, point, mode, ratio);

    return(funcSts);
}


int CEV_dispText(const char *text, TTF_Font *font, SDL_Color colour, SDL_Point point, int mode, float ratio)
{/*affiche un texte***VALIDE***/

        /*DEC*/

    int             funcSts     = FUNC_OK;
    SDL_Surface     *surf_text  = NULL;  /*la surface*/
    SDL_Texture     *texture    = NULL;  /*la texture*/
    SDL_Renderer    *dst        = NULL;  /*surface d'affichage*/
    SDL_Rect        pos;                 /*position de blit*/
        /*PRL*/

    if(text == NULL || font == NULL)
    {
        fprintf(stderr, "Err at %s / %d : Arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    dst = CEV_videoSystemGet()->render;

        /*EXE*/

    /*on crée la surface texte*/
    surf_text = TTF_RenderText_Solid(font, text, colour);

    if (surf_text == NULL)
    {/*gestion d'erreur*/
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
        funcSts = FUNC_ERR;
        goto end;
    }

    texture = SDL_CreateTextureFromSurface(dst, surf_text);

    if (texture == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        funcSts = FUNC_ERR;
        goto err_1;
    }

    /*on fignole le cadre*/
    SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);

    CEV_dispBlitPos(&pos, point, mode, ratio);
    /*on blit*/
    SDL_RenderCopy(dst, texture, NULL, &pos);

    /*---POST---*/

    /*on libère*/
    SDL_DestroyTexture(texture);

err_1 :
    SDL_FreeSurface(surf_text);

end:
    return(funcSts);
}


void CEV_dispWarning(char *title, char* message)
{
    int buttonId;

    const SDL_MessageBoxButtonData button = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT |SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT , 0, "OK"};

    const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_WARNING, /* .flags */
            NULL, /* .window */
            title, /* .title */
            message, /* .message */
            1, /* .numbuttons */
            &button, /* .buttons */
            NULL};

    SDL_ShowMessageBox(&messageboxdata, &buttonId);
}


int CEV_dispConfirm(char *title, char* message)
{
    int buttonId;

    const SDL_MessageBoxButtonData yesNoButtons[2] =
    {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" }
    };

    const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION, /* .flags */
            NULL, /* .window */
            title, /* .title */
            message, /* .message */
            2, /* .numbuttons */
            yesNoButtons, /* .buttons */
            NULL};

    SDL_ShowMessageBox(&messageboxdata, &buttonId);

    return buttonId;
}


SDL_Texture* CEV_createTTFTexture(const char *text, TTF_Font *font, SDL_Color colour)
{/*TTF Texture**/

        /*DEC*/

    SDL_Surface *surf_text  = NULL;  /*la surface*/
    SDL_Texture *texture    = NULL;  /*la texture*/
    SDL_Renderer *dst       = CEV_videoSystemGet()->render;

        /*****PRELIMINAIRE*****/

    /*on crée la surface texte*/
    surf_text = TTF_RenderText_Solid(font, text, colour);

    if (surf_text == NULL)
    {/*gestion d'erreur*/
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, TTF_GetError());
        goto end;
    }

    texture = SDL_CreateTextureFromSurface(dst, surf_text);

    if (texture == NULL)
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, SDL_GetError());

    SDL_FreeSurface(surf_text);

end:
    return(texture);
}


void CEV_dispBlitPos(SDL_Rect* pos, SDL_Point point, int mode, float ratio)
{
// TODO (drx#1#): mettre une valeur par défaut, j'ai oublié CEV_TOP en argument et ça n'affiche rien
    CEV_rectDimScale(pos, ratio);

    if (mode & CEV_LEFT)
        pos->x = point.x;
    else if (mode & CEV_X_CENTERED)
        pos->x = point.x - (pos->w /2);
    else if (mode & CEV_RIGHT)
        pos->x = point.x - pos->w;


    if (mode & CEV_TOP)
        pos->y = point.y;
    else if (mode & CEV_Y_CENTERED)
        pos->y = point.y - (pos->h/2);
    else if (mode & CEV_BOTTOM)
        pos->y = point.y + pos->h;

}


/*--ZOOM RELATED--*/


CEV_Zoom CEV_zoomInit(int baseW, int baseH, float scaleMax, float step)
{
    CEV_Zoom result;

    result.enable    = 0;
    result.scaleMax  = scaleMax;
    result.scaleAct  = 1.0;
    result.pos.x     = 0;
    result.pos.y     = 0;
    result.pos.w     = baseW;
    result.pos.h     = baseH;
    result.step      = step;

    return result;
}


SDL_Rect CEV_zoomOnCoord(CEV_Zoom *zoom, SDL_Point point)
{/*retourne un SDL_Rect de découpe pour que le zoom semble centré sur les coordonnées envoyées*/

    SDL_Rect funcSts;

    CEV_zoomUpdate(zoom);

    funcSts.w    = zoom->pos.w / zoom->scaleAct;
    funcSts.h    = zoom->pos.h / zoom->scaleAct;
    funcSts.x    = zoom->scaleAct * point.x - point.x;
    funcSts.y    = zoom->scaleAct * point.y - point.y;

    return funcSts;
}


void CEV_zoomUpdate(CEV_Zoom *zoom)
{/**update zoom structure**/

    switch (zoom->enable)
    {
        case 1 :
            if(zoom->scaleAct < zoom->scaleMax)
                zoom->scaleAct += zoom->step;
        break;

        case 0 :
            if(zoom->scaleAct > 1)
                zoom->scaleAct -= zoom->step;
        break;
    }

    CEV_fconstraint(0.0, &zoom->scaleAct, zoom->scaleMax);
}





