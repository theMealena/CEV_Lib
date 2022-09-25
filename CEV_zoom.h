#ifndef TEST_ZOOM_H_INCLUDED
#define TEST_ZOOM_H_INCLUDED

#include <SDL.h>
#include "stdbool.h"

// TODO (drx#1#): voir comment g�rer les zoom inf�rieurs � 1

/** \brief Zoom structure definition
 */
typedef struct CEV_Zoom
{
    bool    enable;     /**< Auto zoom in when true */

    float   scaleMax,   /**< Maximum zoom value */
            scaleMin,   /**< Minimum zoom value */
            scaleAct,   /**< Actual zoom value */
            step;       /**< Zoom stepping */

    SDL_Rect    clip,       /**< Zoom position in texture */
                blit,       /**< blit position of texture */
                baseDim,    /**< Base texture to zoom on dimensions */
                renderDim;  /**< Render dimensions */
}
CEV_Zoom;


enum {ZOOM_NONE, ZOOM_IN, ZOOM_OUT};


/** \brief Dumps structure content to stdin.
 *
 * \param src : CEV_Zoom to dump.
 *
 * \return N/A.
 */
void CEV_zoomDump(CEV_Zoom src);


/** \brief Initialize zoom parameters
 *
 * \param baseW : width of base texture / display to zoom from.
 * \param baseH : Height of base texture / display to zoom from.
 * \param scaleMax : Maximum scaled applied.
 * \param step : Zoom interval at each application of zoom.
 *
 * \return CEV_Zoom structure.
 */
CEV_Zoom CEV_zoomInit(int baseW, int baseH, float scaleMax, float step);


/** \brief Set zoom max value.
 *
 * \param dst : CEV_Zoom* to set max scale.
 * \param scaleMax : float as max scaling value.
 *
 * \return N/A
 */
void CEV_zoomScaleMaxSet(CEV_Zoom* dst, float scaleMax);


/** \brief Forces scale of a zoom
 *
 * \param dst : CEV_Zoom* to be scaled.
 * \param scale : scale to be applied.
 *
 * \return N/A.
 */
void CEV_zoomScaleSet(CEV_Zoom* dst, float scale);


/** \brief Fetches actual zoom's clip
 *
 * \param src : CEV_Zoom* to fetch clip from.
 *
 * \return SDL_Rect as actual clip position:
 *
 */
SDL_Rect* CEV_zoomClipGet(CEV_Zoom* src);


/** \brief Let the zoom behave automatically
 *
 * \param zoom : CEV_Zoom*.
 * \param point : SDL_Point to zoom on/out.
 *
 * \return SDL_Rect as clip.
 */
SDL_Rect CEV_zoomAuto(CEV_Zoom* zoom, SDL_Point point);


/** \brief Applies zoom on reference texture
 *
 * \param zoom : CEV_Zoom* to apply.
 * \param point : SDL_Point to center zoom on.
 * \param direction : of ZOOM_IN, ZOOM_OUT or ZOOM_NONE.
 *
 * \return SDL_Rect as clip.
 */
SDL_Rect CEV_zoomOnCoord(CEV_Zoom *zoom, SDL_Point point, int direction);


/** \brief Update zoom"s actual zoom scale.
 *
 * \param zoom : CEV_Zoom* to update.
 * \param direction : of ZOOM_IN, ZOOM_OUT or ZOOM_NONE.
 *
 * \return bool : true when zoom is done.
 */
bool CEV_zoomScaleUpdate(CEV_Zoom *zoom, int direction);

bool CEV_zoomIsClip(CEV_Zoom *src);

#endif // TEST_ZOOM_H_INCLUDED
