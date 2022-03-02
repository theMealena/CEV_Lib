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

#define SCROLL_MODE_LIST {"SCROLL_UP", "SCROLL_DOWN", "SCROLL_LEFT", "SCROLL_RIGHT"}
#define SCROLL_MODE_NUM 4

#ifdef __cplusplus
extern "C" {
#endif


/** \brief scroll direction instruction
 */
enum {SCROLL_UP = 0, SCROLL_DOWN = 1, SCROLL_LEFT = 2, SCROLL_RIGHT = 3};


/** \brief inner scroll structure
 */
typedef struct L_ScrollTextLine
{/*infos pour une ligne de text*/

    SDL_Texture *img; /*pic of text*/
    SDL_Rect blitPos; /*display position*/

}L_ScrollTextLine;




/** \brief Main scroll structure
 */
typedef struct CEV_ScrollText
{/*scroll management structure*/

    unsigned int lineAct,   /*actual line*/
                 lineNb,    /*number of lines available*/
                 lineFrom,  /*start line index*/
                 lineTo,    /*stop line index*/
                 space,     /*space size between pics, pxl*/
                 mode,      /*scrolling mode*/
                 fontSize,  /*font size*/
                 speed;     /*pxl / frame*/

    int pos;    /*display position*/

    SDL_Renderer *render; /*destination renderer*/

    SDL_Rect  renderDim; /*renderer logical dimension*/

    SDL_Color color;    /*text color*/

    L_ScrollTextLine* texts;    /*pics instances*/

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
 * \return N/A
 */
void CEV_scrollFree(CEV_ScrollText *in);


/** \brief frees content and set to 0/nul/NULL
 *
 * \param in : CEV_ScrollText* of which content will be freed
 *
 * \return N/A
 *
 */
void CEV_scrollRaz(CEV_ScrollText *in);

        /*CONTROL FUNCTIONS*/

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
 * according to display mode
 */
void CEV_scrollPosSet(CEV_ScrollText* in, int pos);


/** \brief updates position and display scroller.
 *
 * \param in : CEV_ScrollText* to update.
 *
 * \return 1 while running, 0 when scroll is finished.
 */
int CEV_scrollUpdate(CEV_ScrollText *in);


        /*FILE FUNCTIONS*/


/** \brief Convert user parameter file into prgm friendly file.
 *
 * \param srcName : natural file to read.
 * \param dstName : data file to create and fill.
 *
 * \return one of the function status.
 */
int CEV_convertScrollTxtToData(const char* srcName, const char* dstName);


/** \brief loads scroll configuration .sdat file.
 *
 * \param fileName : file to open.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollLoad(const char* fileName);



/** \brief loads scroll configuration .sdat from opened file.
 *
 * \param file : FILE* to read from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollLoadf(FILE* file);

#ifdef __cplusplus
}
#endif

#endif /* CONST_H_INCLUDED */
