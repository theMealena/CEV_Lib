//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2017    |   0.1    |    SDL2 rev    **/
//**********************************************************/

#ifndef INPUTS_H_INCLUDED
#define INPUTS_H_INCLUDED

#include <time.h>
#include <stdbool.h>
#include <SDL.h>
#include "CEV_pad.h"
#include "CEV_api.h"


#define MSE_NB_BT 20  /*nombre de boutons de souris*/
#define KEYBOARD 0
#define MOUSE    1
#define PAD      2


#if SDL_MAJOR_VERSION == 2

 #define SDL_BUTTON_WHEELUP	    6
 #define SDL_BUTTON_WHEELDOWN	7
 #define SDL_BUTTON_WHEELRIGHT   8
 #define SDL_BUTTON_WHEELLEFT    9

#endif // SDL_BUTTON_WHEELDOWN

#ifdef __cplusplus
extern "C" {
#endif

/**cvdb
fonctions de gestions des entrées
*/



typedef struct CEV_Window
{/**structure fenetre**/

    bool quitApp,   /*quit apps request*/
         resize,    /*resize request*/
         isFocused; /*window is focused*/
}
CEV_Window;


typedef struct CEV_Mouse
{/**structure souris**/

    bool button[MSE_NB_BT];/*boutons de la souris*/
    SDL_Point pos;/*position x,y actuelle de la souris*/
    SDL_Point rel;/*relatif x.y*/
}
CEV_Mouse;


typedef struct CEV_Input
{/**structure des entrées**/

    bool            key[SDL_NUM_SCANCODES];/*touches clavier*/
    unsigned char   lastDevice,
                    padNum;/*nombre de pad*/
    CEV_Pad         *pad; /*les pads*/
    CEV_Mouse       mouse;/*la souris*/
    CEV_Window      window;/*la fenêtre*/
    unsigned int    lastKeyVal;
    char*           text;
}
CEV_Input;



/**input update**/
/** \brief updates inputs.
 *
 * \return true if any button pressed.
 */
bool CEV_inputUpdate();


/**input structure initialisation**/
/** \brief input initialisation.
 *
 * \return FUNC_OK if init, FATAL on error.
 *
 */
int CEV_inputInit();


/**memory clean-up**/
/** \brief free input structure.
 *
 * \return N/A.
 */
void CEV_inputFree();


/**clear structure content**/
/** \brief clear input content.
 *
 * \return N/A.
 */
void CEV_inputClear();


/**creates permanent link*/
/** \brief creates link to input structure.
 *
 * \param CEV_Input* to set.
 *
 * \return CEV_Input* that was recorded.
 */
CEV_Input* CEV_inputSet(CEV_Input *ptr);


/**fetch input structure*/
/** \brief gets main input structure.
 *
 * \return CEV_Input* as recorded..
 */
CEV_Input* CEV_inputGet();


void CEV_inputValue(int *val);

/**mouse box**/
/** \brief return index of box the mouse is on.
 *
 * \param box : SDL_Rect* to check.
 * \param num : number of boxes in table.
 * \return int : rect index or -1 if none.
 *
 */
int CEV_mouseBox(SDL_Rect* box ,int num);



/**mouse box with ptr table**/
/** \brief return index of box the mouse is on.
 *
 * \param box : SDL_Rect** to check.
 * \param num : number of boxes in table.
 * \return int : rect index or -1 if none.
 *
 */
int CEV_mouseBoxPtr(SDL_Rect** box ,int num);




#ifdef __cplusplus
}
#endif

#endif /* INPUTS_H_INCLUDED */
