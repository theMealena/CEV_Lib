#ifndef CEV_SELECTION_H_INCLUDED
#define CEV_SELECTION_H_INCLUDED

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "CEV_api.h"
#include "CEV_types.h"

/**

- file extension : file.mdat

- ID as 0xTTOOIIII
TT = Type of file = IS_MENU (10 - 0x0A)
OO = type of Object (0)
IIII = ID for this game object


//menu file content
// u32le        id of file
//              num of buttons
//              fontSize <> 0 if font embedded
// CEV_Capsule  embedded font if any (fontSize != 0)
//
//num of buttons times :
//  u32le M_TYPE for this button
//  button definition


//text button in file
// u8 : r,g,b,a off color
// u8 : r,g,b,a hover color
// u32le : pos.x, pos.y, justif, scale(*100)//
// u8[] : text char nul terminated

//pic button in file
// u32_t : num of state for this button
// u32_t : x, y as blit pos
// CEV_Capsule embedded picture

//slider in file
// u32_t : x, y, w, h as background clip
// u32_t : x, y, w, h as foreground clip
// u32_t : x, y as background blit position
// CEV_Capsule embedded picture
*/


#define M_TYPE_NAMES {"M_IS_SLIDE", "M_IS_PIC", "M_IS_TEXT"}
//                          0           1           2
#define M_TYPE_ID (IS_MENU<<24)



/** \brief Buttons / interactive types.
 */
typedef enum M_TYPE
{//buttons types
    M_IS_SLIDE  = 0,    /**< slider like option. */
    M_IS_PIC    = 1,    /**< picture as button. */
    M_IS_TEXT   = 2,    /**< text as button. */
    M_TYPE_LAST = 3     /**< num of types in this list. */
}
M_TYPE;


/** \brief Slider-like value selection
 */
typedef struct CEV_MSlide
{//slider
    M_TYPE   type;              /**< type identifier.*/
    unsigned char   value,      /**< local slider value (0-255).*/
                    *valuePtr;  /**< ptr to external value command.*/

    SDL_Texture     *pic;       /**< base picture to display.*/
    SDL_Rect        blitPos[2], /**< blit positions (back / fore).*/
                    clip[2];    /**< clip positions (back / fore).*/
}
CEV_MSlide;


/** \brief multistates Pictured button
 */
typedef struct CEV_MPic
{//pic button
    M_TYPE   type;              /**< type identifier.*/
    unsigned char   value,      /**< local value (0-255).*/
                    *valuePtr;  /**< ptr to external value command.*/

    unsigned int    stateNum;     /**< num of states.*/

    SDL_Texture     *pic;       /**< base picture to display.*/

    SDL_Rect        blitPos,    /**< blit positions.*/
                    clip;       /**< clip positions.*/
}
CEV_MPic;


/** \brief clicable Text button
 */
typedef struct CEV_MText
{//text button
    M_TYPE   type;              /**< type identifier.*/

    unsigned char   value,      /**< local state, is 1 when text is hovered.*/
                    *valuePtr;  /**< ptr to external value command.*/

    char text[64];                 /**< array of char as string to display.*/

    float scale;                /**< text scale as ratio of font size.*/

    unsigned int justif;        /**< text justification.*/

    SDL_Point pos;              /**< point on which justif is done.*/

    SDL_Color color[2];        /**< texts colors. (off/on)*/
    SDL_Rect blitPos;           /**< blit position.*/
}
CEV_MText;


/** \brief any of menu object.
 */
typedef union CEV_MSelector
{
    M_TYPE type;            /**< type identifier.*/

    struct
    {
        M_TYPE type;
        unsigned char value,
                      *valuePtr;
    }
    comn;                   /**< common parameters.*/

    CEV_MSlide slide;  /**< slider like button.*/
    CEV_MPic pic;       /**< pictured button.*/
    CEV_MText text;     /**< text button.*/
}
CEV_MSelector;


/** \brief Complete menu structure
 */
typedef struct CEV_Menu
{
    uint32_t id;                /**< Unique ID */
    unsigned int numOfButton;   /**< num of objects in this menu.*/
    CEV_Font *font;             /**< Font used for text display */
    CEV_Edge edge;              /**< local edge detection.*/

    SDL_Rect **buttonPos;       /**< array of button positions for quick indexing selection.*/

    CEV_MSelector *button;      /**< objects array.*/
}
CEV_Menu;


    //USER END FUNCTIONS

/** \brief Dumps menu structure content into stdout.
 *
 * \param this : CEV_Menu* to dump.
 *
 * \return void.
 */
void CEV_menuDump(CEV_Menu *this);


/** \brief Dumps menu's button structure content into stdout.
 *
 * \param this : CEV_MSelector* to dump.
 *
 * \return void.
 */
void CEV_menuButtonDump(CEV_MSelector* this);


/** \brief Loads CEV_Menu from file.
 *
 * \param fileName : file to load from.
 *
 * \return CEV_Menu* on success, NULL on error.
 */
CEV_Menu* CEV_menuLoad(const char* fileName);


/** \brief Loads CEV_Menu from RWops.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool closes src if true.
 *
 * \return CEV_Menu*  on success, NULL on error.
 */
CEV_Menu* CEV_menuLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Reads CEV_Menu from src and fills dst.
 *
 * \param src : FILE* to read from.
 * \param dst : CEV_Menu* to fill.
 *
 * \return int : as std funcSts.
 */
int CEV_menuTypeRead(FILE* src, CEV_Menu* dst);


/** \brief Reads CEV_Menu from src and fills dst.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_Menu* to fill.
 *
 * \return int : as std funcSts.
 */
int CEV_menuTypeRead_RW(SDL_RWops* src, CEV_Menu* dst);



/** \brief Frees menu content and itself.
 *
 * \param menu : CEV_Menu* to destroy.
 *
 * \return void.
 */
void CEV_menuDestroy(CEV_Menu *menu);


/** \brief Updates and displays menu.
 *
 * \param menu : CEV_Menu* to update.
 * \param selected : index for hover.
 * \param clic : selection validation.
 * \param x : x value for slider.
 *
 * \return int : index of text clicked or -1.
 */
int CEV_menuUpdate(CEV_Menu * menu, int selected, char clic, int x);



/** \brief Gets button access.
 *
 * \param menu : CEV_Menu* to fetch button from.
 * \param index : button index to fetch.
 *
 * \return CEV_MSelector* on success or NULL if index is out of range.
 */
CEV_MSelector* CEV_menuButtonGet(CEV_Menu *menu, unsigned int index);



/** \brief Links buttons within menu.
 *
 * \param menu : CEV_Menu* to link buttons from.
 * \param mastIndex : button index of which action will change slaveIndex value.
 * \param slaveIndex : button index of which value will be modified by action on mastIndex.
 *
 * \return N/A. Nothing happens if one of index is out of range.
 */
void CEV_menuButtonValueLink(CEV_Menu * menu, unsigned int mastIndex, unsigned int slaveIndex);



/** \brief Links button to external variable.
 *
 * \param menu : CEV_Menu* to which button belongs to.
 * \param mastIndex : button index of which action will change slave value.
 * \param slave : unsigned char* variable address to be affected by button mastIndex
 *
 * \return void.
 */
void CEV_menuButtonLink(CEV_Menu *menu, unsigned int mastIndex, unsigned char* slave);


/** \brief Converts parameters file into program friendly data.
 *
 * \param srcName : file to read from.
 * \param dstName : file to write into.
 *
 * \return int : any of std function status.
 */
int CEV_menuConvertTxtToData(const char* srcName, const char* dstName);


#endif // CEV_SELECTION_H_INCLUDED
