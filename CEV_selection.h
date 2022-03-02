#ifndef CEV_SELECTION_H_INCLUDED
#define CEV_SELECTION_H_INCLUDED

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "CEV_api.h"
#include "CEV_types.h"

#define M_TYPE_NUM 3
#define M_TYPE_LIST {"M_IS_SLIDE", "M_IS_PIC", "M_IS_TEXT"}
#define M_PIC_MAX 5




enum
{
    M_IS_SLIDE    = 0,
    M_IS_PIC      = 1,
    M_IS_TEXT     = 2

};

typedef struct CEV_MSlider
{
    unsigned char   type,
                    value,
                    *valuePtr;

    SDL_Texture     *pic;
    SDL_Rect        blitPos[2],
                    clip[2];//back / for
}
CEV_MSlider;


typedef struct CEV_MPic
{
    unsigned char   type,
                    value,
                    *valuePtr;

    unsigned int    picNum;

    SDL_Texture     *pic;

    SDL_Rect        blitPos,
                    clip;//off / on /options
}
CEV_MPic;


typedef struct CEV_MText
{
    unsigned char   type,
                    value, //1 if text hover
                    *valuePtr;
    char    *text;

    float   scale;

    unsigned int    justif;

    SDL_Point   pos;

    SDL_Rect    blitPos;

    SDL_Color   color[2]; //off / hover
}
CEV_MText;



typedef union CEV_Selector
{
    unsigned char type;

    struct
    {
        unsigned char type,
                      value,
                      *valuePtr;
    } comn;

    CEV_MSlider slide;
    CEV_MPic pic;
    CEV_MText text;
}
CEV_Selector;


typedef struct CEV_Menu
{
    unsigned int numOfButton;

    CEV_Font *font;

    CEV_Edge edge;

    SDL_Rect **buttonPos;

    CEV_Selector *button;
}
CEV_Menu;

/***USER END FUNCTIONS***/


/** \brief updates and displays menu.
 *
 * \param menu : CEV_Menu* to update.
 * \param selected : index for hover
 * \param clic : selection validation.
 * \param x : x value for slider.
 *
 * \return index of text clicked or -1.
 *
 */
int CEV_menuUpdate(CEV_Menu * menu, int selected, char clic, int x);


/**frees menu structure**/
/** \brief frees menu structure.
 *
 * \param menu : CEV_Menu* to free.
 *
 * \return N/A.
 */
void CEV_menuFree(CEV_Menu *menu);


/**getting button access**/
/** \brief get button access.
 *
 * \param menu : CEV_Menu* to fetch button from.
 * \param index : button index to fetch.
 *
 * \return CEV_Selector* on success or NULL if index is out of range.
 */
CEV_Selector* CEV_menuButtonGet(CEV_Menu *menu, unsigned int index);


/**links button together*/
/** \brief links buttons within menu.
 *
 * \param menu : CEV_Menu* to link buttons from.
 * \param mastIndex : button index of which action will change slaveIndex value.
 * \param slaveIndex : button index of which value will be modified by action on mastIndex.
 *
 * \return N/A. Nothing happens if one of index is out of range.
 */
void CEV_menuButtonValueLink(CEV_Menu * menu, unsigned int mastIndex, unsigned int slaveIndex);


/**external link**/
/** \brief links button to external variable.
 *
 * \param menu : CEV_Menu* to which button belongs to.
 * \param mastIndex : button index of which action will change slave value.
 * \param slave : unsigned char* variable address to be affected by button mastIndex
 *
 * \return void
 *
 */
void CEV_menuButtonLink(CEV_Menu *menu, unsigned int mastIndex, unsigned char* slave);


/** \brief convert parameters file into programm friendly data.
 *
 * \param srcName : file to read from.
 * \param dstName : file to write into.
 *
 * \return any of function status.
 */
int CEV_convertMenuTxtToData(const char* srcName, const char* dstName);


/** \brief loads menu.mdat file.
 *
 * \param fileName : file to load.
 *
 * \return CEV_Menu* on success, NULL on error.
 */
CEV_Menu * CEV_menuLoad(const char* fileName);


CEV_Menu * CEV_menuLoadf(FILE* src);



#endif // CEV_SELECTION_H_INCLUDED
