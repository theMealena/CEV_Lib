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

//#define METHOD_OVERWRITE 1
//#define METHOD_REDRAW 2

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
    char *comment,
         signature[4],
         version[4],
         loopDone,
         direction,
         refresh;

    uint8_t loopMode;
    unsigned int time,
                timeAct;
    int         imgNum,
                imgAct;

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
