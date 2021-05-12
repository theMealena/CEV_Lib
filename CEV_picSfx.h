//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   1.0    |    SDL2 rev    **/
//**********************************************************/


#ifndef PICS_SFX_H_INCLUDED
#define PICS_SFX_H_INCLUDED

#include <stdint.h>
#include <SDL.h>

#define VFX_NUM 20

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Selects one of the available transition effect.
 *
 * \param src  : SDL_Texture* which effect starts from.
 * \param dst : SDL_Texture* which effect leads to.
 * \param which : index of effect to apply, -1 for random.
 * \param time : time ms the transition lasts (approx).
 *
 * \return N/A
 *
 */
void CEV_videoSfxSelect(SDL_Texture* dst,int which, uint32_t time);



#ifdef __cplusplus
}
#endif

#endif // PICS_SFX_H_INCLUDED
