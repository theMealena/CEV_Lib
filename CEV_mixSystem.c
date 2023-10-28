
//**********************************************************/
//** Done by  |    Date    |  version |    comment
//**------------------------------------------------------
//**   CEV    |  02-2015   |   1.0    |    creation
//**   CEV    |  05-2016   |   2.0    |    SDL2 rev
//**   CEV    |  03-2017   |   2.1    | usage corrections
//**   CEV    |  11-2017   |   2.1.1  |   diag improved
//**   CEV    |  01-2020   |   2.1.2  | loaded music ptr   ->CEV_playingMusicGet()
//**   CEV    |  05-2021   |   2.1.2  | local funcs as static
//**********************************************************


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "project_def.h"
#include "CEV_mixSystem.h"
#include "CEV_input.h"



    /*LOCAL FUNCTIONS DECLARATION*/

/*initialize video system as window and renderer*/
static int L_videoSystemCreate();

/*free & close video*/
static void L_videoSystemFree();

/*create sound environment SDL_Mixer*/
static int L_soundSystemCreate();

/*free / close sound system*/
static void L_soundSystemFree();

/*mesmorize main system*/
static CEV_MainSystem* L_systemSet(CEV_MainSystem* sys);

/*automatic selection 4/3 - 16/10 - 16/9*/
static bool L_videoAutoAdapt(CEV_VideoDisplay * cfg);



        /*USER END*/


int CEV_systemInit(void)
{/*system initialization **/

    CEV_MainSystem  *sys    = NULL; /*main system*/

    sys = calloc(1, sizeof(*sys));

    if(sys == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to alloc main system : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto exit_err;
    }

    L_systemSet(sys);

    srand(time(NULL));/*random seed*/

    if (L_videoSystemCreate() != FUNC_OK)
    {/*video*/
        fprintf(stderr, "Err at %s / %d : video : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto exit_err;
    }

    if (IMG_Init(CEV_SDL_IMG_FLAG) != CEV_SDL_IMG_FLAG)/*SDL_Img*/
    {/*SDL_image init*/
        fprintf(stderr, "Err at %s / %d : image : %s.\n", __FUNCTION__, __LINE__, IMG_GetError());
        goto exit_err_1;
    }

    if ( L_soundSystemCreate() != FUNC_OK )
    {/*SDL_mixer initialization*/
        fprintf(stderr, "Err at %s / %d : sound : %s %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        goto exit_err_2;
    }

    if (TTF_Init()<0)
    {/*ttf initialization*/
        fprintf(stderr, "Err at %s / %d : font : %s.\n", __FUNCTION__, __LINE__, TTF_GetError());
        goto exit_err_3;
    }

    if( CEV_inputInit() != FUNC_OK )
    {/*input initialization*/
        fprintf(stderr, "Err at %s / %d : input.\n", __FUNCTION__, __LINE__);
        goto exit_err_4;
    }

        /*POST***/

    printf("All systems have been correctly initialized.\n");

    return(FUNC_OK);

        /*EXIT ON ERROR*/

exit_err_4:
    TTF_Quit();

exit_err_3:
    L_soundSystemFree();

exit_err_2:
    IMG_Quit();

exit_err_1:
    L_videoSystemFree();

exit_err:

    return(FATAL);
}


void CEV_systemClose(void)
{/*System close**/

    CEV_inputFree();        /*frees input structure & allocations*/
    L_soundSystemFree();    /*frees sound structures and closes FMODeX / SDL_mixer*/
    L_videoSystemFree();    /*frees video structures and closes SDL*/
    TTF_Quit();             /*closes SDL_TTF*/
    IMG_Quit();             /*closes SDL_IMG*/

    free(CEV_systemGet());/*Finally frees system structure*/

    printf("All Modules have been correctly closed and freed.\n");
}


CEV_MainSystem* L_systemSet(CEV_MainSystem* sys)
{/*sets mainSystem pointer to be fetched**/

    static CEV_MainSystem *system = NULL;

    if(sys != NULL)
        system = sys;

    return system;
}


CEV_MainSystem* CEV_systemGet(void)
{/*fetches main system pointer**/

    return L_systemSet(NULL);
}


CEV_SoundSystem* CEV_soundSystemGet(void)
{/*fetches sound system pointer**/

    return &(CEV_systemGet()->sound);
}


CEV_VideoSystem* CEV_videoSystemGet(void)
{/*fetches video system pointer**/

    return &(CEV_systemGet()->video);
}


CEV_Music *CEV_playingMusicGet(void)
{
    return CEV_soundSystemGet()->loadedMusic;
}


int CEV_screenSwitch(void)
{/*to and from fullscreen*/

    int sts = FUNC_OK;

    CEV_VideoSystem *sys = CEV_videoSystemGet();

    if (!sys->isFullScreen)
        sts = SDL_SetWindowFullscreen(sys->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
    {
        sts = SDL_SetWindowFullscreen(sys->window, 0);
    }

    if (!sts)
        sys->isFullScreen ^= 1;
    else
        sts = FUNC_ERR;

    return sts;
}


    /**Local functions**/

static int L_videoSystemCreate(void)
{/*creates SDL_window and renderer*/

    CEV_VideoSystem  *sys   = CEV_videoSystemGet();
    SDL_Renderer    *render = NULL;
    SDL_Window      *window = NULL;

    //SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");

    if (SDL_Init(CEV_SDL_FLAG)<0)
    {/*SDL initialization*/
        fprintf(stderr, "Err at %s / %d : unable to init SDL : %s\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_exit;
    }

    /*adapting video screen ratio*/
    L_videoAutoAdapt(&sys->info);

    printf("Using ratio %s at %d*%d.\n", sys->info.type, sys->info.logicW, sys->info.logicH);

    window = SDL_CreateWindow(APP_NAME,
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             sys->info.logicW,
                             sys->info.logicH,
                             CEV_WIN_FLAG);


    if (window != NULL)
        render = SDL_CreateRenderer(window, -1, CEV_RENDER_FLAG);

    if((window == NULL) || (render == NULL))
    {/* window and renderer */
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, SDL_GetError());
        goto err_exit;
    }

    SDL_RenderSetLogicalSize(render, SCREEN_WIDTH, SCREEN_HEIGHT);

    sys->isFullScreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP);
    sys->render       = render;
    sys->window       = window;

    return FUNC_OK;

err_exit :

    L_videoSystemFree();

    return FATAL;
}


static void L_videoSystemFree(void)
{/*frees sdl window/renderer and quit*/

    CEV_VideoSystem* video = CEV_videoSystemGet();

    if (video != NULL)
    {
        if (video->render != NULL)
            SDL_DestroyRenderer(video->render);

        if (video->window != NULL)
            SDL_DestroyWindow(video->window);

        if (SDL_WasInit(0))
            SDL_Quit();
    }
}


static int L_soundSystemCreate(void)
{/*creates audio system and init mixer*/

    int             sts;
    CEV_MainSystem  *sys    = CEV_systemGet();

    /*system init **/
    sts = Mix_OpenAudio(CEV_FREQUENCY, CEV_MIX_FORMAT, CEV_STEREO, CEV_CHUNK_SIZE);

    if(sts < 0)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to open audio device : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        goto err;
    }

    /*mixer Init **/
    sts = Mix_Init(CEV_MIX_FLAG);

    if(sts != CEV_MIX_FLAG)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to init sound.\n", __FUNCTION__, __LINE__);
        goto err;
    }

    /*claims chunks channels**/
    sts = Mix_AllocateChannels(CEV_CHANNEL_NUM);

    if (sts != CEV_CHANNEL_NUM)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to allocate channels : %s.\n", __FUNCTION__, __LINE__, Mix_GetError());
        goto err;
    }

    /*parameters default setting*/
    sys->sound.musicVolume  = CEV_MUSIC_VOLUME;
    sys->sound.sfxVolume    = CEV_SFX_VOLUME;
    sys->sound.channelNum   = CEV_CHANNEL_NUM;
    sys->sound.loadedMusic  = NULL;

    /*setting default volume*/
    Mix_VolumeMusic(CEV_MUSIC_VOLUME);

    return FUNC_OK;

err :

    L_soundSystemFree();

    return FATAL;
}


static void L_soundSystemFree(void)
{/* frees and closes system SDL_Mixer */

    int numTimesOpened, /*used to close as many times as has been opened*/
        freq,           /*mostly dummies...*/
        chans;
    uint16_t format;

    /*playing music cleaning*/
    CEV_Music* music = CEV_playingMusicGet();
    if(music)
    {
        Mix_HaltMusic();
        CEV_musicClose(music);
    }

    while(Mix_Init(0))
        Mix_Quit();

    numTimesOpened = Mix_QuerySpec(&freq, &format, &chans);

    for(int i = 0; i < numTimesOpened; i++)
        Mix_CloseAudio();

    //CEV_SoundSystem *sdSys = CEV_soundSystemGet();
    //free(sdSys);
}


static bool L_videoAutoAdapt(CEV_VideoDisplay * cfg)
{/* screen ratio auto adaptation*/

#if CEV_AUTO_SIZE

    SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

    SDL_GetDesktopDisplayMode(0, &mode);

    cfg->screenW = mode.w;
    cfg->screenH = mode.h;


    cfg->proportion = (float)mode.w/mode.h;

    if (CEV_floatIsEqual(cfg->proportion, 1.77, 0.01))
    {
        cfg->type = "16/9";
        cfg->logicW = SCREEN_WXLARGE;
        cfg->logicH = SCREEN_HXLARGE;
    }
    else if (CEV_floatIsEqual(cfg->proportion, 1.6, 0.05))
    {
        cfg->type   = "16/10";
        cfg->logicW = SCREEN_WLARGE;
        cfg->logicH = SCREEN_HLARGE;
    }
    else if (CEV_floatIsEqual(cfg->proportion, 1.33, 0.05))
    {
        cfg->type   = "4/3";
        cfg->logicW = SCREEN_WSMALL;
        cfg->logicH = SCREEN_HSMALL;
    }

    #if SCREEN_DEFAULT_IS_DESKTOP
    {
        cfg->type   = "desktop";
        cfg->logicW = cfg->screenW;
        cfg->logicH = cfg->screenH;    }
    #endif // SCREEN_DEFAULT_IS_DESKTOP

#else
    {
        cfg->type   = "predefined";
        cfg->logicW = SCREEN_WDEFAULT;
        cfg->logicH = SCREEN_HDEFAULT;
    }
#endif // CEV_AUTO_SIZE
    return true;
}


