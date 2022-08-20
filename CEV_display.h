//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  20-05-2016   |   0.1    |    SDL2 rev    **/
//**********************************************************/
#ifndef EZ_DISPLAY_H_INCLUDED
#define EZ_DISPLAY_H_INCLUDED

#include <SDL.h>
#include <SDL_ttf.h>
#include "CEV_api.h"
#include "CEV_dataFile.h"



#define ZOOM_SCALE 1.05


#define CEV_LEFT        0x1
#define CEV_RIGHT       0x2
#define CEV_X_CENTERED  0x4
#define CEV_TOP         0x8
#define CEV_BOTTOM      0x10
#define CEV_Y_CENTERED  0x20
#define NUM_MAX_DIGIT   20

#ifdef __cplusplus
extern "C" {
#endif



/*affichage des textes du menu**/
//int dispMenu(CEV_Menu *menu, SDL_Rect* mseBox);
/*
- affiche avec la police envoyée selon les paramètres de menu du haut vers le bas.
- menu.numLigneSurbrille défini la ligne en surbrillance, -1 pour passer toutes les lignes en surbrillance.
- menu.numLine doit contenir le nombre d'élément des tableaux
text et mseBox.
- renseigne la mseBox en fonction de la taille des images
**/



/**Direct value display*/
 /** \brief displays value from TTF font.
 *
 * \param val : value to be displayed.
 * \param font : TTF_Font* to be applied.
 * \param colour : SDL_Color to be applied.
 * \param point : position to diplay value.
 * \param mode : display align mode.
 * \param ratio : ratio of FONT_MAX.
 *
 * \return one of function status
 */
int CEV_dispValue(const int val, TTF_Font *font, SDL_Color colour, SDL_Point point, int mode, float ratio);


/**Direct text display*/
/** \brief displays text from TTF font.
 *
 * \param text : text to be displayed.
 * \param font : TTF_Font* to be applied.
 * \param colour : SDL_Color to be applied.
 * \param x : horizontal position.
 * \param y : vertical position.
 * \param mode : display align mode.
 * \param ratio : ratio of FONT_MAX.
 *
 * \return one of function status
 */
 int CEV_dispText(const char *text,TTF_Font *font, SDL_Color colour, SDL_Point point, int mode, float ratio);


/** \brief display simple warning box
*
* \param title : box title
* \param message : message to display
*
* \return N/A
*/
void CEV_dispWarning(char *title, char *message);


/** \brief display simple yes/no box.
*
* \param title : box title.
* \param message : message to display.
*
* \return 1 as "yes" (return key), 0 otherwise (escape key).
*/
int CEV_dispConfirm(char *title, char* message);


/** \brief display simple choice box.
*
* \param title : box title.
* \param message : message to display.
* \param opt0 : first choice text (escape key).
* \param opt1 : second choice text (return key).
*
* \return 1 as "opt1", <=0 otherwise
*/
int CEV_dispChoice2(char *title, char* message, char* opt0, char* opt1);


/** \brief display simple choice box
*
* \param title : box title
* \param message : message to display
* \param opt0 : first choice text (escape key).
* \param opt1 : second choice text.
* \param opt2 : third choice text (return key).
*
* \return int : as selected option
*/
int CEV_dispChoice3(char *title, char* message, char* opt0, char* opt1, char* opt2);

/**Texture from font**/
/** \brief creates SDL_Texture*.
 *
 * \param text : text to be displayed on texture.
 * \param font : the font text is base upon.
 * \param colour : color to be displayed.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_createTTFTexture(const char *text, TTF_Font *font, SDL_Color colour);


/** \brief SDL_color version of SDL_SetRenderDrawColor().
 *
 * \param render SDL_Renderer* to apply color to.
 * \param color SDL_Color to be applied.
 *
 * \return N/A.
 */
void CEV_renderColorSet(SDL_Renderer* render, SDL_Color color);


/** \brief SDL_color version of SDL_GetRenderDrawColor().
 *
 * \param render SDL_Renderer* to fetch color from.
 * \param color SDL_Color* to store color into.
 *
 * \return N/A
 */void CEV_renderColorGet(SDL_Renderer* render, SDL_Color* color);


/**adjusts display position*/
void CEV_dispBlitPos(SDL_Rect* pos, SDL_Point point, int mode, float ratio);

//CEV_Zoom CEV_zoomInit(int baseW, int baseH, float scaleMax, float step);

//SDL_Rect CEV_zoomOnCoord(CEV_Zoom *zoom, SDL_Point point);

//void CEV_zoomScaleUpdate(CEV_Zoom *zoom);

//SDL_Surface* switchFullScreen();
/**fullscreen ou pas...**/


#ifdef __cplusplus
}
#endif

#endif /* EZ_DISPLAY_H_INCLUDED */
