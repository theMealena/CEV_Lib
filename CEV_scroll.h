//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  20-05-2016   |   0.1    |    SDL2 rev    **/
//**********************************************************/


#ifndef CONST_H_INCLUDED
#define CONST_H_INCLUDED

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "CEV_types.h"
#include "CEV_texts.h"

/**

- file extension : file.scl

- ID as 0xTTOOIIII
TT = Type of file = IS_SCROLL (11 - 0x0B)
OO = type of Object (0)
IIII = ID for this game object
*/


#ifdef __cplusplus
extern "C" {
#endif

/*file format

u32 :   id
        font size (pxl)

text color :
    u8  : r
        : g
        : b
        : a

u32 : scroll mode
    : spacing (pxl)
    : speed (pxl/frame)
CEV_Text structure
capsule : police.ttf

*/
#define SCROLL_MODE_LIST {"SCROLL_UP", "SCROLL_DOWN", "SCROLL_LEFT", "SCROLL_RIGHT", "SCROLL_STWA"}

#define SCROLL_TYPE_ID (IS_SCROLL<<24)


/** \brief scroll direction instruction
 */
typedef enum
{
    SCROLL_UP,      /**< bottom to top */
    SCROLL_DOWN,    /**< top to bottom */
    SCROLL_LEFT,    /**< right to left */
    SCROLL_RIGHT,   /**< left to right */
    SCROLL_STWA,    /**< bottom to top star wars */
    SCROLL_LAST
}
SCROLL_MODE;


/** \brief inner scroll structure
 */
typedef struct L_ScrollTextLine
{
    SDL_Texture *pic; /**< pic of text */
    SDL_Rect blitPos; /**< display position */
}
L_ScrollTextLine;




/** \brief Main scroll structure
 */
typedef struct CEV_ScrollText
{
    unsigned int id,        /**< unique id */
                 lineAct,   /**< actual line */
                 numOfLine, /**< number of lines available */
                 lineFrom,  /**< start line index */
                 lineTo,    /**< stop line index */
                 space,     /**< space size between pics, pxl */
                 fontSize,  /**< font size */
                 speed;     /**< pxl / frame */

    SCROLL_MODE mode;       /**< scrolling mode */

    int pos;                /**< display position */

    SDL_Renderer *render;   /**< destination renderer */

    SDL_Rect  renderDim;    /**< renderer logical dimension */

    SDL_Color color;        /**< text color */

    L_ScrollTextLine* texts;/**< array of pics instances */

}
CEV_ScrollText;



/** \brief Allocate and create CEV_Scroll structure.
 *
 * \param texts : an array of char* containing text to be displayed.
 * \param num : texts first dimension size (num of lines).
 * \param font : the font to be applied.
 * \param colour : SDL_Color to show text with.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollCreate(char** texts, unsigned int num, TTF_Font* font, SDL_Color colour);



/** \brief fully destroys a CEV_Scroll, including itself.
 *
 * \param in : CEV_ScrollText* to be freed.
 *
 * \return N/A
 */
void CEV_scrollDestroy(CEV_ScrollText *in);


/** \brief frees content and set to 0/nul/NULL
 *
 * \param in : CEV_ScrollText* of which content will be freed
 *
 * \return N/A
 *
 */
void CEV_scrollClear(CEV_ScrollText *in);


/** \brief dumps struct content into stdout.
 *
 * \param this : CEV_ScrollText* to dump.
 *
 * \return void.
 */
void CEV_scrollDump(CEV_ScrollText *this);

        //CONTROL FUNCTIONS*/

/** \brief sets scroll display mode.
 *
 * \param in : CEV_ScrollText* to modify.
 * \param mode : mode value.
 *
 * \return N/A.
 */
void CEV_scrollModeSet(CEV_ScrollText *in, int mode);


/** \brief sets scroll text space.
 *
 * \param in : CEV_ScrollText* to modify.
 * \param pxl : space in pixels between lines.
 *
 * \return N/A.
 */
void CEV_scrollSpaceSet(CEV_ScrollText *in, unsigned int pxl);


/** \brief reset a scroll to start again.
 *
 * \param in : CEV_ScrollText* to restart.
 *
 * \return N/A.
 */
void CEV_scrollRestart(CEV_ScrollText* in);


/** \brief sets scroll text position.
 *
 * \param in : CEV_ScrollText* to modify.
 * \param pos : centered position of display.
 *
 * \return N/A.
 *
 * \note Position is automatically interpreted as x or y
    according to display mode
 */
void CEV_scrollPosSet(CEV_ScrollText* in, int pos);


/** \brief updates position and display scroller.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return 1 while running, 0 when scroll is finished.
 */
int CEV_scrollUpdate(CEV_ScrollText *in);


        //FILE FUNCTIONS*/


/** \brief Convert user parameter file into prgm friendly file.
 *
 * \param srcName : natural file to read.
 * \param dstName : data file to create and fill.
 *
 * \return one of the function status.
 */
int CEV_scrollConvertToData(const char* srcName, const char* dstName);


/** \brief Writes to data file from CEV_Text.
 *
 * \param src : CEV_Text* build from txt file.
 * \param dst : FILE* as destination file.
 * \param srcName : char* as name of file of src.
 *
 * \return int of standard function status.
 *
 * \note src & dst are not freed in this function.
 */
int CEV_scrollConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName);


/** \brief loads scroll configuration file.
 *
 * \param fileName : file to open.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollLoad(const char* fileName);


/** \brief loads scroll from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param freeSrc : bool free src if true;
 *
 * \return CEV_ScrollText* on success, NUL on error.
 * \note if freeSrc is true, src is freed weither function succeed or not.
 */
CEV_ScrollText* CEV_scrollLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief loads scroll configuration .sdat from opened file.
 *
 * \param file : FILE* to read from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollTypeRead(FILE* file);


/** \brief Loads scroller from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 *
 * \return CEV_ScrollText* or NULL on failure.
 */
CEV_ScrollText* CEV_scrollTypeRead_RW(SDL_RWops* src);
#ifdef __cplusplus
}
#endif

#endif // CONST_H_INCLUDED */
