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


#define MSE_BT_NUM 20  /**< mouse button array size*/
#define KEYBOARD 0
#define MOUSE    1
#define PAD      2


#if SDL_MAJOR_VERSION == 2

 #define SDL_BUTTON_WHEELUP     6
 #define SDL_BUTTON_WHEELDOWN	7
 #define SDL_BUTTON_WHEELRIGHT  8
 #define SDL_BUTTON_WHEELLEFT   9

#endif // SDL_BUTTON_WHEELDOWN

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Window request & status.
 */
typedef struct CEV_Window
{

    bool quitApp,   /**< Quit apps request. */
         resize,    /**< Resize request. */
         isFocused; /**< Window is focused. */
}
CEV_Window;


/** \brief Mouse status
 */
typedef struct CEV_Mouse
{

    bool button[MSE_BT_NUM]; /**< Mouse buttons array. */
    SDL_Point pos;          /**< Actual mouse position. */
    SDL_Point rel;          /**< Relative mouse movement. */
}
CEV_Mouse;


/** \brief Input main structure.
 */
typedef struct CEV_Input
{

    bool            key[SDL_NUM_SCANCODES]; /**< Keys array. */
    unsigned char   lastDevice,             /**< Last device used. */
                    padNum;                 /**< Num of pad controller. */
    CEV_Pad         *pad;                   /**< Pads controller array. */
    CEV_Mouse       mouse;                  /**< Mouse. */
    CEV_Window      window;                 /**< Window. */
    unsigned int    lastKeyVal;             /**< Last key pushed index. */
    char            text;                   /**< SDL_TEXTINPUT char. */
}
CEV_Input;




/** \brief Updates inputs.
 *
 * \return true if any button pressed.
 */
bool CEV_inputUpdate();


/** \brief Input initialisation.
 *
 * \return FUNC_OK if init, FATAL on error.
 *
 */
int CEV_inputInit();


/** \brief Frees input structure.
 * \note memory clean-up.
 * \return N/A.
 */
void CEV_inputFree();


/** \brief Clears input content / resets to 0.
 *
 * \return N/A.
 */
void CEV_inputClear();



/** \brief Saves Input structure as default for project.
 *
 * \param CEV_Input* to set.
 *
 * \return CEV_Input* that was recorded.
 */
CEV_Input* CEV_inputSet(CEV_Input *ptr);



/** \brief Fetches main input structure.
 *
 * \return CEV_Input* as recorded..
 */
CEV_Input* CEV_inputGet();


void CEV_inputValue(int *val);


/** \brief Returns index of box the mouse is on.
 *
 * \param box : SDL_Rect* to check.
 * \param num : number of boxes in table.
 *
 * \return int : rect index or -1 if none.
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
