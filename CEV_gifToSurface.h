//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    05-2016    |   1.0    |  creation/SDL2 **/
//**********************************************************/


#ifndef GIFTOSURFACE_H_INCLUDED
#define GIFTOSURFACE_H_INCLUDED

#include <SDL.h>
#include "CEV_gif.h"
#include "CEV_gifDeflate.h"


#ifdef __cplusplus
extern "C" {
#endif


/** \brief animation inner frame structure
 */
typedef struct L_GifFrame
{
    uint8_t dispMethod,
            *pixels;

    uint16_t time;

    SDL_Rect pos;
}
L_GifFrame;//local


/** \brief animation inner surface infos
 */
typedef struct L_GifSurfaceMain
{
    SDL_Rect pos;
    SDL_Texture *surface;
}
L_GifSurfaceMain;


/** \brief animation parameters and status
 */
typedef struct L_GifInfo
{
    char *comment,      /**< gif comment if any */
         signature[4],  /**< gif signature if any */
         version[4],    /**< gif version if any */
         loopDone,      /**< true when end of loop is reached */
         direction,     /**< play direction forth/back */
         refresh;       /**< redraw only without selecting picture */

    uint8_t loopMode;   /**< gif play mode */

    unsigned int time,      /**< localized instance time */
                timeAct;    /**< actual time value at measurment */

    int         imgNum, /**< num of images */
                imgAct; /**< actual selected image */

}L_GifInfo;


/** \brief gif animation instance
 */
struct CEV_GifAnim
{
    L_GifInfo status;
    L_GifSurfaceMain display;
    L_GifFrame *pictures;

};


char GIFL_gifAnimInit(CEV_GifAnim* anim, L_GifFile* gif);

char GIFL_gifBlit(CEV_GifAnim* anim);

void GIFL_gifFillSurface(uint8_t* pixels, L_GifFile* gif, int index);

void GIFL_gifFillSurfaceInterlace(uint8_t* pixels, L_GifFile* gif, int index);

void GIFL_gifFitBoxInto(SDL_Rect* adapt , const SDL_Rect* ref);



#ifdef __cplusplus
}
#endif

#endif // GIFTOSURFACE_H_INCLUDED
