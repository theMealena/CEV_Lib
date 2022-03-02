//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  02-2015   |   1.0    |    creation       **/
//**   CEV    |  05-2016   |   2.0    |    SDL2 rev       **/
//**   CEV    |  03-2017   |   2.1    | usage corrections **/
//**********************************************************/

#ifndef CEV_MIXSYSTEM_H_INCLUDED
#define CEV_MIXSYSTEM_H_INCLUDED


#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "CEV_input.h"
#include "CEV_types.h"

    /* DEFINES */

/**system configuration*/

#define SCREEN_WIDTH    (CEV_videoSystemGet()->info.logicW)
#define SCREEN_HEIGHT   (CEV_videoSystemGet()->info.logicH)

/*SDLinit flags*/
#define CEV_SDL_FLAG        (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC)
/*SDL window*/
#define CEV_WIN_FLAG        (0/*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP*/)
/*SDL_Render*/
#define CEV_RENDER_FLAG     (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
/*SDL_IMG*/
#define CEV_SDL_IMG_FLAG    (IMG_INIT_JPG | IMG_INIT_PNG)
/*SDL_Mixer*/
#define CEV_MIX_FLAG        (/*MIX_INIT_MOD | */MIX_INIT_MP3)


#define CEV_MUSIC_VOLUME    (MIX_MAX_VOLUME/4)
#define CEV_SFX_VOLUME      (MIX_MAX_VOLUME/4)
#define CEV_FREQUENCY       MIX_DEFAULT_FREQUENCY
#define CEV_MIX_FORMAT      MIX_DEFAULT_FORMAT
#define CEV_MONO            1
#define CEV_STEREO          2
#define CEV_CHANNEL_NUM     16      /*number of channel to be used*/
#define CEV_CHUNK_SIZE      2048    /*sound chunk bitsize*/

#define CEV_AUTO_SIZE 0 //automatically find display ratio


#ifdef __cplusplus
extern "C" {
#endif


    /**** MISCELLANEOUS DECLARATIONS ****/

/** \brief Video display status
 */
typedef struct CEV_VideoDisplay
{
    int screenW, /**< Display pixel width */
        screenH, /**< Display pixel height */
        logicW,  /**< Render logic width */
        logicH;  /**< Render logic height */

    float proportion; /**< Display ratio as W/H */

    char *type;       /**< Screen type as string */
}
CEV_VideoDisplay;


/** \brief Sound system
 */
typedef struct CEV_SoundSystem
{
    unsigned char musicVolume,  /**< Music volume applied */
                  sfxVolume;    /**< Sfx volume applied */

    unsigned int channelNum;    /**< num of channel opened */

    CEV_Music *loadedMusic;     /**< Actually playing music */
}
CEV_SoundSystem;



/** \brief Video system
 */
typedef struct CEV_VideoSystem
{
    char isFullScreen;          /**< fullscreen on/off */

    SDL_Window      *window;    /**< Main window */
    SDL_Renderer    *render;    /**< Main renderer */
    CEV_VideoDisplay   info;    /**< Display infos */

}
CEV_VideoSystem;


/** \brief Global system
*/
typedef struct CEV_MainSystem
{
    CEV_VideoSystem   video; /**< Main video sytem */
    CEV_SoundSystem   sound; /**< Main sound system */
    CEV_Input         input; /**< Main input struct */
}
CEV_MainSystem;


        /*USER END FUNCTIONS*/


/** \brief initialze most common system as display and sound
 *-SDL
 *-SDL_mixer
 *-TTF
 *-SDL_IMG
 * \return any of function status
 */
int CEV_systemInit(void);



/** \brief free / close all sub systems
 *
 * \return N/A
 */
void CEV_systemClose(void);


/** \brief fetches main system
 *
 * \return CEV_MainSystem* or NULL if system not defined
 */
CEV_MainSystem* CEV_systemGet(void);


/** \brief fetches sound system
 *
 * \return CEV_SoundSystem* or NULL if not defined
 */
CEV_SoundSystem* CEV_soundSystemGet(void);


/** \brief fetches video system
 *
 * \return CEV_VideoSystem* or NULL if not defined
 *
 */
CEV_VideoSystem* CEV_videoSystemGet(void);


/** \brief fetch loaded music
 *
 * \param N/A
 *
 * \return valid ptr or NULL if empty
 */
CEV_Music *CEV_playingMusicGet(void);


/** \brief Switches fullscreen on/off.
 *
 * \return N/A.
 *
 */
int CEV_screenSwitch(void);



#ifdef __cplusplus
}
#endif

#endif // CEV_MIXSYSTEM_H_INCLUDED
